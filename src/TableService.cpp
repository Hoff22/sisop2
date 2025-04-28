#include "../include/TableService.hpp"
#include "../include/TimeUtils.hpp"  // for getFormattedTime
#include <arpa/inet.h>

TableService::TableService(std::shared_ptr<ITableOutputObserver> observer)
    : observer(std::move(observer)) {}

ClientInfo& TableService::getOrInsertClient(uint32_t ip, uint16_t port) {
    std::unique_lock lock(rw_mutex);

    auto key = std::make_pair(ip, port);
    auto [it, inserted] = clients.try_emplace(key);
    return it->second;
}

bool TableService::isDuplicate(uint32_t ip, uint16_t port, uint32_t seqn) const {
    std::shared_lock lock(rw_mutex);

    auto key = std::make_pair(ip, port);
    auto it = clients.find(key);

    if (it == clients.end()) return false;

    return seqn <= it->second.last_sequence;
}

void TableService::update(uint32_t ip, uint16_t port, uint32_t seqn, uint64_t newSum, uint32_t value) {
    std::unique_lock lock(rw_mutex);

    auto key = std::make_pair(ip, port);
    auto& info = clients[key];

    const bool isDuplicate = (seqn <= info.last_sequence);
    if (!isDuplicate) {
        info.last_sequence = seqn;
        info.last_sum = newSum;
    }

    if (observer) {
        std::string timestamp = getFormattedTime();
        std::string ipStr = inet_ntoa(in_addr{ip});

        observer->onRequestProcessed(
            timestamp,
            ipStr,
            seqn,
            value,
            info.last_sequence,
            info.last_sum,
            isDuplicate
        );
    }
}
