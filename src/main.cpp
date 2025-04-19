#include <iostream>

#include "../include/Server.hpp"
#include "../include/UdpSocket.hpp"

// Placeholder stubs for now — you’ll implement them shortly
class DummyDiscovery : public IDiscoveryService {
public:
	sockaddr_in findServer() override { return {}; }
	void listenForDiscoveryRequests() override {}
};

class DummyProcessing : public IProcessingService {
public:
	void handleRequest(const Packet& request, const sockaddr_in& addr) override {}
};

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cerr << "Usage: ./server <port>\n";
		return 1;
	}

	uint16_t port = static_cast<uint16_t>(std::stoi(argv[1]));

	auto socket = std::make_shared<UdpSocket>();
	auto discovery = std::make_shared<DummyDiscovery>();
	auto processing = std::make_shared<DummyProcessing>();

	socket->bind(port);

	Server server(socket, discovery, processing);
	server.start();

	return 0;
}