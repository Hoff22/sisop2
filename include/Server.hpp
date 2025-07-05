#pragma once
#include "ISocket.hpp"
#include "RequestDispatcher.hpp"
#include <memory>

class Server {
    int server_id;
    std::shared_ptr<ISocket> socket;
    const std::shared_ptr<RequestDispatcher> dispatcher;

    bool discover(uint16_t port);
    void worker(std::shared_ptr<ISocket> socket);

public:
    Server(int id, std::shared_ptr<ISocket> socket,
           const std::shared_ptr<RequestDispatcher>& request_dispatcher);

    void start(uint16_t port);
};
