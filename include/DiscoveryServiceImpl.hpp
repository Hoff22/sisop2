#include "IDiscoveryService.hpp"
#include "ISocket.hpp"
#include "TableService.hpp"

class DiscoveryServiceImpl : public IDiscoveryService {
public:
    DiscoveryServiceImpl(std::shared_ptr<ISocket> socket,
                         std::shared_ptr<TableService> table);

    void listenForDiscoveryRequests() override;
    sockaddr_in findServer() override;

private:
    std::shared_ptr<ISocket> socket;
    std::shared_ptr<TableService> table;
};