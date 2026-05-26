#ifndef CORE_INFERENCE_INFERENCE_SERVICE_HPP
#define CORE_INFERENCE_INFERENCE_SERVICE_HPP

#include <cstdint>
#include <optional>

#include "core/features/feature_vector.hpp"
#include "core/inference/scorer.hpp"
#include "core/store/feature_store.hpp"

struct Prediction {
    uint64_t       user_id;
    FeatureVector  features;
    double         score;
};

struct InferenceStats {
    double avg_latency_us;
    double p50_latency_us;
    double p99_latency_us;
    double throughput_ev_s;
};

class InferenceService {
public:
    InferenceService(const FeatureStore& store, const Scorer& scorer);

    std::optional<Prediction> predict(uint64_t user_id) const;

    InferenceStats run_benchmark(
        uint64_t num_requests,
        unsigned num_threads,
        uint64_t num_users,
        uint64_t seed = 42
    ) const;

private:
    const FeatureStore& store_;
    const Scorer&       scorer_;
};

#endif
