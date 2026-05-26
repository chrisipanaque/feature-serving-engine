#include "feature_store.hpp"
#include <iostream>
#include <shared_mutex>

void FeatureStore::ingest(const Event& event) {
    std::lock_guard<std::shared_mutex> lock(mtx_);
    store_[event.user_id].apply(event);
}

std::optional<FeatureVector> FeatureStore::get_features(uint64_t user_id) const {
    std::shared_lock<std::shared_mutex> lock(mtx_);
    auto it = store_.find(user_id);
    if (it == store_.end())
        return std::nullopt;
    return it->second;
}

void FeatureStore::print_all() const {
    std::shared_lock<std::shared_mutex> lock(mtx_);
    for (const auto& [uid, fv] : store_) {
        fv.print(std::cout, uid);
        std::cout << '\n';
    }
}
