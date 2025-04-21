#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>

#include "../include/Client.hpp"
#include "../include/Logger.hpp"
#include "../include/TimeUtils.hpp"


Client::Client(std::shared_ptr<ISocket> sock) : socket(std::move(sock)) {}

bool Client::discover(uint16_t port) {
    const int enable = 1;
    setsockopt(socket->getRawSocket(), SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable));

    const Packet discovery(PacketType::DISCOVERY, 0);
    const auto data = discovery.serialize();

    sockaddr_in broadcastAddr{};
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(port);
    broadcastAddr.sin_addr.s_addr = inet_addr("255.255.255.255");

    LOG_INFO("[CLIENT] Broadcasting discovery...");
    socket->sendTo(data, broadcastAddr);

    const std::vector<uint8_t> response = socket->receiveFrom(serverAddr);
    if (response.empty()) {
        LOG_WARN("[CLIENT] Discovery timed out.");
        return false;
    }

    try {
        if (const Packet ack = Packet::deserialize(response); ack.type == PacketType::DISCOVERY_ACK) {
            std::cout << getFormattedTime()
                    << " server_addr " << inet_ntoa(serverAddr.sin_addr) << std::endl;
            return true;
        }
    } catch (...) {
        LOG_ERROR("[CLIENT] Failed to parse discovery response.");
    }

    return false;
}

bool Client::connectToServer(const uint16_t port) {
    for (int attempt = 0; attempt < 10; ++attempt) {
        if (discover(port)) return true;
        LOG_WARN("[CLIENT] Retry discovery...");
        sleep(1);
    }
    return false;
}

void Client::run() {
    int value;

    while (std::cin >> value) {
        Packet req(PacketType::REQUEST, sequence);
        req.request.value = static_cast<uint32_t>(value);
        auto data = req.serialize();

        bool acknowledged = false;

        while (!acknowledged) {
            socket->sendTo(data, serverAddr);

            sockaddr_in replyAddr{};
            std::vector<uint8_t> response = socket->receiveFrom(replyAddr);

            if (response.empty()) {
                LOG_WARN("[CLIENT] Timeout. Retrying...");
                continue;
            }

            try {
                const Packet ack = Packet::deserialize(response);

                if (ack.type != PacketType::REQUEST_ACK) {
                    LOG_WARN("[CLIENT] Invalid response type.");
                    continue;
                }

                if (ack.seqn == sequence) {
                    LOG_INFO("[CLIENT] Server confirmed request #" + std::to_string(ack.seqn) +
                        ", sum = " + std::to_string(ack.ack.total_sum));
                    std::cout << getFormattedTime()
                            << " server " << inet_ntoa(serverAddr.sin_addr)
                            << " id_req " << ack.seqn
                            << " value " << value
                            << " num_reqs " << ack.ack.num_requests
                            << " total_sum " << ack.ack.total_sum << std::endl;

                    acknowledged = true;
                    sequence++;
                } else if (ack.seqn < sequence) {
                    LOG_WARN("[CLIENT] Duplicate ACK received (ack.seqn=" + std::to_string(ack.seqn) +
                        ", expected=" + std::to_string(sequence) + "). Resending request...");
                } else {
                    LOG_WARN("[CLIENT] Server already processed this request. Last confirmed: " +
                        std::to_string(ack.seqn) + ". Aborting this request.");
                    acknowledged = true;
                    sequence++;
                }

            } catch (const std::exception& e) {
                LOG_ERROR("[CLIENT] Failed to parse ACK: " + std::string(e.what()));
            }
        }
    }
}
