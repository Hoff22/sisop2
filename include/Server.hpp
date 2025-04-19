#pragma once
#include "ISocket.hpp"
#include "IDiscoveryService.hpp"
#include "IProcessingService.hpp"
#include <memory>

class Server {
    std::shared_ptr<ISocket> socket;
    std::shared_ptr<IDiscoveryService> discoveryService;
    std::shared_ptr<IProcessingService> processingService;

public:
    Server(std::shared_ptr<ISocket> socket,
           std::shared_ptr<IDiscoveryService> discovery,
           std::shared_ptr<IProcessingService> processing);

    void start();
    void startDiscovery();
};
