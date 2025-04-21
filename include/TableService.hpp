#pragma once

#include "ITableOutputObserver.hpp"
#include "ClientInfo.hpp"
#include <unordered_map>
#include <shared_mutex>
#include <utility>
#include <memory>
#include <cstdint>
#include <string>

class TableService {
public:
    TableService(std::shared_ptr<ITableOutputObserver> observer);

    ClientInfo& getOrInsert(uint32_t ip, uint16_t port);
    bool isDuplicate(uint32_t ip, uint16_t port, uint32_t seqn) const;
    void update(uint32_t ip, uint16_t port, uint32_t seqn, uint64_t newSum, uint32_t value);

private:
    using ClientKey = std::pair<uint32_t, uint16_t>;
    struct pair_hash {
        size_t operator()(const ClientKey& k) const {
            return std::hash<uint64_t>()((static_cast<uint64_t>(k.first) << 16) | k.second);
        }
    };

    std::unordered_map<ClientKey, ClientInfo, pair_hash> clients;
    mutable std::shared_mutex rw_mutex;

    std::shared_ptr<ITableOutputObserver> observer;
};
