#include <atomic>
#include <iostream>
#include <string>
#include <arpa/inet.h>

#include "../include/ProcessingServiceImpl.hpp"
#include "../include/Logger.hpp"
#include "../include/TimeUtils.hpp"

static std::atomic<uint64_t> totalRequests = 0;
static std::atomic<uint64_t> totalSum = 0;

ProcessingServiceImpl::ProcessingServiceImpl(std::shared_ptr<ISocket> socket)
    : socket(std::move(socket)) {}

void ProcessingServiceImpl::handleRequest(const Packet& request, const sockaddr_in& addr) {
    const uint32_t ip = addr.sin_addr.s_addr;
    const uint16_t port = ntohs(addr.sin_port);

    ClientInfo& info = clientTable.getOrInsert(ip, port);

    const std::string now = getFormattedTime();
    const std::string client_ip = std::string(inet_ntoa(addr.sin_addr));

    if (request.seqn <= info.last_sequence) {
        // Resend old response
        Packet ack(PacketType::REQUEST_ACK, request.seqn);
        ack.ack.total_sum = info.last_sum;
        ack.ack.num_requests = totalRequests.load();

        std::cout << now << " client " << client_ip
                  << " DUP!! id_req " << request.seqn
                  << " value " << request.request.value
                  << " num_reqs " << totalRequests
                  << " total_sum " << totalSum << std::endl;

        socket->sendTo(ack.serialize(), addr);
        return;
    }

    // update state
    info.last_sum += request.request.value;
    info.last_sequence = request.seqn;

    ++totalRequests;
    totalSum += request.request.value;

    Packet ack(PacketType::REQUEST_ACK, request.seqn);
    ack.ack.total_sum = totalSum;
    ack.ack.num_requests = totalRequests;

    std::cout << now << " client " << client_ip
              << " id_req " << request.seqn
              << " value " << request.request.value
              << " num_reqs " << totalRequests
              << " total_sum " << totalSum << std::endl;

    socket->sendTo(ack.serialize(), addr);
}
