#pragma once
#include "ISocket.hpp"
#include "TableService.hpp"
#include "HeartbeatService.hpp"
#include "RequestDispatcher.hpp"
#include <memory>
#include <semaphore>
#include <algorithm>

// class RequestDispatcher;

class Server {
    mutable std::mutex election_lock;

    std::shared_ptr<ISocket> socket;
    const std::shared_ptr<RequestDispatcher> dispatcher;
    std::shared_ptr<TableService> client_table;
    std::shared_ptr<HeartbeatService> heartbeatService;

    bool discover(uint16_t port);

public:
    std::atomic<bool> isManager;
    std::atomic<bool> running_election;
    int server_id;
    Server(int id, std::shared_ptr<ISocket> socket,
           const std::shared_ptr<RequestDispatcher>& request_dispatcher,
           std::shared_ptr<TableService> client_table);

    void start(uint16_t port);
    void notifyNewManager(){
        const Packet ack(PacketType::DISCOVERY_ACK, 0);
        for(int i = 0; i < client_table->getTable().current_clients; i++){
            auto &info = client_table->getTable().client_index[i];
            sockaddr_in clientAddr{};
            clientAddr.sin_addr.s_addr = info.first;
            clientAddr.sin_port = htons(info.second);
            std::cout << "[DEBUG] sending new manager to " << inet_ntoa(clientAddr.sin_addr) << "/" << htons(clientAddr.sin_port) << std::endl;
            socket->sendTo(ack.serialize(), clientAddr);
        }
    }
    bool doElection(){
        // for(int i = 0; i < dispatcher->numThreads-1; i++) dispatcher->semaphore.acquire(); 

        const auto replica_table = dispatcher->serverDiscoveryService->replica_table;

        int ownId = 0;
        for(int i = 0; i < replica_table->getTable().current_replicas; i++){
            const ReplicaInfo &info = replica_table->getTable().table[i];
            if(socket->getSocketIp() == info.ip){
                ownId = info.id;
            }
        }
        
        Packet request_election(PacketType::ELECTION, 0);
        for(int i = 0; i < replica_table->getTable().current_replicas; i++){
            const ReplicaInfo &info = replica_table->getTable().table[i];
            if(info.id <= ownId) continue;
            socket->sendTo(request_election.serialize(), info.replicaToSockaddr());
        }

        // clear buffer until 
        sockaddr_in serverAddr{};
        std::vector<uint8_t> response;
        Packet ack;
        socket->addTimeoutElection();
        do {
            response = socket->receiveFrom(serverAddr);
            if (response.empty()) {
                break;
            }
            ack = Packet::deserialize(response);
            std::cout << "[DEBUG] " << "during election got " << PacketString[(uint16_t)ack.type] << " from " << inet_ntoa(serverAddr.sin_addr) << std::endl;
            if(ack.type == PacketType::ELECTION){
                Packet ack2(PacketType::ELECTION_ACK, 0);
                socket->sendTo(ack2.serialize(), serverAddr);
            }
        } while (ack.type != PacketType::ELECTION_ACK);
        socket->addTimeout();

        // for(int i = 0; i < dispatcher->numThreads-1; i++) dispatcher->semaphore.release();

        if (response.empty() or ack.type != PacketType::ELECTION_ACK) {
            std::cout << "No server answered election" << std::endl;

            auto proc_serv = dispatcher->processingService;

            proc_serv->totalSum = 0;
            proc_serv->totalRequests = 0;
            //fill the ClientTable
            for (int i = 0; i < client_table->client_table.current_clients; i++) {
                proc_serv->totalSum = std::max(proc_serv->totalSum.load(), client_table->client_table.table[i].last_sum);
                proc_serv->totalRequests = std::max(proc_serv->totalRequests.load(), client_table->client_table.table[i].last_numreq);
            }

            notifyNewManager();
            return isManager = true; // I am new manager
        }
        return isManager = false; // I am not new manager
    }
    void startElection(){
        running_election = true;
        dispatcher->clearQueue();
        std::cout << "\tstart election" << std::endl;
        // dispatcher->clearQueue();
    }
    void endElection(){
        running_election = false;
        std::cout << "\tend election" << std::endl;
    }
};
