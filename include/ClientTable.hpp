#pragma once
#include "ClientInfo.hpp"
#include <unordered_map>
#include <mutex>
#include <netinet/in.h>

template<>
struct std::hash<std::pair<uint32_t, uint16_t>> {
    std::size_t operator()(const std::pair<uint32_t, uint16_t>& k) const {
        return std::hash<uint64_t>()((static_cast<uint64_t>(k.first) << 16) | k.second);
    }
};

class ClientTable {
    std::mutex mutex;

    // key: pair of (IP, port)
    std::unordered_map<std::pair<uint32_t, uint16_t>, ClientInfo> clients;

public:
    ClientInfo& getOrInsert(uint32_t ip, uint16_t port);
    bool isDuplicate(uint32_t ip, uint16_t port, uint32_t seqn);
    void update(uint32_t ip, uint16_t port, uint32_t seqn, uint64_t sum);
};
