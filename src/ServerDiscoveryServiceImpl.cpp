#include "../include/ServerDiscoveryServiceImpl.hpp"
#include "../include/Packet.hpp"
#include <arpa/inet.h>

ServerDiscoveryServiceImpl::ServerDiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                                           std::shared_ptr<ReplicaTableService> replica_table,
                                           std::shared_ptr<TableService> client_table)
    : socket(std::move(socket)), replica_table(std::move(replica_table)), client_table(std::move(client_table)) {
}

void ServerDiscoveryServiceImpl::handleRequest(const Packet &request, const sockaddr_in &clientAddr) {
    const uint32_t ip = clientAddr.sin_addr.s_addr;
    const uint16_t port = ntohs(clientAddr.sin_port);
    const uint32_t id = request.seqn;

    replica_table->getOrInsertReplica(ip, port, id);

    Packet ack(PacketType::SERVER_DISCOVERY_ACK, 0);

    ReplicaTable &t = replica_table->getTable();

    ack.replicaTable.replica_table_size = t.current_replicas;
    ack.replicaTable.replica_table      = t.table;

    ack.replicaTable.client_table_size  = client_table->client_table.current_clients;
    ack.replicaTable.client_table       = client_table->client_table.table;
    ack.replicaTable.client_index       = client_table->client_table.client_index;

    socket->sendTo(ack.serialize(), clientAddr);
}