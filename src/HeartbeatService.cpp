#include "HeartbeatService.hpp"

HeartbeatService::HeartbeatService(std::shared_ptr<ISocket> socket,
		std::shared_ptr<ReplicaTableService> replica_table, 
		bool* isManager)
	: isManager(isManager),
	  socket(socket),
	  replica_table(replica_table),
	  timepoint(std::chrono::steady_clock::now())
{}

void HeartbeatService::start(){
	resetTimer();
	while (true) {
    	if(checkTimer()){
	        if(*isManager){
	        	sendHeartbeat();
	        }
	        else{
	        	std::cout << "send election!" << std::endl;
        		// send election
        	}
        	resetTimer();
        }
    }
}
void HeartbeatService::sendHeartbeat(){
	Packet request_heartbeat(PacketType::HEARTBEAT, 0);
	for(int i = 0; i < replica_table->getTable().current_replicas; i++){
		const ReplicaInfo &info = replica_table->getTable().table[i];
		if(socket->getSocketIp() == info.ip) continue;
        socket->sendTo(request_heartbeat.serialize(), info.replicaToSockaddr());
	}
}
bool HeartbeatService::checkTimer(){
	auto now = std::chrono::steady_clock::now();
	auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - timepoint);
	// in ms
	if((elapsed.count() > 1500 && *isManager) || (elapsed.count() > 3000 && !*isManager)) return 1;
	return 0;
}
void HeartbeatService::resetTimer(){
	timepoint = std::chrono::steady_clock::now();
}
