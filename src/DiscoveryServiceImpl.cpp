#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/Packet.hpp"
#include <arpa/inet.h>

DiscoveryServiceImpl::DiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                                           std::shared_ptr<TableService> table)
    : socket(std::move(socket)), table(std::move(table)) {}

void DiscoveryServiceImpl::listenForDiscoveryRequests() {
    while (true) {
        sockaddr_in clientAddr{};
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);

        if (data.empty()) continue;

        Packet packet = Packet::deserialize(data);
        if (packet.type == PacketType::DISCOVERY) {
            uint32_t ip = clientAddr.sin_addr.s_addr;
            uint16_t port = ntohs(clientAddr.sin_port);

            table->getOrInsert(ip, port);

            Packet ack(PacketType::DISCOVERY_ACK, 0);
            socket->sendTo(ack.serialize(), clientAddr);
        }
    }
}

sockaddr_in DiscoveryServiceImpl::findServer() {
    return {}; // unused in server side
}
