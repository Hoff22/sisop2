#include "../include/UdpSocket.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>

UdpSocket::UdpSocket() : sockfd(-1) {}

UdpSocket::~UdpSocket() {
    if (sockfd != -1) {
        close(sockfd);
    }
}

void UdpSocket::open() {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        throw std::runtime_error("Failed to create socket");
    }
}

void UdpSocket::bind(uint16_t port) {
    if (sockfd == -1) {
        open();
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;  // Listen on all interfaces
    addr.sin_port = htons(port);

    if (::bind(sockfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("Bind failed");
    }
}

void UdpSocket::sendTo(const std::vector<uint8_t>& data, const sockaddr_in& to) {
    ssize_t bytes_sent = sendto(sockfd, data.data(), data.size(), 0,
                                reinterpret_cast<const sockaddr*>(&to), sizeof(to));
    if (bytes_sent < 0) {
        perror("sendto");
        std::cerr << "Failed to send data\n";
    }
}


std::vector<uint8_t> UdpSocket::receiveFrom(sockaddr_in& from) {
    std::vector<uint8_t> buffer(1024);
    socklen_t from_len = sizeof(from);

    std::cout << "[UDP] Waiting to receive...\n" << std::flush;
    ssize_t bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                      reinterpret_cast<sockaddr*>(&from), &from_len);
    std::cout << "[UDP] Bytes received: " << bytes_received << "\n";

    
    if (bytes_received < 0) {
        perror("recvfrom");
        std::cerr << "Failed to receive data\n";
        from.sin_addr.s_addr = 0;  // mark as invalid
        return {};
    }

    buffer.resize(bytes_received);
    return buffer;
}