#include "../include/RequestDispatcher.hpp"

#include <iostream>

RequestDispatcher::RequestDispatcher(std::shared_ptr<IProcessingService> processingService,
                                     std::shared_ptr<IDiscoveryService> discoveryService, const size_t numThreads)
    : processingService(std::move(processingService)),
      discoveryService(std::move(discoveryService)),
      numThreads(numThreads),
      running(false) {
    threads.reserve(numThreads);
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


void RequestDispatcher::enqueue(Packet & packet, sockaddr_in &clientAddr) {
    std::lock_guard<std::mutex> lock(mutex);
    queue.push({std::move(packet), clientAddr});
    cond.notify_one();
}


void RequestDispatcher::worker() {
    while (running) {
        Request request; {
            std::unique_lock<std::mutex> lock(mutex);
            cond.wait(lock, [&]() { return !queue.empty() || !running; });

            if (!running && queue.empty()) return;

            request = queue.front();
            queue.pop();
        }
        if (request.packet.type == PacketType::REQUEST) {
            processingService->handleRequest(request.packet, request.clientAddr);
        } else if (request.packet.type == PacketType::DISCOVERY) {
            discoveryService->handleRequest(request.clientAddr);
        }
    }
}
