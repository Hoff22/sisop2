#include <bits/stdc++.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "debug_utils.hpp"
#include "network_utils.hpp"

using namespace std;

#define PORT    8080 
#define MAXLINE 1024 
   
const char *hello = "Hello from server";
const uint32_t hello_size = 17;

int extractInt(uint8_t* buffer){
	int res = ((int)buffer[0] << 24) + ((int)buffer[1] << 16) + ((int)buffer[2] << 8) + ((int)buffer[3] << 0);
	return res;
}

long long sum = 0;

// Driver code 
int main(int argc, char** argv) { 

	hut::printList(hut::readArgs(argc, argv));
	cout << endl;

    char buffer[MAXLINE]; 
	
	hun::portMessage pm;
	hun::gateKeeper gk;
	gk.bindTo(htonl(INADDR_ANY), htons(PORT));
	while(gk.listen(&pm)){
		int data = extractInt((uint8_t*)pm.buffer);
		std::cout << "message data: " << data << std::endl;
		gk.send(pm.addr.sin_addr.s_addr, pm.addr.sin_port, hello,hello_size);
	}
}