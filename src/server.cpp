#include <thread>
#include <utility>
#include <vector>
#include <arpa/inet.h>
#include <iostream>

#include "../include/Packet.hpp"
#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/TimeUtils.hpp"

Server::Server(std::shared_ptr<ISocket> socket,
               std::shared_ptr<IDiscoveryService> discovery,
               std::shared_ptr<IProcessingService> processing)
    : socket(std::move(socket)),
      discoveryService(std::move(discovery)),
      processingService(std::move(processing)),
      dispatcher(processingService, discoveryService, 4) {
}

void Server::start() {
    std::cout << getFormattedTime() << " num_reqs 0 total_sum 0" << std::endl;

    dispatcher.start();

    while (true) {
        sockaddr_in clientAddr{};
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);

        if (data.empty()) {
            continue;
        }

        try {
            Packet packet = Packet::deserialize(data);
            dispatcher.enqueue(packet, clientAddr);
        } catch (const std::exception &e) {
            std::cerr << "Failed to deserialize packet: " << e.what() << std::endl;
        }
    }
}
