// endian_utils.hpp
#pragma once
#include <cstdint>

inline uint64_t htobe64(uint64_t host_64) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(host_64);
#else
    return host_64;
#endif
}

inline uint64_t be64toh(uint64_t net_64) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(net_64);
#else
    return net_64;
#endif
}
