#pragma once

#include "Packet.hpp"
#include "IDiscoveryService.hpp"
#include "IProcessingService.hpp"

#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <atomic>


class RequestDispatcher {
public:
    RequestDispatcher(std::shared_ptr<IProcessingService> processingService,
                      std::shared_ptr<IDiscoveryService> discoveryService,
                      size_t numThreads = 8
    );

    ~RequestDispatcher();

    void enqueue(Packet &packet, sockaddr_in &clientAddr);

    void start();

    void stop();

private:
    void worker();

    struct Request {
        Packet packet;
        sockaddr_in clientAddr;
    };

    std::queue<Request> queue;
    std::mutex mutex;
    std::condition_variable cond;
    std::vector<std::thread> threads;
    std::shared_ptr<IProcessingService> processingService;
    std::shared_ptr<IDiscoveryService> discoveryService;
    size_t numThreads;
    std::atomic<bool> running;
};
