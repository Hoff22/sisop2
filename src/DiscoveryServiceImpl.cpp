#include <chrono>
#include <thread>
#include <arpa/inet.h>
#include "../include/Packet.hpp"

#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/Logger.hpp"

DiscoveryServiceImpl::DiscoveryServiceImpl(std::shared_ptr<ISocket> sock) : socket(std::move(sock)) {
}

void DiscoveryServiceImpl::listenForDiscoveryRequests() {
    while (true) {
        LOG_INFO("[DISCOVERY] Listening for discovery requests...");

        sockaddr_in clientAddr{};
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);

        LOG_INFO("[DISCOVERY] Received " + std::to_string(data.size()) + " bytes from " +
            std::string(inet_ntoa(clientAddr.sin_addr)));

        const Packet packet = Packet::deserialize(data);
        if (packet.type == PacketType::DISCOVERY) {
            LOG_INFO("[DISCOVERY] Discovery packet from: " + std::string(inet_ntoa(clientAddr.sin_addr)));

            Packet ack(PacketType::DISCOVERY_ACK, 0);
            auto response = ack.serialize();
            socket->sendTo(response, clientAddr);
        }
    }
}

sockaddr_in DiscoveryServiceImpl::findServer() {
    return {}; // not used on server
}
