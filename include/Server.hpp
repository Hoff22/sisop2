#pragma once
#include "ISocket.hpp"
#include "RequestDispatcher.hpp"
#include "TableService.hpp"
#include <memory>

class Server {
    int server_id;
    std::shared_ptr<ISocket> socket;
    const std::shared_ptr<RequestDispatcher> dispatcher;
    std::shared_ptr<TableService> client_table;

    bool discover(uint16_t port);
    void worker(std::shared_ptr<ISocket> socket);

public:
    Server(int id, std::shared_ptr<ISocket> socket,
           const std::shared_ptr<RequestDispatcher>& request_dispatcher,
           std::shared_ptr<TableService> client_table);

    void start(uint16_t port);
};
