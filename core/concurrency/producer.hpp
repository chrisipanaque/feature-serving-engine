#ifndef CORE_CONCURRENCY_PRODUCER_HPP
#define CORE_CONCURRENCY_PRODUCER_HPP

#include <atomic>
#include <chrono>
#include <thread>

#include "core/events/generator.hpp"
#include "core/queue/thread_safe_queue.hpp"

class Producer {
public:
    Producer(EventGenerator& gen, ThreadSafeQueue<Event>& queue);

    void start();
    void stop();

    uint64_t produced_count() const;

private:
    void run();

    EventGenerator&          gen_;
    ThreadSafeQueue<Event>&  queue_;
    std::thread              thread_;
    std::atomic<bool>        running_{false};
    std::atomic<uint64_t>    produced_{0};
};

#endif
