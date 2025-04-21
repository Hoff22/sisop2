#include <memory>
#include <iostream>
#include <unistd.h>

#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/ProcessingServiceImpl.hpp"
#include "../include/Logger.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./server <port>\n";
        return 1;
    }

    const std::string log_file = "server_" + std::to_string(getpid()) + ".log";
    Logger::init(log_file);
    LOG_INFO("Server started.");

    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

    try {
        auto socket = std::make_shared<UdpSocket>();
        socket->bind(port);

        const auto discovery = std::make_shared<DiscoveryServiceImpl>(socket);
        const auto processing = std::make_shared<ProcessingServiceImpl>(socket);

        const Server server(socket, discovery, processing);
        server.start();

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
