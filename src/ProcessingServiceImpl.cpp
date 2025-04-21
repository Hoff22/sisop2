#include "../include/ProcessingServiceImpl.hpp"
#include "../include/TimeUtils.hpp"
#include <arpa/inet.h>

#include "../include/Client.hpp"

ProcessingServiceImpl::ProcessingServiceImpl(std::shared_ptr<ISocket> socket,
                                             std::shared_ptr<TableService> table)
    : socket(std::move(socket)), table(std::move(table)) {}

void ProcessingServiceImpl::handleRequest(const Packet& request, const sockaddr_in& addr) {
    const uint32_t ip = addr.sin_addr.s_addr;
    const uint16_t port = ntohs(addr.sin_port);

    const bool isDup = table->isDuplicate(ip, port, request.seqn);

    //TODO change this to be done only by the discovery service
    ClientInfo &info = table->getOrInsert(ip, port);

    uint32_t ackSeqn;
    uint64_t ackSum;

    if (isDup) {
        // Req with seqn already processed, send latest state to the client
        ackSeqn = info.last_sequence;
        ackSum  = info.last_sum;

        // Notify InterfaceService with flag isDuplicate
        table->update(ip, port, request.seqn, ackSum, request.request.value);  // não altera estado real
    } else {
        ++totalRequests;
        totalSum += request.request.value;

        ackSeqn = request.seqn;
        ackSum = totalSum;

        table->update(ip, port, ackSeqn, ackSum, request.request.value);
    }

    Packet ack(PacketType::REQUEST_ACK, ackSeqn);
    ack.ack.total_sum = ackSum;
    ack.ack.num_requests = totalRequests;

    socket->sendTo(ack.serialize(), addr);
}
