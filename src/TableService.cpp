#include "../include/TableService.hpp"
#include "../include/TimeUtils.hpp" // for getFormattedTime
#include <arpa/inet.h>
#include <memory>
#include <mutex>

TableService::TableService(std::shared_ptr<ITableOutputObserver> observer)
    : observer(std::move(observer))
{
}

ClientInfo &TableService::getOrInsertClient(uint32_t ip, uint16_t port)
{
    const auto key = std::make_pair(ip, port);
    ClientInfo &info = client_table.getOrInsert(key, rw_mutex);
    return info;
}

ClientInfo &TableService::getClientInfo(uint32_t ip, uint16_t port)
{
    const auto key = std::make_pair(ip, port);
    ClientInfo &info = client_table.getClientInfo(key);
    return info;
}

bool TableService::isDuplicate(uint32_t ip, uint16_t port, uint32_t seqn)
{
    const auto key = std::make_pair(ip, port);
    const ClientInfo &info = client_table.getOrInsert(key, rw_mutex);
    return seqn <= info.last_sequence;
}

void TableService::update(uint32_t ip, uint16_t port, const uint32_t seqn, const uint64_t newSum,
                          const uint32_t value, const uint64_t numreq)
{
    const auto key = std::make_pair(ip, port);
    auto &info = client_table.getClientInfo(key);

    const bool isDuplicate = (seqn <= info.last_sequence);
    if (!isDuplicate)
    {
        info.last_sequence = seqn;
        info.last_sum = newSum;
        info.last_numreq = numreq;
    }

    if (observer)
    {
        const std::string timestamp = getFormattedTime();
        const std::string ipStr = inet_ntoa(in_addr{ip});

        observer->onRequestProcessed(
            timestamp,
            ipStr,
            seqn,
            value,
            info.last_numreq,
            info.last_sum,
            isDuplicate);
    }
}
