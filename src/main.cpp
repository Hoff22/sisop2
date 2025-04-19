#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"
#include "../include/DiscoveryServiceImpl.hpp"
#include "../include/IProcessingService.hpp"
#include <memory>
#include <iostream>

// Dummy processing service just to complete the interface
class DummyProcessing : public IProcessingService {
public:
    void handleRequest(const Packet& request, const sockaddr_in& addr) override {
        // Do nothing for now — real logic comes later
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./server <port>\n";
        return 1;
    }

    uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

    try {
        auto socket = std::make_shared<UdpSocket>();
        socket->bind(port);

        auto discovery = std::make_shared<DiscoveryServiceImpl>(socket);
        auto processing = std::make_shared<DummyProcessing>();

        Server server(socket, discovery, processing);
        server.start();

    } catch (const std::exception& e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
