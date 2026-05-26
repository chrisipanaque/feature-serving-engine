#include "feature_store.hpp"
#include <iostream>

void FeatureStore::ingest(const Event& event) {
    std::lock_guard<std::mutex> lock(mtx_);
    store_[event.user_id].apply(event);
}

void FeatureStore::print_all() const {
    std::lock_guard<std::mutex> lock(mtx_);
    for (const auto& [uid, fv] : store_) {
        fv.print(std::cout, uid);
        std::cout << '\n';
    }
}
