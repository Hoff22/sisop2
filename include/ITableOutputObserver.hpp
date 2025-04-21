#pragma once

#include <string>
#include <cstdint>

// Basically we are going to have an InterfaceService module that reacts to changes/operations made in the
// client table. It will follow an observer pattern.
class ITableOutputObserver {
public:
    virtual ~ITableOutputObserver() = default;

    virtual void onClientInserted(uint32_t ip, uint16_t port) = 0;

    virtual void onRequestProcessed(const std::string& timestamp,
                                    const std::string& clientIp,
                                    uint32_t seqn,
                                    uint32_t value,
                                    uint64_t numRequests,
                                    uint64_t totalSum,
                                    bool isDuplicate) = 0;
};
