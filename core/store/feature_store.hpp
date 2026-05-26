#ifndef CORE_STORE_FEATURE_STORE_HPP
#define CORE_STORE_FEATURE_STORE_HPP

#include <mutex>
#include <unordered_map>

#include "core/events/event.hpp"
#include "core/features/feature_vector.hpp"

class FeatureStore {
public:
    void ingest(const Event& event);
    void print_all() const;

private:
    std::unordered_map<uint64_t, FeatureVector> store_;
    mutable std::mutex                          mtx_;
};

#endif
