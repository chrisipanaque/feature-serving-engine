#include <chrono>
#include <iostream>
#include <thread>

#include "core/concurrency/consumer.hpp"
#include "core/concurrency/producer.hpp"
#include "core/events/generator.hpp"
#include "core/queue/thread_safe_queue.hpp"
#include "core/retrieval/feature_retriever.hpp"
#include "core/store/feature_store.hpp"

int main() {
    constexpr uint64_t NUM_USERS      = 50;
    constexpr int      PRINT_INTERVAL = 3;
    constexpr int      RUN_DURATION   = 15;

    EventGenerator gen(42);
    gen.set_user_count(NUM_USERS);

    ThreadSafeQueue<Event> queue;
    FeatureStore           store;

    Producer producer(gen, queue);
    Consumer consumer(queue, store);

    producer.start();
    consumer.start();

    uint64_t prev_consumed = 0;

    for (int elapsed = 0; elapsed < RUN_DURATION; elapsed += PRINT_INTERVAL) {
        std::this_thread::sleep_for(std::chrono::seconds(PRINT_INTERVAL));

        uint64_t consumed   = consumer.consumed_count();
        uint64_t produced   = producer.produced_count();
        uint64_t queue_size = queue.size();
        uint64_t rate       = (consumed - prev_consumed) / PRINT_INTERVAL;
        prev_consumed       = consumed;

        std::cout << "\n--- Feature State [t=" << elapsed + PRINT_INTERVAL
                  << "s] ---\n";
        store.print_all();
        std::cout << "[Metrics] produced=" << produced
                  << " consumed=" << consumed
                  << " queue=" << queue_size
                  << " rate=" << rate << " ev/s\n";
    }

    std::cout << "\n--- Shutting down ---\n";
    producer.stop();
    consumer.stop();

    uint64_t final_produced = producer.produced_count();
    uint64_t final_consumed = consumer.consumed_count();

    std::cout << "Final: produced=" << final_produced
              << " consumed=" << final_consumed
              << " (lost=" << (final_produced - final_consumed) << ")\n";

    std::cout << "\n--- Final Feature State ---\n";
    store.print_all();

    std::cout << "\n--- Retrieval Benchmark ---\n";
    FeatureRetriever retriever(store);
    LatencyStats stats = retriever.run_benchmark(10000, 4, NUM_USERS);
    std::cout << "Requests: 10000, Threads: 4\n";
    std::cout << "Average latency: " << stats.avg_us << " us\n";
    std::cout << "P50 latency:     " << stats.p50_us << " us\n";
    std::cout << "P99 latency:     " << stats.p99_us << " us\n";

    return 0;
}
