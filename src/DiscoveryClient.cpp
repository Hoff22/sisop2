#include "../include/UdpSocket.hpp"
#include "../include/Packet.hpp"
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./discovery_client <port>\n";
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));
    UdpSocket socket;
    socket.open(); // Important!

    // Enable broadcast
    int enable = 1;
    if (setsockopt(socket.getRawSocket(), SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0) {
        perror("setsockopt");
        return 1;
    }

    std::cout << "[CLIENT] Sending discovery...\n";

    // Create discovery packet
    Packet discovery(PacketType::DISCOVERY, 0);
    auto data = discovery.serialize();

    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    socket.sendTo(data, broadcastAddr);

    std::cout << "[CLIENT] Discovery sent, waiting for response...\n";

    sockaddr_in serverAddr{};
    std::vector<uint8_t> response = socket.receiveFrom(serverAddr);

    if (response.empty()) {
        std::cerr << "[CLIENT] No response received.\n";
        return 1;
    }

    try {
        Packet ack = Packet::deserialize(response);
        if (ack.type == PacketType::DISCOVERY_ACK) {
            std::cout << "[CLIENT] Server found at: " << inet_ntoa(serverAddr.sin_addr) << std::endl;
        } else {
            std::cerr << "[CLIENT] Unexpected packet type.\n";
        }
    } catch (const std::exception& e) {
        std::cerr << "[CLIENT] Failed to parse response: " << e.what() << std::endl;
    }

    return 0;
}
