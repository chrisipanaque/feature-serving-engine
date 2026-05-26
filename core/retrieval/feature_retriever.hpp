#ifndef CORE_RETRIEVAL_FEATURE_RETRIEVER_HPP
#define CORE_RETRIEVAL_FEATURE_RETRIEVER_HPP

#include <cstdint>
#include <random>

#include "core/retrieval/latency_stats.hpp"
#include "core/store/feature_store.hpp"

class FeatureRetriever {
public:
    explicit FeatureRetriever(const FeatureStore& store);

    std::optional<FeatureVector> get_features(uint64_t user_id) const;

    LatencyStats run_benchmark(
        uint64_t num_requests,
        unsigned num_threads,
        uint64_t num_users,
        uint64_t seed = 42
    ) const;

private:
    const FeatureStore& store_;
};

#endif
