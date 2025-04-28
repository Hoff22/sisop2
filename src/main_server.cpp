#include <memory>
#include <iostream>
#include <unistd.h>

#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/ProcessingServiceImpl.hpp"
#include "../include/InterfaceService.hpp"
#include "../include/TableService.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./server <port>\n";
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

    try {
        auto socket = std::make_shared<UdpSocket>();
        socket->bind(port);

        auto interface = std::make_shared<InterfaceService>();
        auto table = std::make_shared<TableService>(interface);

        const auto discovery = std::make_shared<DiscoveryServiceImpl>(socket, table);
        const auto processing = std::make_shared<ProcessingServiceImpl>(socket, table);

        Server server(socket, discovery, processing);
        server.start();

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
