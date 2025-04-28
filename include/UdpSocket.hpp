#pragma once
#include "ISocket.hpp"

class UdpSocket : public ISocket {
    int sockfd;

public:
    UdpSocket();
    ~UdpSocket() override;

    void open();
    void bind(uint16_t port) override;
    void addTimeout() const;
    void sendTo(const std::vector<uint8_t>& data, const sockaddr_in& to) override;
    std::vector<uint8_t> receiveFrom(sockaddr_in& from) override;
    int getRawSocket() const override { return sockfd; }
};
