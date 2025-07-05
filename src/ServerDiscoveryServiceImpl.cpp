#include "../include/ServerDiscoveryServiceImpl.hpp"
#include <arpa/inet.h>
#include "../include/Packet.hpp"

ServerDiscoveryServiceImpl::ServerDiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                                           std::shared_ptr<ReplicaTableService> table)
    : socket(std::move(socket)), table(std::move(table)) {
}

void ServerDiscoveryServiceImpl::handleRequest(const Packet &request, const sockaddr_in &clientAddr) {
    const uint32_t ip = clientAddr.sin_addr.s_addr;
    const uint16_t port = ntohs(clientAddr.sin_port);
    const uint32_t id = request.seqn;

    table->getOrInsertReplica(ip, port, id);

    Packet ack(PacketType::SERVER_DISCOVERY_ACK, 0);

    ReplicaTable &t = table->getTable();

    ack.replicaTable.table_size = t.current_replicas;
    ack.replicaTable.table      = t.table;

    socket->sendTo(ack.serialize(), clientAddr);
}