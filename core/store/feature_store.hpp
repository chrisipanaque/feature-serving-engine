#ifndef CORE_STORE_FEATURE_STORE_HPP
#define CORE_STORE_FEATURE_STORE_HPP

#include <array>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "core/events/event.hpp"
#include "core/features/feature_vector.hpp"

class FeatureStore {
public:
    static constexpr size_t NUM_SHARDS = 16;

    void ingest(const Event& event);
    void ingest_batch(const std::vector<Event>& events);
    std::optional<FeatureVector> get_features(uint64_t user_id) const;
    void print_all() const;

private:
    struct Shard {
        std::unordered_map<uint64_t, FeatureVector> store;
        mutable std::shared_mutex                   mtx;
    };

    std::array<Shard, NUM_SHARDS> shards_;

    size_t shard_index(uint64_t user_id) const {
        return user_id % NUM_SHARDS;
    }
};

#endif
