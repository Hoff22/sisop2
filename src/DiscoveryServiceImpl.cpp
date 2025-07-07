#include <arpa/inet.h>
#include <string>
#include <iostream>
#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/Packet.hpp"

DiscoveryServiceImpl::DiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                                           std::shared_ptr<TableService> table)
    : socket(std::move(socket)), table(std::move(table)) {
}

inline std::string IPv4addrToString(const uint32_t addr){
    std::string str = std::to_string((htonl(addr)>>24) & 0xFF) + "." + std::to_string((htonl(addr)>>16) & 0xFF) + "." + std::to_string((htonl(addr)>>8) & 0xFF) + "." + std::to_string((htonl(addr)) & 0xFF);
    return str;
}

void DiscoveryServiceImpl::handleRequest(const sockaddr_in &clientAddr) {
    const uint32_t ip = clientAddr.sin_addr.s_addr;
    const uint16_t port = ntohs(clientAddr.sin_port);

    table->getOrInsertClient(ip, port);

    const Packet ack(PacketType::DISCOVERY_ACK, 0);
    socket->sendTo(ack.serialize(), clientAddr);
}
