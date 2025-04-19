#pragma once
#include <vector>

enum class PacketType : uint16_t {
    DISCOVERY = 1,
    DISCOVERY_ACK = 2,
    REQUEST = 3,
    REQUEST_ACK = 4
};

struct RequestPayload {
    uint32_t value;
};

struct AckPayload {
    uint32_t seqn;
    uint32_t num_requests;
    uint64_t total_sum;
};

class Packet {
public:
    PacketType type;
    uint32_t seqn;

    union {
        RequestPayload request;
        AckPayload ack;
    };

    Packet(PacketType type, uint32_t seqn);
    std::vector<uint8_t> serialize() const;
    static Packet deserialize(const std::vector<uint8_t>& data);
};
