#pragma once
#include "ISocket.hpp"

class UdpSocket : public ISocket {
    int sockfd;

public:
    UdpSocket();
    ~UdpSocket();

    void open();
    void bind(uint16_t port) override;
    void sendTo(const std::vector<uint8_t>& data, const sockaddr_in& to) override;
    std::vector<uint8_t> receiveFrom(sockaddr_in& from) override;
    int getRawSocket() const { return sockfd; }
};
