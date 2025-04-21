#include "../include/InterfaceService.hpp"

#include <iostream>
#include <arpa/inet.h>
#include <netinet/in.h>

void InterfaceService::onClientInserted(uint32_t ip, uint16_t port) {
    // This is optional — we can use it if the spec requires it
    // std::cout << getFormattedTime() << " new client " << ip_str << ":" << port << std::endl;
}

void InterfaceService::onRequestProcessed(const std::string& timestamp,
                                          const std::string& clientIp,
                                          uint32_t seqn,
                                          uint32_t value,
                                          uint64_t numRequests,
                                          uint64_t totalSum,
                                          bool isDuplicate) {
    std::cout << timestamp
              << " client " << clientIp
              << (isDuplicate ? " DUP!!" : "")
              << " id_req " << seqn
              << " value " << value
              << " num_reqs " << numRequests
              << " total_sum " << totalSum
              << std::endl;
}
