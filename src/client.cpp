#include <bits/stdc++.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 

#include "../include/debug_utils.hpp"
#include "../include/network_utils.hpp"

using namespace std;

#define PORT    8080 
#define MAXLINE 1024 

const char *hello = "Hello from client";  
const char *hello_s = "Hello from server";  
const uint32_t hello_size = 17;

int extractInt(uint8_t* buffer){
    int res = ((int)buffer[0] << 24) + ((int)buffer[1] << 16) + ((int)buffer[2] << 8) + ((int)buffer[3] << 0);
    return res;
}

void sendNextInt(){
    
}

// Driver code 
int main(int argc, char** argv) { 

	hut::printList(hut::readArgs(argc, argv));
	cout << endl;
    
    uint32_t server_address;
    uint32_t server_port;

    hun::gateKeeper gk;

    // find server routine:
    gk.send(htonl(INADDR_ANY), htons(PORT), hello, hello_size);
    hun::portMessage pm;
    int n = gk.recieve(htonl(INADDR_ANY), htons(PORT), &pm);

    cout << "LAST_RECIEVED: " << endl << "  address:\t" << hun::IPv4addrToString(gk.last_recieved.address) << endl << "  port:\t\t" << htons(gk.last_recieved.port) << endl;

    pm.buffer[n] = '\0'; 
    std::cout<<"Server: "<<pm.buffer<<std::endl; 

    if(strcmp(pm.buffer, hello_s) == 0){
        cout << "connected!" << endl << endl;
        server_address = gk.last_recieved.address;
        server_port = gk.last_recieved.port;
    }
    else{
        std::cerr << "[ERR] failed to connect to server" << std::endl;
        exit(-1);
    }

    // sending routine
    int x = 0;
    long long sum = 0;

    int k = 1;

    while(cin >> x){
        sum += x;
        
        x = htonl(x);
        gk.send(server_address, server_port, (const char*)&x, sizeof(x));

        n = gk.recieve(server_address, server_port, &pm);

        pm.buffer[n] = '\0'; 
        std::cout<<"Server: "<<pm.buffer<<std::endl; 
    }

    
    
    // cout << "Server addr: " << hun::IPv4addrToString(servaddr.sin_addr.s_addr) << endl;
   
    return 0; 
}