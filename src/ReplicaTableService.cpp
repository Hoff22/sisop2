#include "../include/ReplicaTableService.hpp"
#include "../include/TimeUtils.hpp" // for getFormattedTime
#include <arpa/inet.h>
#include <memory>
#include <mutex>

ReplicaTableService::ReplicaTableService()
{
}

ReplicaInfo &ReplicaTableService::getOrInsertReplica(uint32_t ip, uint16_t port, uint32_t id)
{
    const auto key = std::make_pair(ip, port);
    ReplicaInfo &info = replica_table.getOrInsert(key, id, rw_mutex);
    return info;
}

ReplicaInfo &ReplicaTableService::getReplicaInfo(uint32_t ip, uint16_t port)
{
    const auto key = std::make_pair(ip, port);
    ReplicaInfo &info = replica_table.getReplicaInfo(key);
    return info;
}

void ReplicaTableService::update(uint32_t ip, uint16_t port, const uint32_t id)
{
    const auto key = std::make_pair(ip, port);
    auto &info = replica_table.getReplicaInfo(key);

    info.ip = ip;
    info.port = port;
    info.id = id;

    // if (observer)
    // {
    //     const std::string timestamp = getFormattedTime();
    //     const std::string ipStr = inet_ntoa(in_addr{ip});

    //     observer->onRequestProcessed(
    //         timestamp,
    //         ipStr,
    //         seqn,
    //         value,
    //         info.last_numreq,
    //         info.last_sum,
    //         isDuplicate);
    // }
}
