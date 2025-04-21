#include <thread>
#include <utility>
#include <vector>
#include <arpa/inet.h>
#include <iostream>

#include "../include/Logger.hpp"
#include "../include/Packet.hpp"
#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/TimeUtils.hpp"

Server::Server(std::shared_ptr<ISocket> socket,
               std::shared_ptr<IDiscoveryService> discovery,
               std::shared_ptr<IProcessingService> processing)
    : socket(std::move(socket)),
      discoveryService(std::move(discovery)),
      processingService(std::move(processing)) {
}

void Server::start() const {
    std::cout << getFormattedTime() << " num_reqs 0 total_sum 0" << std::endl;

    std::thread discoveryThread(&Server::startDiscovery, this);

    while (true) {
        LOG_INFO("[SERVER] Listening for requests...");
        sockaddr_in clientAddr{};
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);

        if (data.empty()) {
            continue;
        }

        try {
            if (Packet packet = Packet::deserialize(data); packet.type == PacketType::REQUEST) {
                processingService->handleRequest(packet, clientAddr);
            }
        } catch (const std::exception &e) {
            LOG_ERROR(std::string("[SERVER] Failed to deserialize packet: ") + e.what());
        }
    }

    discoveryThread.join();
}

void Server::startDiscovery() const {
    LOG_INFO("[DISCOVERY] Starting discovery service...");
    discoveryService->listenForDiscoveryRequests();
}
