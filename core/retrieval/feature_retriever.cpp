#include "feature_retriever.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

FeatureRetriever::FeatureRetriever(const FeatureStore& store)
    : store_(store)
{}

std::optional<FeatureVector> FeatureRetriever::get_features(uint64_t user_id) const {
    return store_.get_features(user_id);
}

LatencyStats FeatureRetriever::run_benchmark(
    uint64_t num_requests,
    unsigned num_threads,
    uint64_t num_users,
    uint64_t seed
) const {
    std::vector<double> all_latencies;
    all_latencies.reserve(num_requests);

    std::mutex collect_mtx;

    std::vector<std::thread> threads;
    uint64_t per_thread = num_requests / num_threads;
    uint64_t remainder  = num_requests % num_threads;

    for (unsigned t = 0; t < num_threads; ++t) {
        uint64_t count = per_thread + (t < remainder ? 1 : 0);
        threads.emplace_back([this, count, num_users, seed, t, &all_latencies, &collect_mtx] {
            std::mt19937_64 rng(seed + t);
            std::vector<double> local;
            local.reserve(count);

            for (uint64_t i = 0; i < count; ++i) {
                uint64_t uid = rng() % num_users;

                auto start = std::chrono::high_resolution_clock::now();
                store_.get_features(uid);
                auto end = std::chrono::high_resolution_clock::now();

                double us = std::chrono::duration<double, std::micro>(end - start).count();
                local.push_back(us);
            }

            std::lock_guard<std::mutex> lock(collect_mtx);
            all_latencies.insert(all_latencies.end(), local.begin(), local.end());
        });
    }

    for (auto& th : threads)
        th.join();

    std::sort(all_latencies.begin(), all_latencies.end());

    double sum = std::accumulate(all_latencies.begin(), all_latencies.end(), 0.0);
    size_t n = all_latencies.size();

    LatencyStats stats;
    stats.avg_us = sum / n;
    stats.p50_us = all_latencies[n / 2];
    stats.p99_us = all_latencies[static_cast<size_t>(n * 0.99)];
    return stats;
}
