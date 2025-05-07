#pragma once

#include "ITableOutputObserver.hpp"
#include "ClientInfo.hpp"
#include <unordered_map>
#include <utility>
#include <memory>
#include <cstdint>
#include <mutex>
#include <string>
#include <sstream>

// The idea here is that since we are working with few clients <= 10, this will have less overhead than using a map.
// We are not treating the case in which we have more than 100 clients, if that happens the program will just explode.
struct ClientTable
{
    static constexpr int maxClients = 100;
    int current_clients = 0;

    ClientInfo table[maxClients];
    std::pair<uint32_t, uint16_t> client_index[maxClients];

    ClientInfo &getOrInsert(const std::pair<uint32_t, uint16_t> &key, std::mutex &mutex)
    {
        std::lock_guard<std::mutex> lock(mutex);

        for (int i = 0; i < current_clients; i++)
        {
            if (client_index[i] == key)
            {
                return table[i];
            }
        }

        client_index[++current_clients] = key;
        return table[current_clients - 1];
    }

    /**
     * gets info for the specific key without mutex overhead. READONLY!!
     * if key is not present throws
     */
    ClientInfo &getClientInfo(const std::pair<uint32_t, uint16_t> &key)
    {
        for (int i = 0; i < current_clients; i++)
        {
            if (client_index[i] == key)
            {
                return table[i];
            }
        }
        std::ostringstream oss;
        oss << "Could not find index for key [" << key.first << "," << key.second << "] in client index table";
        throw std::runtime_error(oss.str());
    }
};

class TableService
{
public:
    TableService(std::shared_ptr<ITableOutputObserver> observer);

    ClientInfo &getOrInsertClient(uint32_t ip, uint16_t port);
    ClientInfo &getClientInfo(uint32_t ip, uint16_t port);
    bool isDuplicate(uint32_t ip, uint16_t port, uint32_t seqn);
    void update(uint32_t ip, uint16_t port, uint32_t seqn, uint64_t newSum, uint32_t value, uint64_t numreq);

private:
    ClientTable client_table;
    mutable std::mutex rw_mutex;

    std::shared_ptr<ITableOutputObserver> observer;
};
