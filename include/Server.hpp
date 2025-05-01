#pragma once
#include "ISocket.hpp"
#include "RequestDispatcher.hpp"
#include <memory>

class Server {
    std::shared_ptr<ISocket> socket;
    const std::shared_ptr<RequestDispatcher> dispatcher;

public:
    Server(std::shared_ptr<ISocket> socket,
           const std::shared_ptr<RequestDispatcher>& request_dispatcher);

    void start();
};
