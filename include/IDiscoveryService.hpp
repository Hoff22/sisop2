#pragma once
#include <netinet/in.h>


class IDiscoveryService {
public:
    virtual ~IDiscoveryService() = default;
    virtual void handleRequest(const sockaddr_in &clientAddr) = 0; // server
};
