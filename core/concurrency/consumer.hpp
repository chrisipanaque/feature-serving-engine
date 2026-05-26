#ifndef CORE_CONCURRENCY_CONSUMER_HPP
#define CORE_CONCURRENCY_CONSUMER_HPP

#include <atomic>
#include <chrono>
#include <thread>

#include "core/queue/thread_safe_queue.hpp"
#include "core/store/feature_store.hpp"

class Consumer {
public:
    Consumer(ThreadSafeQueue<Event>& queue, FeatureStore& store);

    void start();
    void stop();

    uint64_t consumed_count() const;

private:
    void run();

    ThreadSafeQueue<Event>&  queue_;
    FeatureStore&            store_;
    std::thread              thread_;
    std::atomic<bool>        running_{false};
    std::atomic<uint64_t>    consumed_{0};
};

#endif
