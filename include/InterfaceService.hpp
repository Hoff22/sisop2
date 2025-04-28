#pragma once

#include "ITableOutputObserver.hpp"
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>

struct RequestInfo {
    std::string timestamp;
    std::string clientIp;
    uint32_t seqn;
    uint32_t value;
    uint64_t numRequests;
    uint64_t totalSum;
    bool isDuplicate;
};

class InterfaceService : public ITableOutputObserver {
public:
    InterfaceService();
    ~InterfaceService() override;

    void onRequestProcessed(const std::string& timestamp,
                            const std::string& clientIp,
                            uint32_t seqn,
                            uint32_t value,
                            uint64_t numRequests,
                            uint64_t totalSum,
                            bool isDuplicate) override;

private:
    void run();
    std::queue<RequestInfo> queue;
    std::mutex mutex;
    std::condition_variable cv;
    std::thread worker;
    bool shutdown = false;
};