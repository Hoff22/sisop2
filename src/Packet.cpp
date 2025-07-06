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
            payload_size = sizeof(uint32_t) // replica_table_size
                         + replicaTable.replica_table_size * sizeof(ReplicaInfo)
                         + sizeof(int) // client_table_size
                         + replicaTable.client_table_size * sizeof(ClientInfo)
                         + replicaTable.client_table_size * (sizeof(uint32_t) + sizeof(uint16_t)); // client_index array
            break;
        case PacketType::REQUEST_REPLICATION:
            payload_size = sizeof(RequestReplicationPayload);
            break;
        default:
            payload_size = 0;
    }

    buffer.resize(sizeof(uint16_t) + sizeof(uint32_t) + payload_size);
    size_t offset = 0;

    // Packet type
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
        // --- Replica table ---
        uint32_t table_size_net = htonl(replicaTable.replica_table_size);
        std::memcpy(buffer.data() + offset, &table_size_net, sizeof(table_size_net));
        offset += sizeof(table_size_net);

        for (uint32_t i = 0; i < replicaTable.replica_table_size; ++i) {
            const ReplicaInfo& info = replicaTable.replica_table[i];
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

        // --- Client table ---
        int client_table_size_net = htonl(replicaTable.client_table_size);
        std::memcpy(buffer.data() + offset, &client_table_size_net, sizeof(client_table_size_net));
        offset += sizeof(client_table_size_net);

        for (int i = 0; i < replicaTable.client_table_size; ++i) {
            const ClientInfo& client = replicaTable.client_table[i];
            uint32_t last_seq_net = htonl(client.last_sequence);
            uint64_t last_sum_net = htobe64(client.last_sum);
            uint64_t last_req_net = htobe64(client.last_numreq);

            std::memcpy(buffer.data() + offset, &last_seq_net, sizeof(last_seq_net));
            offset += sizeof(last_seq_net);
            std::memcpy(buffer.data() + offset, &last_sum_net, sizeof(last_sum_net));
            offset += sizeof(last_sum_net);
            std::memcpy(buffer.data() + offset, &last_req_net, sizeof(last_req_net));
            offset += sizeof(last_req_net);
        }

        // --- Client index array ---
        for (int i = 0; i < replicaTable.client_table_size; ++i) {
            uint32_t key_net = htonl(replicaTable.client_index[i].first);
            uint16_t val_net = htons(replicaTable.client_index[i].second);

            std::memcpy(buffer.data() + offset, &key_net, sizeof(key_net));
            offset += sizeof(key_net);
            std::memcpy(buffer.data() + offset, &val_net, sizeof(val_net));
            offset += sizeof(val_net);
        }
    } else if (type == PacketType::REQUEST_REPLICATION) {
        uint32_t ip_net = htonl(requestReplication.ip);
        uint16_t port_net = htons(requestReplication.port);
        uint32_t seqn_net = htonl(requestReplication.seqn);
        uint64_t sum_net = htobe64(requestReplication.newSum);
        uint64_t numreq_net = htobe64(requestReplication.numreq);

        std::memcpy(buffer.data() + offset, &ip_net, sizeof(ip_net));
        offset += sizeof(ip_net);
        std::memcpy(buffer.data() + offset, &port_net, sizeof(port_net));
        offset += sizeof(port_net);
        std::memcpy(buffer.data() + offset, &seqn_net, sizeof(seqn_net));
        offset += sizeof(seqn_net);
        std::memcpy(buffer.data() + offset, &sum_net, sizeof(sum_net));
        offset += sizeof(sum_net);
        std::memcpy(buffer.data() + offset, &numreq_net, sizeof(numreq_net));
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
        // Deserialize replica_table_size
        if (data.size() < offset + sizeof(uint32_t)) {
            throw std::runtime_error("Missing replica_table_size");
        }

        uint32_t table_size_net;
        std::memcpy(&table_size_net, data.data() + offset, sizeof(table_size_net));
        packet.replicaTable.replica_table_size = ntohl(table_size_net);
        offset += sizeof(table_size_net);

        // Deserialize replica_table
        size_t expected_replica_size = packet.replicaTable.replica_table_size * sizeof(ReplicaInfo);
        if (data.size() < offset + expected_replica_size) {
            throw std::runtime_error("Incomplete ReplicaInfo table in packet");
        }

        packet.replicaTable.replica_table = new ReplicaInfo[packet.replicaTable.replica_table_size];
        for (uint32_t i = 0; i < packet.replicaTable.replica_table_size; ++i) {
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

            packet.replicaTable.replica_table[i] = info;
        }

        // Deserialize client_table_size
        if (data.size() < offset + sizeof(int)) {
            throw std::runtime_error("Missing client_table_size");
        }

        int client_table_size_net;
        std::memcpy(&client_table_size_net, data.data() + offset, sizeof(client_table_size_net));
        packet.replicaTable.client_table_size = ntohl(client_table_size_net);
        offset += sizeof(client_table_size_net);

        // Deserialize client_table
        packet.replicaTable.client_table = new ClientInfo[packet.replicaTable.client_table_size];
        for (int i = 0; i < packet.replicaTable.client_table_size; ++i) {
            ClientInfo info;

            uint32_t last_seq_net;
            uint64_t last_sum_net;
            uint64_t last_req_net;

            std::memcpy(&last_seq_net, data.data() + offset, sizeof(last_seq_net));
            offset += sizeof(last_seq_net);
            std::memcpy(&last_sum_net, data.data() + offset, sizeof(last_sum_net));
            offset += sizeof(last_sum_net);
            std::memcpy(&last_req_net, data.data() + offset, sizeof(last_req_net));
            offset += sizeof(last_req_net);

            info.last_sequence = ntohl(last_seq_net);
            info.last_sum = be64toh(last_sum_net);
            info.last_numreq = be64toh(last_req_net);

            packet.replicaTable.client_table[i] = info;
        }

        // Deserialize client_index
        packet.replicaTable.client_index = new std::pair<uint32_t, uint16_t>[packet.replicaTable.client_table_size];
        for (int i = 0; i < packet.replicaTable.client_table_size; ++i) {
            uint32_t key_net;
            uint16_t val_net;

            std::memcpy(&key_net, data.data() + offset, sizeof(key_net));
            offset += sizeof(key_net);
            std::memcpy(&val_net, data.data() + offset, sizeof(val_net));
            offset += sizeof(val_net);

            packet.replicaTable.client_index[i].first = ntohl(key_net);
            packet.replicaTable.client_index[i].second = ntohs(val_net);
        }
    } else if (type == PacketType::REQUEST_REPLICATION) {
        if (data.size() < offset + sizeof(RequestReplicationPayload)) {
            throw std::runtime_error("Invalid REQUEST_REPLICATION packet size");
        }

        uint32_t ip_net;
        uint16_t port_net;
        uint32_t seqn_net;
        uint64_t sum_net;
        uint64_t numreq_net;

        std::memcpy(&ip_net, data.data() + offset, sizeof(ip_net));
        offset += sizeof(ip_net);
        std::memcpy(&port_net, data.data() + offset, sizeof(port_net));
        offset += sizeof(port_net);
        std::memcpy(&seqn_net, data.data() + offset, sizeof(seqn_net));
        offset += sizeof(seqn_net);
        std::memcpy(&sum_net, data.data() + offset, sizeof(sum_net));
        offset += sizeof(sum_net);
        std::memcpy(&numreq_net, data.data() + offset, sizeof(numreq_net));
        offset += sizeof(numreq_net);

        packet.requestReplication.ip = ntohl(ip_net);
        packet.requestReplication.port = ntohs(port_net);
        packet.requestReplication.seqn = ntohl(seqn_net);
        packet.requestReplication.newSum = be64toh(sum_net);
        packet.requestReplication.numreq = be64toh(numreq_net);
    }

    return packet;
}
