#pragma once
#include "IDiscoveryService.hpp"
#include "ISocket.hpp"
#include <memory>

class DiscoveryServiceImpl : public IDiscoveryService {
    std::shared_ptr<ISocket> socket;

public:
    explicit DiscoveryServiceImpl(std::shared_ptr<ISocket> sock);
    void listenForDiscoveryRequests() override;
    sockaddr_in findServer() override; // Not used on server, just a stub
};
