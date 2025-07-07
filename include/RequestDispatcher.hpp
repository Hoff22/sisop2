#pragma once

#include "Packet.hpp"
#include "IDiscoveryService.hpp"
#include "IProcessingService.hpp"
#include "ServerDiscoveryServiceImpl.hpp"
#include "HeartbeatService.hpp"

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <atomic>
#include <sstream>

#include "ProcessingServiceImpl.hpp"

class RequestDispatcher
{
public:
    RequestDispatcher(std::shared_ptr<ProcessingServiceImpl> processingService,
                      std::shared_ptr<IDiscoveryService> discoveryService,
                      std::shared_ptr<ServerDiscoveryServiceImpl> serverDiscoveryService,
                      size_t numThreads = 8);

    ~RequestDispatcher();

    void enqueue(Packet &packet, sockaddr_in &clientAddr);
    void enterA();
    void exitA();
    void enterB();
    void exitB();

    void start();
    void stop();


    static constexpr int maxClients = 10;
    int current_clients = 0;
    std::mutex in_proc[maxClients];
    std::pair<uint32_t, uint16_t> client_index[maxClients];
    void worker();
    int getClientIndex(uint32_t ip, uint16_t port);
    void setClientIndex(uint32_t ip, uint16_t port);

    struct Request
    {
        Packet packet;
        sockaddr_in clientAddr;
    };

    std::mutex mutex;
    std::mutex a_b_mutex;
    std::condition_variable cond;
    std::condition_variable a_b_cv;
    std::vector<std::thread> threads;

    std::shared_ptr<ProcessingServiceImpl> processingService;
    std::shared_ptr<IDiscoveryService> discoveryService;
    std::shared_ptr<ServerDiscoveryServiceImpl> serverDiscoveryService;
    std::shared_ptr<HeartbeatService> heartbeatService;
    size_t numThreads;
    std::atomic<bool> running;

    std::vector<std::optional<Request>> buffer;
    size_t head = 0;
    size_t tail = 0;
    size_t bufferCapacity;

    int active_A = 0;
    int active_B = 0;
    int waiting_A = 0;
};
