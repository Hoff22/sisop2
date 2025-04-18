#include <bits/stdc++.h>
#include <chrono>
#include <ctime>  
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <errno.h> 

namespace hun{

	constexpr uint16_t BUFF_LEN = 1024;

	struct portMessage{
		sockaddr_in addr;
		uint32_t data_len;
		char buffer[BUFF_LEN];
	};

	std::string timeNowAsString();
	std::string IPv4addrToString(uint32_t addr);

	class gateKeeper{

		int sockfd;
		int bound_to;

	public:
		
		struct {uint32_t address; uint16_t port;} last_recieved;

		gateKeeper() :
			sockfd(socket(AF_INET, SOCK_DGRAM, 0)),
			bound_to(-1)
		{
		    if (this->sockfd < 0) { 
		        std::cerr << "[ERR] failed to initialize socket: " << strerror(errno) << std::endl;
		        exit(EXIT_FAILURE); 
		    } 
		}

		~gateKeeper(){
			close(sockfd);
		}

		int send(uint32_t ipv4, uint16_t port, const char* data, uint32_t size){
			sockaddr_in dst_info;
			memset(&dst_info, 0, sizeof(dst_info));
			dst_info.sin_family = AF_INET;
			dst_info.sin_port = port;
			dst_info.sin_addr.s_addr = ipv4;

			int amt = sendto(this->sockfd, (const char *)data, size, 0, (const struct sockaddr *) &dst_info,  sizeof(dst_info));
			if(amt < 0){
		    	std::cerr << "[ERR] failed to send message: " << strerror(errno) << std::endl;
		    	exit(EXIT_FAILURE);
		    } 

#ifdef _DEBUG__
		    std::cout << "[" << hun::timeNowAsString() << "] Sending message to " << hun::IPv4addrToString(ipv4) << ":" << htons(port) << std::endl;
#endif

		    return amt;
		}

		// listens on *port* until a message is recieved
		// returns number of bytes read
		// returns on *res* the contents of the message
		// address of message source returned on *res.sin_addr.s_addr*
		int recieve(uint32_t ipv4, uint16_t port, portMessage* res){
			memset(res, 0, sizeof(*res));

			res->addr.sin_family = AF_INET;
			res->addr.sin_port = port;
			res->addr.sin_addr.s_addr = ipv4;

			socklen_t len = sizeof(res->addr); 
			int n = recvfrom(this->sockfd, (char*)res->buffer, BUFF_LEN, 0, (sockaddr *) &res->addr, &len);
			res->data_len = n;

			if(n < 0){
				std::cerr << "[ERR] failed to recieve message: " << strerror(errno) << std::endl;
		    	exit(EXIT_FAILURE);
			}

			this->last_recieved = {res->addr.sin_addr.s_addr, res->addr.sin_port};

#ifdef _DEBUG__
			std::cout << "[" << hun::timeNowAsString() << "] Recieved message from " << hun::IPv4addrToString(res->addr.sin_addr.s_addr) << ":" << htons(res->addr.sin_port) << std::endl;
#endif
			
			return n;
		}

		int bindTo(uint32_t ipv4, uint16_t port){
			sockaddr_in sv_info;
			memset(&sv_info, 0, sizeof(sv_info));
			sv_info.sin_family = AF_INET;
			sv_info.sin_port = port;
			sv_info.sin_addr.s_addr = ipv4;

			int res = bind(this->sockfd, (const struct sockaddr *)&sv_info, sizeof(sv_info));

			if(res < 0){
		    	std::cerr << "[ERR] failed to bind: " << strerror(errno) << std::endl;
		    	exit(EXIT_FAILURE);
		    } 

		    this->bound_to = port;

		    return res;
		}

		int listen(portMessage* res){
			assert(this->bound_to != -1);
			memset(res, 0, sizeof(*res));
			socklen_t len = sizeof(res->addr); 
			int n = recvfrom(this->sockfd, (char*)res->buffer, BUFF_LEN, 0, (sockaddr *) &res->addr, &len);
			res->data_len = n;
			if(n < 0){
				std::cerr << "[ERR] failed to recieve message: " << strerror(errno) << std::endl;
		    	exit(EXIT_FAILURE);
			}

			this->last_recieved = {res->addr.sin_addr.s_addr, res->addr.sin_port};

#ifdef _DEBUG__
			std::cout << "[" << hun::timeNowAsString() << "] Recieved message from " << hun::IPv4addrToString(res->addr.sin_addr.s_addr) << ":" << htons(res->addr.sin_port) << std::endl;
#endif

			return n;
		}
	};

	std::string timeNowAsString(){
 		const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
 		const std::time_t t_c = std::chrono::system_clock::to_time_t(now);
 		char buffer[80];
 		std::strftime(buffer, 80, "%Y-%m-%d-%H:%M:%S", localtime(&t_c));
    	std::string result(buffer);
    	return result;
	}

	std::string IPv4addrToString(uint32_t addr){
		addr = htonl(addr);
		std::string str = std::to_string((addr>>24) & 0xFF) + "." + std::to_string((addr>>16) & 0xFF) + "." + std::to_string((addr>>8) & 0xFF) + "." + std::to_string((addr) & 0xFF);
		return str;
	}

}