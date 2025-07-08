#pragma once
#include "ISocket.hpp"
#include "Packet.hpp"
#include "ReplicaTableService.hpp"
#include <iostream>
#include <chrono>
#include <memory>
#include <atomic>
#include <arpa/inet.h>

class HeartbeatService {
	std::atomic<bool> *isManager, *running_election;
	std::shared_ptr<ISocket> socket;
	std::shared_ptr<ReplicaTableService> replica_table;
	std::chrono::time_point<std::chrono::steady_clock> timepoint;
public:
	HeartbeatService();
	HeartbeatService(std::shared_ptr<ISocket> socket,
	std::shared_ptr<ReplicaTableService> replica_table, 
		std::atomic<bool>* isManager, std::atomic<bool> *running_election);
	void start();
	void sendHeartbeat();
	bool checkTimer();
	void resetTimer();
};