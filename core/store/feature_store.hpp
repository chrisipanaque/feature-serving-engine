#ifndef CORE_STORE_FEATURE_STORE_HPP
#define CORE_STORE_FEATURE_STORE_HPP

#include <mutex>
#include <optional>
#include <shared_mutex>
#include <unordered_map>

#include "core/events/event.hpp"
#include "core/features/feature_vector.hpp"

class FeatureStore {
public:
    void ingest(const Event& event);
    std::optional<FeatureVector> get_features(uint64_t user_id) const;
    void print_all() const;

private:
    std::unordered_map<uint64_t, FeatureVector> store_;
    mutable std::shared_mutex                   mtx_;
};

#endif
