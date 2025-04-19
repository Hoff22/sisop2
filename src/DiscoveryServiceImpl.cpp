#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/Packet.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <thread>
#include <chrono>

DiscoveryServiceImpl::DiscoveryServiceImpl(std::shared_ptr<ISocket> sock) : socket(std::move(sock)) {}

void DiscoveryServiceImpl::listenForDiscoveryRequests() {
    while (true) {
        std::cout << "[DISCOVERY] Listening for discovery requests..." << std::endl;
 
        sockaddr_in clientAddr{};
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);
        std::cout << "[DISCOVERY] Received " << data.size() << " bytes from "
                  << inet_ntoa(clientAddr.sin_addr) << std::endl;

        Packet packet = Packet::deserialize(data);
        if (packet.type == PacketType::DISCOVERY) {
            std::cout << "[DISCOVERY] Received from: " << inet_ntoa(clientAddr.sin_addr) << std::endl;

            Packet ack(PacketType::DISCOVERY_ACK, 0);
            auto response = ack.serialize();
            socket->sendTo(response, clientAddr);
        }

    }
}

sockaddr_in DiscoveryServiceImpl::findServer() {
    return {}; // not used on server
}
