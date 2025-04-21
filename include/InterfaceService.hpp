#pragma once

#include "ITableOutputObserver.hpp"

class InterfaceService : public ITableOutputObserver {
public:
    void onClientInserted(uint32_t ip, uint16_t port) override;

    void onRequestProcessed(const std::string& timestamp,
                            const std::string& clientIp,
                            uint32_t seqn,
                            uint32_t value,
                            uint64_t numRequests,
                            uint64_t totalSum,
                            bool isDuplicate) override;
};
