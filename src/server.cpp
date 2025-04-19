#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/Packet.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <arpa/inet.h>

Server::Server(std::shared_ptr<ISocket> socket,
               std::shared_ptr<IDiscoveryService> discovery,
               std::shared_ptr<IProcessingService> processing)
    : socket(socket), discoveryService(discovery), processingService(processing) {}

void Server::start() {
    // Start the discovery service in a separate thread
    //std::thread discoveryThread(&Server::startDiscovery, this);
    startDiscovery();

    // Main loop to receive requests and process them
    while (true) {
        std::cout << "[SERVER] Listening for requests..." << std::endl;
        sockaddr_in clientAddr;
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);

        if (data.empty()) {
            continue;
        }

        try {
            Packet packet = Packet::deserialize(data);

            if (packet.type == PacketType::REQUEST) {
                processingService->handleRequest(packet, clientAddr);
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Failed to deserialize packet: " << e.what() << std::endl;
        }
    }

    // Never reached, but clean exit if needed
    //discoveryThread.join();
}

void Server::startDiscovery() {
    std::cout << "[DISCOVERY] Starting discovery service..." << std::endl;
    discoveryService->listenForDiscoveryRequests();
}
