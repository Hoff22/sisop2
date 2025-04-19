#pragma once
#include <netinet/in.h>

class IDiscoveryService {
public:
    virtual ~IDiscoveryService() = default;
    virtual void listenForDiscoveryRequests() = 0; // server
    virtual sockaddr_in findServer() = 0; // client
};
