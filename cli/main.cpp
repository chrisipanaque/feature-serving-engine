#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "core/concurrency/consumer.hpp"
#include "core/concurrency/producer.hpp"
#include "core/events/generator.hpp"
#include "core/inference/inference_service.hpp"
#include "core/inference/scorer.hpp"
#include "core/network/server.hpp"
#include "core/persistence/event_log.hpp"
#include "core/queue/thread_safe_queue.hpp"
#include "core/retrieval/feature_retriever.hpp"
#include "core/store/feature_store.hpp"

static constexpr const char* DEFAULT_LOG = "events.log";

static void run_benchmark(FeatureStore& store, uint64_t num_users) {
    std::cout << "\n--- Retrieval Benchmark ---\n";
    FeatureRetriever retriever(store);
    LatencyStats stats = retriever.run_benchmark(10000, 4, num_users);
    std::cout << "Requests: 10000, Threads: 4\n";
    std::cout << "Average latency: " << stats.avg_us << " us\n";
    std::cout << "P50 latency:     " << stats.p50_us << " us\n";
    std::cout << "P99 latency:     " << stats.p99_us << " us\n";
}

static void run_inference_benchmark(const InferenceService& service, uint64_t num_users) {
    std::cout << "\n--- Inference Benchmark ---\n";
    InferenceStats stats = service.run_benchmark(10000, 4, num_users);
    std::cout << "Requests: 10000, Threads: 4\n";
    std::cout << "Average latency: " << stats.avg_latency_us << " us\n";
    std::cout << "P50 latency:     " << stats.p50_latency_us << " us\n";
    std::cout << "P99 latency:     " << stats.p99_latency_us << " us\n";
    std::cout << "Throughput:      " << stats.throughput_ev_s << " ev/s\n";
}

static void run_replay(const std::string& path) {
    constexpr uint64_t NUM_USERS = 50;

    std::cout << "--- Replay Mode ---\n";
    FeatureStore store;
    EventLogReader reader(path);
    uint64_t count = 0;

    Event ev;
    while (reader.read_next(ev)) {
        store.ingest(ev);
        ++count;
    }

    Scorer scorer;
    InferenceService service(store, scorer);

    std::cout << "Replayed " << count << " events from " << path << "\n";
    std::cout << "\n--- Feature State ---\n";
    store.print_all();
    run_benchmark(store, NUM_USERS);
    run_inference_benchmark(service, NUM_USERS);
}

static void run_normal(short port) {
    constexpr uint64_t NUM_USERS      = 50;
    constexpr int      PRINT_INTERVAL = 3;
    constexpr int      RUN_DURATION   = 15;

    // Startup reconstruction
    FeatureStore store;
    try {
        EventLogReader reader(DEFAULT_LOG);
        uint64_t reconstructed = 0;
        Event ev;
        while (reader.read_next(ev)) {
            store.ingest(ev);
            ++reconstructed;
        }
        if (reconstructed > 0)
            std::cout << "Reconstructed " << reconstructed
                      << " events from " << DEFAULT_LOG << "\n";
    } catch (...) {
        // No existing log — start fresh
    }

    // Inference service (shared across sessions)
    Scorer scorer;
    InferenceService service(store, scorer);

    // TCP server
    Server server(service, port);
    server.start();

    EventGenerator gen(42);
    gen.set_user_count(NUM_USERS);

    ThreadSafeQueue<Event> queue;
    EventLogWriter writer(DEFAULT_LOG);

    Producer producer(gen, queue);
    Consumer consumer(queue, store, &writer);

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
    server.stop();
    producer.stop();
    consumer.stop();

    uint64_t final_produced = producer.produced_count();
    uint64_t final_consumed = consumer.consumed_count();

    std::cout << "Final: produced=" << final_produced
              << " consumed=" << final_consumed
              << " (lost=" << (final_produced - final_consumed) << ")\n"
              << "Persisted: " << writer.count() << " events to "
              << DEFAULT_LOG << "\n";

    std::cout << "\n--- Final Feature State ---\n";
    store.print_all();
    run_benchmark(store, NUM_USERS);
    run_inference_benchmark(service, NUM_USERS);
}

int main(int argc, char* argv[]) {
    short port = 8080;

    if (argc >= 3 && std::strcmp(argv[1], "--port") == 0) {
        port = static_cast<short>(std::stoi(argv[2]));
        run_normal(port);
    } else if (argc >= 3 && std::strcmp(argv[1], "--replay") == 0) {
        run_replay(argv[2]);
    } else if (argc >= 2 && std::strcmp(argv[1], "--replay") == 0) {
        std::cerr << "Usage: " << argv[0] << " [--port <n>] [--replay <logfile>]\n";
        return 1;
    } else {
        run_normal(port);
    }
    return 0;
}
