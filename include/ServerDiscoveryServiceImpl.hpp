#pragma once
#include "IProcessingService.hpp"
#include "ISocket.hpp"
#include "ReplicaTableService.hpp"

class ServerDiscoveryServiceImpl : public IProcessingService {
public:
    ServerDiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                         std::shared_ptr<ReplicaTableService> table);

    void handleRequest(const Packet& request, const sockaddr_in &clientAddr) override;

    std::shared_ptr<ISocket> socket;
    std::shared_ptr<ReplicaTableService> table;
};