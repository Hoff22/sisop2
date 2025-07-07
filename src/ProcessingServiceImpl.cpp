#include "../include/ProcessingServiceImpl.hpp"

#include <iostream>
#include <arpa/inet.h>

#include "../include/Client.hpp"

inline std::string IPv4addrToString(const uint32_t addr){
    std::string str = std::to_string((htonl(addr)>>24) & 0xFF) + "." + std::to_string((htonl(addr)>>16) & 0xFF) + "." + std::to_string((htonl(addr)>>8) & 0xFF) + "." + std::to_string((htonl(addr)) & 0xFF);
    return str;
}

ProcessingServiceImpl::ProcessingServiceImpl(std::shared_ptr<ISocket> socket,
                                             std::shared_ptr<TableService> table,
                                             std::shared_ptr<ReplicaTableService> replica_table)
    : socket(std::move(socket)), table(std::move(table)), replica_table(std::move(replica_table)) {}

void ProcessingServiceImpl::handleRequest(const Packet &request, const sockaddr_in &addr)
{
    const uint32_t ip = addr.sin_addr.s_addr;
    const uint16_t port = ntohs(addr.sin_port);

    const bool isDup = table->isDuplicate(ip, port, request.seqn);

    // TODO change this to be done only by the discovery service
    const ClientInfo &info = table->getClientInfo(ip, port);

    uint32_t ackSeqn;
    uint64_t ackSum;

    if (isDup)
    {
        // Req with seqn already processed, send latest state to the client
        ackSeqn = info.last_sequence;
        ackSum = info.last_sum;

        // Notify InterfaceService with flag isDuplicate
        table->update(ip, port, request.seqn, ackSum, request.request.value, totalRequests); // não altera estado real
    }
    else
    {
        ++totalRequests;
        totalSum += request.request.value;

        ackSeqn = request.seqn;
        ackSum = totalSum;

        table->update(ip, port, ackSeqn, ackSum, request.request.value, totalRequests);
    }

    Packet ack(PacketType::REQUEST_ACK, ackSeqn);
    ack.ack.total_sum = ackSum;
    ack.ack.num_requests = totalRequests;

    socket->sendTo(ack.serialize(), addr);

    if (isDup) return;

    // Only the replica manager will access this part
    Packet request_replication(PacketType::REQUEST_REPLICATION, ackSeqn);
    request_replication.requestReplication.ip = ip;
    request_replication.requestReplication.port= port;
    request_replication.requestReplication.seqn = ackSeqn;
    request_replication.requestReplication.newSum = ackSum;
    request_replication.requestReplication.numreq = totalRequests;
    std::cout << "sending replication of request from " << IPv4addrToString(request_replication.requestReplication.ip) << "/" << request_replication.requestReplication.port << std::endl;

    std::cout << "starting to send replication message to replicas " << std::endl;
    const auto& replica_table_struct = replica_table->getTable();
    for (int i = 0; i < replica_table_struct.current_replicas; i++) {
        const auto &replica_info = replica_table_struct.table[i];
        if(socket->getSocketIp() == replica_info.ip) continue;
        std::cout << "sending message to replica in " <<  inet_ntoa(replica_info.replicaToSockaddr().sin_addr)
            << std::endl;

        socket->sendTo(request_replication.serialize(), replica_info.replicaToSockaddr());
    }
}

void ProcessingServiceImpl::handleUpdateReplicaRequest(const Packet& request, const sockaddr_in &addr) const {
    // note how ip gets here with NETWROK BYTE ORDER and port DOES NOT!
    std::cout << "processing replication of request from " << IPv4addrToString(request.requestReplication.ip) << "/" << request.requestReplication.port << std::endl;
    table->update_without_observer(request.requestReplication.ip, request.requestReplication.port,
        request.requestReplication.seqn, request.requestReplication.newSum, request.requestReplication.numreq);
}

