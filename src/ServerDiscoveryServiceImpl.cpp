#include "../include/ServerDiscoveryServiceImpl.hpp"
#include "../include/Packet.hpp"
#include <arpa/inet.h>

ServerDiscoveryServiceImpl::ServerDiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                                           std::shared_ptr<ReplicaTableService> replica_table,
                                           std::shared_ptr<TableService> client_table)
    : socket(std::move(socket)), replica_table(std::move(replica_table)), client_table(std::move(client_table)) {
}

void ServerDiscoveryServiceImpl::handleRequest(const Packet &request, const sockaddr_in &clientAddr) {
    const uint32_t ip = ntohl(clientAddr.sin_addr.s_addr);
    const uint16_t port = ntohs(clientAddr.sin_port);
    const uint32_t id = request.seqn;

    Packet ack(PacketType::SERVER_DISCOVERY_ACK, 0);
    
    ReplicaTable &t_replica = replica_table->getTable();

    ack.replicaTable.replica_table_size = t_replica.current_replicas;
    ack.replicaTable.replica_table      = t_replica.table;

    ClientTable &t_client = client_table->getTable();

    ack.replicaTable.client_table_size  = t_client.current_clients;
    ack.replicaTable.client_table       = t_client.table;
    ack.replicaTable.client_index       = t_client.client_index;
    
    // inserting the NEW server AFTER defining the ack package
    // so I don't send its own IP to it
    replica_table->getOrInsertReplica(ip, port, id);

    socket->sendTo(ack.serialize(), clientAddr);
}