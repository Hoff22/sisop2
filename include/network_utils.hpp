#pragma once

#include <string>
#include <netinet/in.h>

namespace hun {
	constexpr uint16_t BUFF_LEN = 1024;

	struct portMessage {
		sockaddr_in addr;
		uint32_t data_len;
		char buffer[BUFF_LEN];
	};

	std::string timeNowAsString();
	std::string IPv4addrToString(uint32_t addr);

	class gateKeeper {
		int sockfd;
		int bound_to;

	public:
		struct { uint32_t address; uint16_t port; } last_received;

		gateKeeper();
		~gateKeeper();

		int bindTo(const uint32_t ipv4, const uint16_t port);
		int send(const uint32_t ipv4, const uint16_t port, const char* data, const uint32_t size);
		int receive(const uint32_t ipv4, const uint16_t port, portMessage* res);
		int listen(portMessage* res);
	};
}
