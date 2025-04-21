#include <iostream>
#include <unistd.h>

#include "../include/Client.hpp"
#include "../include/Logger.hpp"
#include "../include/UdpSocket.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./main_client <port>\n";
        return 1;
    }

    const std::string log_file = "client_" + std::to_string(getpid()) + ".log";
    Logger::init(log_file);
    LOG_INFO("Client started.");

    const uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

    const auto socket = std::make_shared<UdpSocket>();
    socket->open();
    socket->addTimeout();

    Client client(socket);
    if (!client.connectToServer(port)) {
        std::cerr << "[CLIENT] Unable to connect to server.\n";
        return 1;
    }

    client.run();
    return 0;
}
