#pragma once
#include <cstdint>
#include <arpa/inet.h>

struct ReplicaInfo{
	uint32_t ip;
	uint16_t port;
	uint32_t id;

	sockaddr_in replicaToSockaddr() const {
		sockaddr_in addr{};
	    addr.sin_family = AF_INET;
	    addr.sin_port = ntohs(port);
	    addr.sin_addr.s_addr = ntohl(ip);
	    return addr;
	}
};