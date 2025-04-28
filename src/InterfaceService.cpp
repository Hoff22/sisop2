#include "../include/InterfaceService.hpp"

#include <iostream>

InterfaceService::InterfaceService() : worker(&InterfaceService::run, this) {}

InterfaceService::~InterfaceService() {
    {
        std::lock_guard lock(mutex);
        shutdown = true;
    }
    cv.notify_one();
    worker.join();
}

void InterfaceService::onRequestProcessed(const std::string& timestamp,
                                          const std::string& clientIp,
                                          uint32_t seqn,
                                          uint32_t value,
                                          uint64_t numRequests,
                                          uint64_t totalSum,
                                          bool isDuplicate) {
    {
        std::lock_guard lock(mutex);
        queue.push(RequestInfo{timestamp, clientIp, seqn, value, numRequests, totalSum, isDuplicate});
    }
    cv.notify_one();
}

void InterfaceService::run() {
    while (true) {
        RequestInfo info;

        {
            std::unique_lock lock(mutex);
            cv.wait(lock, [this] { return !queue.empty() || shutdown; });

            if (shutdown && queue.empty()) break;

            info = queue.front();
            queue.pop();
        }

        std::cout << info.timestamp
                  << " client " << info.clientIp
                  << (info.isDuplicate ? " DUP!!" : "")
                  << " id_req " << info.seqn
                  << " value " << info.value
                  << " num_reqs " << info.numRequests
                  << " total_sum " << info.totalSum
                  << std::endl;
    }
}
