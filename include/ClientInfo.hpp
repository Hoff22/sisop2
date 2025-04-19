#pragma once
#include <cstdint>

struct ClientInfo {
    uint32_t last_sequence = 0;
    uint64_t last_sum = 0;
};
