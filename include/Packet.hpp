#pragma once
#include <vector>
#include <cstdint>

#include "ClientInfo.hpp"
#include "ReplicaInfo.hpp"

enum class PacketType : uint16_t {
    OTHER,
    DISCOVERY,
    DISCOVERY_ACK,
    SERVER_DISCOVERY,
    SERVER_DISCOVERY_ACK,
    REQUEST,
    REQUEST_ACK,
    NUM_TYPES,
    REQUEST_REPLICATION,
    REQUEST_REPLICATION_ACK,
};

struct RequestPayload {
    uint32_t value;
};

struct AckPayload {
    uint32_t seqn;
    uint32_t num_requests;
    uint64_t total_sum;
};

struct ReplicaTableAckPayload {
    uint32_t replica_table_size;
    ReplicaInfo* replica_table;

    int client_table_size;
    ClientInfo* client_table;
    std::pair<uint32_t, uint16_t>* client_index;
};

struct RequestReplicationPayload {
    uint32_t ip;
    uint16_t port;
    uint32_t seqn;
    uint64_t newSum;
    uint64_t numreq;
};

class Packet {
public:
    PacketType type;
    uint32_t seqn;

    union {
        RequestPayload request;
        AckPayload ack;
        ReplicaTableAckPayload replicaTable;
        RequestReplicationPayload requestReplication;
    };

    Packet();
    Packet(PacketType type, uint32_t seqn);
    std::vector<uint8_t> serialize() const;
    static Packet deserialize(const std::vector<uint8_t>& data);
};