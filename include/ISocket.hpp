#pragma once
#include <vector>
#include <netinet/in.h>

class ISocket {
public:
    virtual ~ISocket() = default;
    virtual void bind(uint16_t port) = 0;
    virtual void sendTo(const std::vector<uint8_t>& data, const sockaddr_in& to) = 0;
    virtual std::vector<uint8_t> receiveFrom(sockaddr_in& from) = 0;
};
