#pragma once
#include "IProcessingService.hpp"
#include "ISocket.hpp"
#include "ReplicaTableService.hpp"
#include "TableService.hpp"

class ServerDiscoveryServiceImpl : public IProcessingService {
public:
    ServerDiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                         std::shared_ptr<ReplicaTableService> replica_table,
                         std::shared_ptr<TableService> client_table);

    void handleRequest(const Packet& request, const sockaddr_in &clientAddr) override;

    std::shared_ptr<ISocket> socket;
    std::shared_ptr<ReplicaTableService> replica_table;
    std::shared_ptr<TableService> client_table;
};
