#include "feature_vector.hpp"
#include <iostream>

void FeatureVector::apply(const Event& event) {
    switch (event.type) {
        case EventType::View:     ++views;    break;
        case EventType::Click:    ++clicks;   break;
        case EventType::Purchase: ++purchases; break;
    }
}

void FeatureVector::print(std::ostream& os, uint64_t user_id) const {
    os << "User " << user_id
       << ": views=" << views
       << " clicks=" << clicks
       << " purchases=" << purchases;
}
