#ifndef CORE_FEATURES_FEATURE_VECTOR_HPP
#define CORE_FEATURES_FEATURE_VECTOR_HPP

#include <cstdint>
#include <iosfwd>

#include "core/events/event.hpp"

struct FeatureVector {
    uint64_t views    = 0;
    uint64_t clicks   = 0;
    uint64_t purchases = 0;

    void apply(const Event& event);
    void print(std::ostream& os, uint64_t user_id) const;
};

#endif
