#pragma once
#include <mutex>
#include <condition_variable>

class Aggregator {
    std::mutex mutex;
    std::condition_variable updated;

    uint64_t total_sum = 0;
    uint32_t total_requests = 0;

public:
    void add(uint32_t value);
    uint64_t getSum();
    uint32_t getRequestCount();

    void waitForUpdate();
};
