#include "../include/UdpSocket.hpp"
#include "../include/Logger.hpp"

#include <cerrno>
#include <stdexcept>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

UdpSocket::UdpSocket() : sockfd(-1) {
}

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
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (::bind(sockfd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
        throw std::runtime_error("Bind failed");
    }

    LOG_INFO("[UDP] Socket bound to port " + std::to_string(port));
}

void UdpSocket::addTimeout() const {
    struct timeval tv{};
    tv.tv_sec = 0;
    tv.tv_usec = 10;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
}

void UdpSocket::sendTo(const std::vector<uint8_t> &data, const sockaddr_in &to) {
    ssize_t bytes_sent = sendto(sockfd, data.data(), data.size(), 0,
                                reinterpret_cast<const sockaddr *>(&to), sizeof(to));
    if (bytes_sent < 0) {
        perror("sendto");
        LOG_ERROR("[UDP] Failed to send data");
    } else {
        LOG_INFO(
            "[UDP] Sent " + std::to_string(bytes_sent) + " bytes to " + std::string(inet_ntoa(to.sin_addr)) + ":" + std
            ::to_string(ntohs(to.sin_port)));
    }
}

std::vector<uint8_t> UdpSocket::receiveFrom(sockaddr_in &from) {
    std::vector<uint8_t> buffer(1024);
    socklen_t from_len = sizeof(from);

    LOG_INFO("[UDP] Waiting to receive...");
    const ssize_t bytes_received = recvfrom(sockfd, buffer.data(), buffer.size(), 0,
                                            reinterpret_cast<sockaddr *>(&from), &from_len);

    if (bytes_received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_WARN("[UDP] Timeout while waiting for data.");
        } else {
            perror("recvfrom");
            LOG_ERROR("[UDP] Failed to receive data");
        }

        from.sin_addr.s_addr = 0;
        return {};
    }

    LOG_INFO("[UDP] Bytes received: " + std::to_string(bytes_received));
    buffer.resize(bytes_received);
    return buffer;
}
