#include "inference_service.hpp"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <numeric>
#include <random>
#include <thread>
#include <vector>

InferenceService::InferenceService(const FeatureStore& store, const Scorer& scorer)
    : store_(store)
    , scorer_(scorer)
{}

std::optional<Prediction> InferenceService::predict(uint64_t user_id) const {
    auto fv = store_.get_features(user_id);
    if (!fv)
        return std::nullopt;
    return Prediction{user_id, *fv, scorer_.score(*fv)};
}

InferenceStats InferenceService::run_benchmark(
    uint64_t num_requests,
    unsigned num_threads,
    uint64_t num_users,
    uint64_t seed
) const {
    std::vector<double> all_latencies;
    all_latencies.reserve(num_requests);

    std::mutex collect_mtx;

    auto overall_start = std::chrono::high_resolution_clock::now();

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
                predict(uid);
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

    auto overall_end = std::chrono::high_resolution_clock::now();
    double total_sec = std::chrono::duration<double>(overall_end - overall_start).count();

    std::sort(all_latencies.begin(), all_latencies.end());

    double sum = std::accumulate(all_latencies.begin(), all_latencies.end(), 0.0);
    size_t n = all_latencies.size();

    InferenceStats stats;
    stats.avg_latency_us = sum / n;
    stats.p50_latency_us = all_latencies[n / 2];
    stats.p99_latency_us = all_latencies[static_cast<size_t>(n * 0.99)];
    stats.throughput_ev_s = (total_sec > 0.0) ? static_cast<double>(n) / total_sec : 0.0;
    return stats;
}
