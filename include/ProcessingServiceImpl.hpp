#pragma once

#include "IProcessingService.hpp"
#include "ISocket.hpp"
#include "Packet.hpp"
#include "ClientTable.hpp"
#include <memory>

class ProcessingServiceImpl : public IProcessingService {
    std::shared_ptr<ISocket> socket;
    ClientTable clientTable;

public:
    explicit ProcessingServiceImpl(std::shared_ptr<ISocket> socket);
    void handleRequest(const Packet& request, const sockaddr_in& addr) override;
};
