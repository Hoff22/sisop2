#include "../include/Packet.hpp"
#include "../include/endian_utils.hpp"
#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <arpa/inet.h>

Packet::Packet(const PacketType type, const uint32_t seqn) : type(type), seqn(seqn) {
    std::memset(&request, 0, sizeof(request));  // clears all union fields safely
}

Packet::Packet() : type(PacketType::OTHER), seqn(0), request({}) {
}

std::vector<uint8_t> Packet::serialize() const {
    std::vector<uint8_t> buffer;

    size_t payload_size = 0;

    switch (type) {
        case PacketType::REQUEST:
            payload_size = sizeof(RequestPayload);
            break;
        case PacketType::REQUEST_ACK:
            payload_size = sizeof(AckPayload);
            break;
        case PacketType::SERVER_DISCOVERY_ACK:
            payload_size = sizeof(uint32_t) + replicaTable.table_size * sizeof(ReplicaInfo);
            break;
        default:
            payload_size = 0;
    }

    buffer.resize(sizeof(uint16_t) + sizeof(uint32_t) + payload_size);
    size_t offset = 0;

    // Type
    uint16_t type_net = htons(static_cast<uint16_t>(type));
    std::memcpy(buffer.data() + offset, &type_net, sizeof(type_net));
    offset += sizeof(type_net);

    // Sequence number
    uint32_t seqn_net = htonl(seqn);
    std::memcpy(buffer.data() + offset, &seqn_net, sizeof(seqn_net));
    offset += sizeof(seqn_net);

    if (type == PacketType::REQUEST) {
        uint32_t value_net = htonl(request.value);
        std::memcpy(buffer.data() + offset, &value_net, sizeof(value_net));
    } else if (type == PacketType::REQUEST_ACK) {
        uint32_t ack_seqn_net = htonl(ack.seqn);
        uint32_t num_reqs_net = htonl(ack.num_requests);
        uint64_t sum_net = htobe64(ack.total_sum);

        std::memcpy(buffer.data() + offset, &ack_seqn_net, sizeof(ack_seqn_net));
        offset += sizeof(ack_seqn_net);

        std::memcpy(buffer.data() + offset, &num_reqs_net, sizeof(num_reqs_net));
        offset += sizeof(num_reqs_net);

        std::memcpy(buffer.data() + offset, &sum_net, sizeof(sum_net));
    } else if (type == PacketType::SERVER_DISCOVERY_ACK) {
        // First write the table size
        uint32_t table_size_net = htonl(replicaTable.table_size);
        std::memcpy(buffer.data() + offset, &table_size_net, sizeof(table_size_net));
        offset += sizeof(table_size_net);

        // Then write each ReplicaInfo
        for (uint32_t i = 0; i < replicaTable.table_size; ++i) {
            const ReplicaInfo& info = replicaTable.table[i];

            uint32_t ip_net = htonl(info.ip);
            uint16_t port_net = htons(info.port);
            uint16_t id_net = htons(info.id);

            std::memcpy(buffer.data() + offset, &ip_net, sizeof(ip_net));
            offset += sizeof(ip_net);
            std::memcpy(buffer.data() + offset, &port_net, sizeof(port_net));
            offset += sizeof(port_net);
            std::memcpy(buffer.data() + offset, &id_net, sizeof(id_net));
            offset += sizeof(id_net);
        }
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

        uint32_t ack_seqn_net, num_reqs_net;
        uint64_t sum_net;

        std::memcpy(&ack_seqn_net, data.data() + offset, sizeof(ack_seqn_net));
        offset += sizeof(ack_seqn_net);
        std::memcpy(&num_reqs_net, data.data() + offset, sizeof(num_reqs_net));
        offset += sizeof(num_reqs_net);
        std::memcpy(&sum_net, data.data() + offset, sizeof(sum_net));
        offset += sizeof(sum_net);

        packet.ack.seqn = ntohl(ack_seqn_net);
        packet.ack.num_requests = ntohl(num_reqs_net);
        packet.ack.total_sum = be64toh(sum_net);
    } else if (type == PacketType::SERVER_DISCOVERY_ACK) {
        if (data.size() < offset + sizeof(uint32_t)) {
            throw std::runtime_error("Invalid SERVER_DISCOVERY_ACK packet size");
        }

        uint32_t table_size_net;
        std::memcpy(&table_size_net, data.data() + offset, sizeof(table_size_net));
        packet.replicaTable.table_size = ntohl(table_size_net);
        offset += sizeof(table_size_net);

        size_t expected_size = offset + packet.replicaTable.table_size * sizeof(ReplicaInfo);
        if (data.size() < expected_size) {
            throw std::runtime_error("Incomplete ReplicaInfo table in packet");
        }

        packet.replicaTable.table = new ReplicaInfo[packet.replicaTable.table_size];

        for (uint32_t i = 0; i < packet.replicaTable.table_size; ++i) {
            ReplicaInfo info;
            std::memcpy(&info.ip, data.data() + offset, sizeof(info.ip));
            info.ip = ntohl(info.ip);
            offset += sizeof(info.ip);

            std::memcpy(&info.port, data.data() + offset, sizeof(info.port));
            info.port = ntohs(info.port);
            offset += sizeof(info.port);

            std::memcpy(&info.id, data.data() + offset, sizeof(info.id));
            info.id = ntohs(info.id);
            offset += sizeof(info.id);

            packet.replicaTable.table[i] = info;
        }
    }

    return packet;
}
