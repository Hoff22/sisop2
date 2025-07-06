#include <memory>
#include <iostream>
#include <unistd.h>
#include <string>

#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/ServerDiscoveryServiceImpl.hpp"
#include "../include/ProcessingServiceImpl.hpp"
#include "../include/InterfaceService.hpp"
#include "../include/TableService.hpp"
#include "../include/ReplicaTableService.hpp"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: ./server <port> <server id>\n";
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

    try {
        auto manager_socket = std::make_shared<UdpSocket>();
        manager_socket->bind(port);
        manager_socket->addTimeout();

        auto interface = std::make_shared<InterfaceService>();
        auto replica_table = std::make_shared<ReplicaTableService>();
        auto client_table = std::make_shared<TableService>(interface);

        const auto server_discovery = std::make_shared<ServerDiscoveryServiceImpl>(manager_socket, replica_table,
            client_table);
        const auto client_discovery = std::make_shared<DiscoveryServiceImpl>(manager_socket, client_table);
        const auto processing = std::make_shared<ProcessingServiceImpl>(manager_socket, client_table);

        const auto request_dispatcher = std::make_shared<RequestDispatcher>(processing, client_discovery,
            server_discovery, 4);

        Server server(std::stoi(argv[2]), manager_socket, request_dispatcher, client_table);
        server.start(port);

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
