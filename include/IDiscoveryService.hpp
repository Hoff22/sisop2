#pragma once
#include <netinet/in.h>


class IDiscoveryService {
public:
    virtual ~IDiscoveryService() = default;
    virtual void handleRequest(const sockaddr_in &clientAddr, bool isManager) = 0; // server
};
