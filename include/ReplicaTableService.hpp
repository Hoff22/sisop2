#pragma once
#include "ReplicaInfo.hpp"
#include <unordered_map>
#include <utility>
#include <memory>
#include <cstdint>
#include <mutex>
#include <string>
#include <sstream>

struct ReplicaTable
{
    static constexpr int maxReplicas = 100;
    int current_replicas = 0;

    ReplicaInfo table[maxReplicas];
    std::pair<uint32_t, uint16_t> replica_index[maxReplicas];

    ReplicaInfo &getOrInsert(const std::pair<uint32_t, uint16_t> &key, uint32_t id, std::mutex &mutex)
    {
        std::lock_guard<std::mutex> lock(mutex);

        for (int i = 0; i < current_replicas; i++)
        {
            if (replica_index[i] == key)
            {
                return table[i];
            }
        }

        replica_index[++current_replicas] = key;
        return table[current_replicas - 1] = {key.first, key.second, id};
    }

    /**
     * gets info for the specific key without mutex overhead. READONLY!!
     * if key is not present throws
     */
    ReplicaInfo &getReplicaInfo(const std::pair<uint32_t, uint16_t> &key)
    {
        for (int i = 0; i < current_replicas; i++)
        {
            if (replica_index[i] == key)
            {
                return table[i];
            }
        }
        std::ostringstream oss;
        oss << "Could not find index for key [" << key.first << "," << key.second << "] in replica index table";
        throw std::runtime_error(oss.str());
    }
};

class ReplicaTableService
{
public:
    ReplicaTableService();

    ReplicaInfo  &getOrInsertReplica(uint32_t ip, uint16_t port, uint32_t id);
    ReplicaInfo  &getReplicaInfo(uint32_t ip, uint16_t port);
    ReplicaTable &getTable() { return replica_table; }
    void update(uint32_t ip, uint16_t port, const uint32_t id);

private:
    ReplicaTable replica_table;
    mutable std::mutex rw_mutex;
};