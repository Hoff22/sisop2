#pragma once
#include "IProcessingService.hpp"
#include "ISocket.hpp"
#include "TableService.hpp"

#include <atomic>

class ProcessingServiceImpl : public IProcessingService {
public:
    ProcessingServiceImpl(std::shared_ptr<ISocket> socket,
                          std::shared_ptr<TableService> table);

    void handleRequest(const Packet& request, const sockaddr_in& addr) override;

private:
    std::shared_ptr<ISocket> socket;
    std::shared_ptr<TableService> table;
    std::atomic<uint64_t> totalRequests = 0;
    std::atomic<uint64_t> totalSum = 0;
};
