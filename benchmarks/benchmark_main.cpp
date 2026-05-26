#include <algorithm>
#include <chrono>
#include <cstring>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "core/events/event.hpp"
#include "core/events/generator.hpp"
#include "core/features/feature_vector.hpp"
#include "core/inference/inference_service.hpp"
#include "core/inference/scorer.hpp"
#include "core/profiling/latency_recorder.hpp"
#include "core/profiling/timer.hpp"
#include "core/queue/thread_safe_queue.hpp"
#include "core/store/feature_store.hpp"

static constexpr uint64_t NUM_USERS = 1000;
static constexpr uint64_t NUM_EVENTS = 100000;

static void fill_store(FeatureStore& store, uint64_t num_events) {
    EventGenerator gen(42);
    gen.set_user_count(NUM_USERS);
    for (uint64_t i = 0; i < num_events; ++i)
        store.ingest(gen.next_event());
}

static void throughput_benchmark(uint64_t num_events) {
    std::cout << "\n=== Throughput Benchmark ===\n";
    std::cout << "Events: " << num_events << "\n\n";

    for (size_t batch_size : {1, 8, 32, 64}) {
        FeatureStore store;
        EventGenerator gen(42);
        gen.set_user_count(NUM_USERS);

        // Pre-generate events into a queue
        ThreadSafeQueue<Event> queue;
        for (uint64_t i = 0; i < num_events; ++i)
            queue.push(gen.next_event());

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<Event> batch;
        batch.reserve(batch_size);
        uint64_t consumed = 0;

        while (consumed < num_events) {
            if (batch_size <= 1) {
                Event ev;
                if (queue.pop(ev)) {
                    store.ingest(ev);
                    ++consumed;
                }
            } else {
                batch.clear();
                size_t n = queue.pop_batch(batch, batch_size);
                if (n == 0) break;
                store.ingest_batch(batch);
                consumed += n;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        double sec = std::chrono::duration<double>(end - start).count();

        std::cout << "  batch_size=" << batch_size
                  << ": " << consumed << " events in "
                  << sec << "s = "
                  << (static_cast<double>(consumed) / sec) << " ev/s\n";
    }
}

static void retrieval_latency_benchmark(uint64_t num_requests,
                                         unsigned num_threads) {
    std::cout << "\n=== Retrieval Latency Benchmark ===\n";
    std::cout << "Requests: " << num_requests
              << ", Threads: " << num_threads << "\n";

    FeatureStore store;
    fill_store(store, NUM_EVENTS);

    LatencyRecorder recorder;
    std::mutex collect_mtx;
    std::vector<double> all_latencies;
    all_latencies.reserve(num_requests);

    std::vector<std::thread> threads;
    uint64_t per_thread = num_requests / num_threads;
    uint64_t remainder  = num_requests % num_threads;

    for (unsigned t = 0; t < num_threads; ++t) {
        uint64_t count = per_thread + (t < remainder ? 1 : 0);
        threads.emplace_back([&, count, t] {
            std::mt19937_64 rng(42 + t);
            std::vector<double> local;
            local.reserve(count);

            for (uint64_t i = 0; i < count; ++i) {
                uint64_t uid = rng() % NUM_USERS;
                double us;
                {
                    ScopedTimer timer(us);
                    store.get_features(uid);
                }
                local.push_back(us);
            }

            std::lock_guard<std::mutex> lock(collect_mtx);
            all_latencies.insert(all_latencies.end(), local.begin(), local.end());
        });
    }

    for (auto& th : threads)
        th.join();

    std::sort(all_latencies.begin(), all_latencies.end());
    double sum = 0;
    for (double v : all_latencies) sum += v;
    size_t n = all_latencies.size();

    std::cout << "  Avg: " << (sum / n) << " us"
              << " | P50: " << all_latencies[n / 2] << " us"
              << " | P99: " << all_latencies[static_cast<size_t>(n * 0.99)]
              << " us\n";
}

static void inference_latency_benchmark(uint64_t num_requests,
                                         unsigned num_threads) {
    std::cout << "\n=== Inference Latency Benchmark ===\n";
    std::cout << "Requests: " << num_requests
              << ", Threads: " << num_threads << "\n";

    FeatureStore store;
    fill_store(store, NUM_EVENTS);

    Scorer scorer;
    InferenceService service(store, scorer);

    auto overall_start = std::chrono::high_resolution_clock::now();

    std::mutex collect_mtx;
    std::vector<double> all_latencies;
    all_latencies.reserve(num_requests);

    std::vector<std::thread> threads;
    uint64_t per_thread = num_requests / num_threads;
    uint64_t remainder  = num_requests % num_threads;

    for (unsigned t = 0; t < num_threads; ++t) {
        uint64_t count = per_thread + (t < remainder ? 1 : 0);
        threads.emplace_back([&, count, t] {
            std::mt19937_64 rng(42 + t);
            std::vector<double> local;
            local.reserve(count);

            for (uint64_t i = 0; i < count; ++i) {
                uint64_t uid = rng() % NUM_USERS;
                double us;
                {
                    ScopedTimer timer(us);
                    service.predict(uid);
                }
                local.push_back(us);
            }

            std::lock_guard<std::mutex> lock(collect_mtx);
            all_latencies.insert(all_latencies.end(), local.begin(), local.end());
        });
    }

    for (auto& th : threads)
        th.join();

    auto overall_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(overall_end - overall_start).count();

    std::sort(all_latencies.begin(), all_latencies.end());
    double sum = 0;
    for (double v : all_latencies) sum += v;
    size_t n = all_latencies.size();

    std::cout << "  Avg: " << (sum / n) << " us"
              << " | P50: " << all_latencies[n / 2] << " us"
              << " | P99: " << all_latencies[static_cast<size_t>(n * 0.99)]
              << " us"
              << " | Throughput: " << (static_cast<double>(n) / total_sec)
              << " ev/s\n";
}

int main(int argc, char* argv[]) {
    if (argc >= 2 && std::strcmp(argv[1], "--throughput") == 0) {
        throughput_benchmark(NUM_EVENTS);
    } else if (argc >= 2 && std::strcmp(argv[1], "--latency") == 0) {
        retrieval_latency_benchmark(100000, 4);
        inference_latency_benchmark(100000, 4);
    } else {
        // Run all benchmarks
        throughput_benchmark(50000);
        retrieval_latency_benchmark(100000, 4);
        inference_latency_benchmark(100000, 4);
    }
    return 0;
}
