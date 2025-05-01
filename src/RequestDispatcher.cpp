#include "../include/RequestDispatcher.hpp"
#include <iostream>
#include <optional>

RequestDispatcher::RequestDispatcher(std::shared_ptr<IProcessingService> processingService,
                                     std::shared_ptr<IDiscoveryService> discoveryService,
                                     const size_t numThreads)
    : processingService(std::move(processingService)),
      discoveryService(std::move(discoveryService)),
      numThreads(numThreads),
      running(false),
      bufferCapacity(100)
{
    threads.reserve(numThreads);
    buffer.resize(bufferCapacity);
}

RequestDispatcher::~RequestDispatcher() {
    stop();
}

void RequestDispatcher::start() {
    running = true;
    for (size_t i = 0; i < numThreads; ++i) {
        threads.emplace_back(&RequestDispatcher::worker, this);
    }
}

void RequestDispatcher::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex);
        running = false;
    }
    cond.notify_all();

    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void RequestDispatcher::enqueue(Packet& packet, sockaddr_in& clientAddr) {
    // Since we have only one writer thread, it is theoretically safe to no use a mutex here...
    //std::lock_guard<std::mutex> lock(mutex);

    size_t next_tail = (tail + 1) % bufferCapacity;
    if (next_tail == head) {
        std::cerr << "RequestDispatcher Ring Buffer FULL\n";
        exit(1);
    }

    buffer[tail] = Request{std::move(packet), clientAddr};
    tail = next_tail;

    cond.notify_one();
}

void RequestDispatcher::worker() {
    while (true) {
        std::optional<Request> request_opt;
        {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [&]() { return head != tail || !running; });

            if (!running && head == tail) return;

            request_opt = std::move(buffer[head]);
            buffer[head].reset();
            head = (head + 1) % bufferCapacity;
        }

        if (request_opt) {
            Request& request = *request_opt;
            if (request.packet.type == PacketType::REQUEST) {
                processingService->handleRequest(request.packet, request.clientAddr);
            } else if (request.packet.type == PacketType::DISCOVERY) {
                discoveryService->handleRequest(request.clientAddr);
            }
        }
    }
}
