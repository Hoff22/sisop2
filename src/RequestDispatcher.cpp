#include "../include/RequestDispatcher.hpp"

RequestDispatcher::RequestDispatcher(std::shared_ptr<IProcessingService> processingService, const size_t numThreads)
    : processingService(std::move(processingService)), running(false) {
    threads.reserve(numThreads);
}

RequestDispatcher::~RequestDispatcher() {
    stop();
}

void RequestDispatcher::start() {
    running = true;
    for (size_t i = 0; i < threads.capacity(); ++i) {
        threads.emplace_back(&RequestDispatcher::worker, this);
    }
}

void RequestDispatcher::stop() {
    running = false;
    cond.notify_all();

    for (auto &t: threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void RequestDispatcher::enqueue(const Packet &packet, const sockaddr_in clientAddr) { {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push({packet, clientAddr});
    }
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
        processingService->handleRequest(request.packet, request.clientAddr);
    }
}
