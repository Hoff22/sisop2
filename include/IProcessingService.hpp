#pragma once
#include "Packet.hpp"
#include <netinet/in.h>

class IProcessingService {
public:
    virtual ~IProcessingService() = default;
    virtual void handleRequest(const Packet& request, const sockaddr_in& clientAddress) = 0;
};
