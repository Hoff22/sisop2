#include "RequestDispatcher.hpp"
#include "Server.hpp"
#include <iostream>
#include <optional>

RequestDispatcher::RequestDispatcher(std::shared_ptr<ProcessingServiceImpl> processingService,
                                     std::shared_ptr<IDiscoveryService> discoveryService,
                                     std::shared_ptr<ServerDiscoveryServiceImpl> serverDiscoveryService,
                                     const size_t numThreads)
    : processingService(std::move(processingService)),
      discoveryService(std::move(discoveryService)),
      serverDiscoveryService(std::move(serverDiscoveryService)),
      numThreads(numThreads),
      running(false),
      bufferCapacity(100),
      semaphore(numThreads)
{
    threads.reserve(numThreads);
    buffer.resize(bufferCapacity);
}

RequestDispatcher::~RequestDispatcher()
{
    stop();
}

void RequestDispatcher::start()
{
    running = true;
    for (size_t i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(&RequestDispatcher::worker, this);
    }
    // add a thread here to deal with election sending
}

void RequestDispatcher::stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        running = false;
    }
    cond.notify_all();

    for (auto &t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }
}

void RequestDispatcher::enqueue(Packet &packet, sockaddr_in &clientAddr)
{
    size_t next_tail = (tail + 1) % bufferCapacity;
    if (next_tail == head)
    {
        std::cerr << "RequestDispatcher Ring Buffer FULL\n";
        exit(1);
    }

    buffer[tail] = Request{std::move(packet), clientAddr};
    tail = next_tail;

    cond.notify_one();
}

void RequestDispatcher::clearQueue(){
    std::cout << "[DEBUG]" << " starting to clear queue" << std::endl;
    while(true){
        std::optional<Request> request_opt;
        {
            if(head == tail) return;

            request_opt = std::move(buffer[head]);
            buffer[head].reset();
            head = (head + 1) % bufferCapacity;
        }
    }
    std::cout << "[DEBUG]" << " queue cleared" << std::endl;
}

int RequestDispatcher::getClientIndex(uint32_t ip, uint16_t port)
{
    const auto key = std::make_pair(ip, port);
    for (int i = 0; i < current_clients; i++)
    {
        if (client_index[i] == key)
        {
            return i;
        }
    }
    std::ostringstream oss;
    oss << "Could not find index for key [" << key.first << "," << key.second << "] in client index table";
    throw std::runtime_error(oss.str());
}

void RequestDispatcher::setClientIndex(uint32_t ip, uint16_t port)
{
    const auto key = std::make_pair(ip, port);
    client_index[current_clients++] = key;
}

void RequestDispatcher::enterA()
{
    std::unique_lock<std::mutex> lock(a_b_mutex);
    waiting_A++;
    a_b_cv.wait(lock, [&] { return active_B == 0; });
    waiting_A--;
    active_A++;
}

void RequestDispatcher::exitA()
{
    std::lock_guard<std::mutex> lock(a_b_mutex);
    active_A--;
    a_b_cv.notify_all();
}

void RequestDispatcher::enterB()
{
    std::unique_lock<std::mutex> lock(a_b_mutex);
    a_b_cv.wait(lock, [&] { return waiting_A == 0 && active_A == 0; });
    active_B++;
}

void RequestDispatcher::exitB()
{
    std::lock_guard<std::mutex> lock(a_b_mutex);
    active_B--;
    a_b_cv.notify_all();
}

void RequestDispatcher::worker()
{
    while (true)
    {
        std::optional<Request> request_opt;
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [&]() { return head != tail || !running; });

            if (!running && head == tail)
                return;

            request_opt = std::move(buffer[head]);
            buffer[head].reset();
            head = (head + 1) % bufferCapacity;
        }

        if (request_opt)
        {
            Request &request = *request_opt;
            const uint32_t ip = request.clientAddr.sin_addr.s_addr;
            const uint16_t port = ntohs(request.clientAddr.sin_port);

            if (request.packet.type == PacketType::SERVER_DISCOVERY)
            {
                // semaphore.acquire();
                enterA();
                std::cout << "[DEBUG] " << PacketString[(uint16_t)request.packet.type] << std::endl;
                serverDiscoveryService->handleRequest(request.packet, request.clientAddr);
                exitA();
                // semaphore.release();
            }
            else if(request.packet.type == PacketType::ELECTION){
                enterA();
                // will only enter this if no other worker threads are holding packets
                // will do election while queue is locked
                // will then leave
                std::cout << "[DEBUG] " << PacketString[(uint16_t)request.packet.type] << std::endl;
                processingService->handleElectionRequest(request.packet, request.clientAddr);
                server_reference->startElection();
                // server_reference->endElection();
                exitA();
            }
            else
            {
                // semaphore.acquire();
                enterB();

                if (request.packet.type == PacketType::REQUEST)
                {
                    std::cout << "[DEBUG] " << PacketString[(uint16_t)request.packet.type] << std::endl;
                    int client_idx = getClientIndex(ip, port);
                    in_proc[client_idx].lock();
                    processingService->handleRequest(request.packet, request.clientAddr);
                    in_proc[client_idx].unlock();
                }
                else if (request.packet.type == PacketType::DISCOVERY)
                {
                    std::cout << "[DEBUG] " << PacketString[(uint16_t)request.packet.type] << std::endl;
                    setClientIndex(ip, port);
                    discoveryService->handleRequest(request.clientAddr, server_reference->isManager);
                }
                else if (request.packet.type == PacketType::REQUEST_REPLICATION and !server_reference->isManager)
                {
                    std::cout << "[DEBUG] " << PacketString[(uint16_t)request.packet.type] << std::endl;
                    processingService->handleUpdateReplicaRequest(request.packet, request.clientAddr);
                }
                else if(request.packet.type == PacketType::HEARTBEAT){
                    std::cout << "[DEBUG] " << PacketString[(uint16_t)request.packet.type] << std::endl;
                    heartbeatService->resetTimer();
                }

                exitB();
                // semaphore.release();
            }
        }
    }
}
