#include <thread>
#include <utility>
#include <vector>
#include <arpa/inet.h>
#include <iostream>

#include "../include/Packet.hpp"
#include "../include/Server.hpp"

#include <unistd.h>
#include <netdb.h>

#include "../include/UdpSocket.hpp"
#include "../include/TimeUtils.hpp"
#include "../include/RequestDispatcher.hpp"

Server::Server(int id, std::shared_ptr<ISocket> socket,
               const std::shared_ptr<RequestDispatcher>& request_dispatcher)
    : server_id(id), socket(std::move(socket)),
      dispatcher(request_dispatcher)
{
}

void Server::start(uint16_t port) {
    std::cout << getFormattedTime() << " num_reqs 0 total_sum 0" << std::endl;

    if(discover(port)){
        std::cout << "connected" << std::endl;
    }
    else{
        std::cout << "FUUUUUUUCK" << std::endl;
    }

    dispatcher->start();

    while (true) {
        sockaddr_in clientAddr{};
        std::vector<uint8_t> data = socket->receiveFrom(clientAddr);

        if (data.empty()) {
            continue;
        }

        try {
            Packet packet = Packet::deserialize(data);
            dispatcher->enqueue(packet, clientAddr);
        } catch (const std::exception &e) {
            std::cerr << "Failed to deserialize packet: " << e.what() << std::endl;
        }
    }
}

bool Server::discover(uint16_t port){
    const int enable = 1;
    setsockopt(socket->getRawSocket(), SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

    const Packet discovery(PacketType::SERVER_DISCOVERY, server_id);
    const auto data = discovery.serialize();

    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    socket->sendTo(data, broadcastAddr);

    sockaddr_in serverAddr{};

    std::vector<uint8_t> response;
    Packet ack;
    do {
        response = socket->receiveFrom(serverAddr);
        std::string received_ip = inet_ntoa(serverAddr.sin_addr);

        if (response.empty()) {
            break;
        }

        std::cout << "Received response from: " << received_ip << std::endl;
        ack = Packet::deserialize(response);
    } while (ack.type != PacketType::SERVER_DISCOVERY_ACK);

    if (response.empty()) {
        std::cout << "No server answered in the discovering phase" << std::endl;
        return false;
    }

    try {
        if (Packet ack = Packet::deserialize(response); ack.type == PacketType::SERVER_DISCOVERY_ACK) {

            auto tableService = dispatcher->serverDiscoveryService->table;

            for(size_t i = 0; i < ack.replicaTable.table_size; i++){
                tableService->getOrInsertReplica(ack.replicaTable.table[i].ip, ack.replicaTable.table[i].port, ack.replicaTable.table[i].id);
            }

            std::cout << getFormattedTime()
                    << " server_addr " << inet_ntoa(serverAddr.sin_addr) << std::endl;
            return true;
        }
        std::cout << "Server responded with unexpected PacketType " << inet_ntoa(serverAddr.sin_addr) << std::endl;
    } catch (...) {
        std::cerr << "Failed to parse discovery response." << std::endl;
    }

    return false;
}