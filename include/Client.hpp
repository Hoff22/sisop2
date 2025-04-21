#pragma once

#include "ISocket.hpp"
#include "Packet.hpp"
#include <netinet/in.h>
#include <memory>

class Client {
    std::shared_ptr<ISocket> socket;
    sockaddr_in serverAddr{};
    uint32_t sequence = 1;

    bool discover(uint16_t port);
    void handleResponse(const std::vector<uint8_t>& data);

public:
    explicit Client(std::shared_ptr<ISocket> socket);
    bool connectToServer(uint16_t port);
    void run();
};
