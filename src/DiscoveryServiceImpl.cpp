#include "../include/DiscoveryServiceImpl.hpp"
#include <arpa/inet.h>
#include "../include/Packet.hpp"

DiscoveryServiceImpl::DiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                                           std::shared_ptr<TableService> table)
    : socket(std::move(socket)), table(std::move(table)) {
}

void DiscoveryServiceImpl::handleRequest(const sockaddr_in &clientAddr) {
    const uint32_t ip = clientAddr.sin_addr.s_addr;
    const uint16_t port = ntohs(clientAddr.sin_port);

    table->getOrInsertClient(ip, port);

    const Packet ack(PacketType::DISCOVERY_ACK, 0);
    socket->sendTo(ack.serialize(), clientAddr);
}
