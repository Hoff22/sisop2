#include "../include/ClientInfo.hpp"
#include "../include/ClientTable.hpp"

ClientInfo& ClientTable::getOrInsert(uint32_t ip, uint16_t port) {
    std::lock_guard<std::mutex> lock(mutex);
    return clients[{ip, port}];
}

bool ClientTable::isDuplicate(uint32_t ip, uint16_t port, uint32_t seqn) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = clients.find({ip, port});
    return it != clients.end() && it->second.last_sequence >= seqn;
}

void ClientTable::update(uint32_t ip, uint16_t port, uint32_t seqn, uint64_t sum) {
    std::lock_guard<std::mutex> lock(mutex);
    clients[{ip, port}].last_sequence = seqn;
    clients[{ip, port}].last_sum = sum;
}
