#include "../include/Packet.hpp"
#include "../include/endian_utils.hpp"
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <arpa/inet.h>


Packet::Packet(const PacketType type, const uint32_t seqn) : type(type), seqn(seqn) {
    // Initialize union manually as needed
    std::memset(&request, 0, sizeof(request));
}

std::vector<uint8_t> Packet::serialize() const {
    std::vector<uint8_t> buffer;

    // Reserve enough space
    buffer.resize(sizeof(uint16_t) + sizeof(uint32_t) + sizeof(AckPayload)); // worst-case size

    size_t offset = 0;

    // Write type
    uint16_t type_net = htons(static_cast<uint16_t>(type));
    std::memcpy(buffer.data() + offset, &type_net, sizeof(type_net));
    offset += sizeof(type_net);

    // Write sequence number
    uint32_t seqn_net = htonl(seqn);
    std::memcpy(buffer.data() + offset, &seqn_net, sizeof(seqn_net));
    offset += sizeof(seqn_net);

    if (type == PacketType::REQUEST) {
        uint32_t value_net = htonl(request.value);
        std::memcpy(buffer.data() + offset, &value_net, sizeof(value_net));
        offset += sizeof(value_net);
        buffer.resize(offset); // trim
    } else if (type == PacketType::REQUEST_ACK) {
        uint32_t ack_seqn_net = htonl(ack.seqn);
        uint32_t num_reqs_net = htonl(ack.num_requests);
        uint64_t sum_net = htobe64(ack.total_sum); // use big endian

        std::memcpy(buffer.data() + offset, &ack_seqn_net, sizeof(ack_seqn_net));
        offset += sizeof(ack_seqn_net);
        std::memcpy(buffer.data() + offset, &num_reqs_net, sizeof(num_reqs_net));
        offset += sizeof(num_reqs_net);
        std::memcpy(buffer.data() + offset, &sum_net, sizeof(sum_net));
        offset += sizeof(sum_net);
        buffer.resize(offset);
    }

    return buffer;
}

Packet Packet::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < sizeof(uint16_t) + sizeof(uint32_t)) {
        throw std::runtime_error("Invalid packet size (too small)");
    }


    size_t offset = 0;

    uint16_t type_net;
    std::memcpy(&type_net, data.data() + offset, sizeof(type_net));
    PacketType type = static_cast<PacketType>(ntohs(type_net));
    offset += sizeof(type_net);

    uint32_t seqn_net;
    std::memcpy(&seqn_net, data.data() + offset, sizeof(seqn_net));
    uint32_t seqn = ntohl(seqn_net);
    offset += sizeof(seqn_net);

    Packet packet(type, seqn);

    if (type == PacketType::REQUEST) {
        if (data.size() < offset + sizeof(uint32_t)) {
            throw std::runtime_error("Invalid REQUEST packet size");
        }
        uint32_t value_net;
        std::memcpy(&value_net, data.data() + offset, sizeof(value_net));
        packet.request.value = ntohl(value_net);
    } else if (type == PacketType::REQUEST_ACK) {
        if (data.size() < offset + sizeof(AckPayload)) {
            throw std::runtime_error("Invalid ACK packet size");
        }

        uint32_t ack_seqn_net;
        uint32_t num_reqs_net;
        uint64_t sum_net;

        std::memcpy(&ack_seqn_net, data.data() + offset, sizeof(ack_seqn_net));
        packet.ack.seqn = ntohl(ack_seqn_net);
        offset += sizeof(ack_seqn_net);

        std::memcpy(&num_reqs_net, data.data() + offset, sizeof(num_reqs_net));
        packet.ack.num_requests = ntohl(num_reqs_net);
        offset += sizeof(num_reqs_net);

        std::memcpy(&sum_net, data.data() + offset, sizeof(sum_net));
        packet.ack.total_sum = be64toh(sum_net);
    }

    return packet;
}

