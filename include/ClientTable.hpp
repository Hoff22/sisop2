#pragma once
#include "ClientInfo.hpp"
#include <unordered_map>
#include <mutex>
#include <netinet/in.h>

class ClientTable {
    std::mutex mutex;
    std::unordered_map<uint32_t, ClientInfo> clients; // key: IPv4 as uint32_t

public:
    ClientInfo& getOrInsert(uint32_t ip);
    bool isDuplicate(uint32_t ip, uint32_t seqn);
    void update(uint32_t ip, uint32_t seqn, uint64_t sum);
};
