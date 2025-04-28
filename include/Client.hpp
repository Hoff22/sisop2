#pragma once

#include "ISocket.hpp"
#include "Packet.hpp"
#include <netinet/in.h>
#include <memory>
#include <thread>

class Client {
    std::shared_ptr<ISocket> socket;
    sockaddr_in serverAddr{};
    uint32_t sequence = 1;

    bool discover(uint16_t port);

public:
    explicit Client(std::shared_ptr<ISocket> socket);
    bool connectToServer(uint16_t port);
    void run();
    void startPrinter();
    void stopPrinter();

    std::queue<std::pair<Packet, int>> printQueue;
    std::mutex printMutex;
    std::condition_variable printCond;
    std::thread printThread;
    bool printRunning = false;
};
