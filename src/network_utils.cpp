#include "../include/network_utils.hpp"

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cerrno>

namespace hun {

gateKeeper::gateKeeper()
    : sockfd(socket(AF_INET, SOCK_DGRAM, 0)), bound_to(-1), last_received({0,0})
{
    if (this->sockfd < 0) {
        std::cerr << "[ERR] failed to initialize socket: " << strerror(errno) << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

gateKeeper::~gateKeeper() {
    close(sockfd);
}

int gateKeeper::bindTo(const uint32_t ipv4, const uint16_t port) {
    sockaddr_in sv_info{};
    sv_info.sin_family = AF_INET;
    sv_info.sin_port = port;
    sv_info.sin_addr.s_addr = ipv4;

    const int res = ::bind(this->sockfd, reinterpret_cast<const struct sockaddr*>(&sv_info), sizeof(sv_info));
    if (res < 0) {
        std::cerr << "[ERR] failed to bind: " << strerror(errno) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    this->bound_to = port;
    return res;
}

int gateKeeper::send(const uint32_t ipv4, const uint16_t port, const char* data, const uint32_t size) {
    sockaddr_in dst_info{};
    dst_info.sin_family = AF_INET;
    dst_info.sin_port = port;
    dst_info.sin_addr.s_addr = ipv4;

    const int amt = sendto(this->sockfd, data, size, 0,
                     reinterpret_cast<const struct sockaddr*>(&dst_info),
                     sizeof(dst_info));

    if (amt < 0) {
        std::cerr << "[ERR] failed to send message: " << strerror(errno) << std::endl;
        std::exit(EXIT_FAILURE);
    }

#ifdef HUN_DEBUG
    std::cout << "[" << timeNowAsString() << "] Sending message to "
              << IPv4addrToString(ipv4) << ":" << htons(port) << std::endl;
#endif

    return amt;
}

int gateKeeper::receive(const uint32_t ipv4, const uint16_t port, portMessage* res) {
    std::memset(res, 0, sizeof(*res));

    res->addr.sin_family = AF_INET;
    res->addr.sin_port = port;
    res->addr.sin_addr.s_addr = ipv4;

    socklen_t len = sizeof(res->addr);
    int n = recvfrom(this->sockfd, res->buffer, BUFF_LEN, 0,
                     reinterpret_cast<sockaddr*>(&res->addr), &len);

    if (n < 0) {
        std::cerr << "[ERR] failed to receive message: " << strerror(errno) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    res->data_len = n;
    this->last_received = {res->addr.sin_addr.s_addr, res->addr.sin_port};

#ifdef HUN_DEBUG
    std::cout << "[" << timeNowAsString() << "] Received message from "
              << IPv4addrToString(res->addr.sin_addr.s_addr) << ":" << htons(res->addr.sin_port) << std::endl;
#endif

    return n;
}

int gateKeeper::listen(portMessage* res) {
    if (this->bound_to == -1) {
        std::cerr << "[ERR] socket not bound before listen()" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    std::memset(res, 0, sizeof(*res));

    socklen_t len = sizeof(res->addr);
    const int n = recvfrom(this->sockfd, res->buffer, BUFF_LEN, 0,
                     reinterpret_cast<sockaddr*>(&res->addr), &len);

    if (n < 0) {
        std::cerr << "[ERR] failed to receive message: " << strerror(errno) << std::endl;
        std::exit(EXIT_FAILURE);
    }

    res->data_len = n;
    this->last_received = {res->addr.sin_addr.s_addr, res->addr.sin_port};

#ifdef HUN_DEBUG
    std::cout << "[" << timeNowAsString() << "] Received message from "
              << IPv4addrToString(res->addr.sin_addr.s_addr) << ":" << htons(res->addr.sin_port) << std::endl;
#endif

    return n;
}

std::string timeNowAsString() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d-%H:%M:%S", std::localtime(&t_c));
    return std::string(buffer);
}

std::string IPv4addrToString(uint32_t addr) {
    addr = htonl(addr);
    return std::to_string((addr >> 24) & 0xFF) + "." +
           std::to_string((addr >> 16) & 0xFF) + "." +
           std::to_string((addr >> 8) & 0xFF) + "." +
           std::to_string(addr & 0xFF);
}

} // namespace hun
