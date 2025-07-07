#pragma once
#include "IProcessingService.hpp"
#include "ISocket.hpp"
#include "TableService.hpp"

#include <atomic>

#include "ReplicaTableService.hpp"

class ProcessingServiceImpl : public IProcessingService {
public:
    ProcessingServiceImpl(std::shared_ptr<ISocket> socket,
                          std::shared_ptr<TableService> table,
                          std::shared_ptr<ReplicaTableService> replica_table);

    void handleRequest(const Packet& request, const sockaddr_in& addr) override;
    void handleUpdateReplicaRequest(const Packet& request, const sockaddr_in &addr) const;
    void handleElectionRequest(const Packet& request, const sockaddr_in &addr) const;

private:
    std::shared_ptr<ISocket> socket;
    std::shared_ptr<TableService> table;
    std::shared_ptr<ReplicaTableService> replica_table;
    std::atomic<uint64_t> totalRequests = 0;
    std::atomic<uint64_t> totalSum = 0;
};
