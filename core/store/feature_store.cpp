#include "feature_store.hpp"
#include <iostream>
#include <shared_mutex>

void FeatureStore::ingest(const Event& event) {
    auto& shard = shards_[shard_index(event.user_id)];
    std::lock_guard<std::shared_mutex> lock(shard.mtx);
    shard.store[event.user_id].apply(event);
}

void FeatureStore::ingest_batch(const std::vector<Event>& events) {
    // Group events by shard to lock each shard once
    struct Batch {
        std::vector<Event> evs;
    };
    std::array<Batch, NUM_SHARDS> groups;

    for (const auto& ev : events) {
        size_t idx = shard_index(ev.user_id);
        groups[idx].evs.push_back(ev);
    }

    for (size_t i = 0; i < NUM_SHARDS; ++i) {
        if (groups[i].evs.empty())
            continue;
        std::lock_guard<std::shared_mutex> lock(shards_[i].mtx);
        for (const auto& ev : groups[i].evs) {
            shards_[i].store[ev.user_id].apply(ev);
        }
    }
}

std::optional<FeatureVector> FeatureStore::get_features(uint64_t user_id) const {
    auto& shard = shards_[shard_index(user_id)];
    std::shared_lock<std::shared_mutex> lock(shard.mtx);
    auto it = shard.store.find(user_id);
    if (it == shard.store.end())
        return std::nullopt;
    return it->second;
}

void FeatureStore::print_all() const {
    for (size_t i = 0; i < NUM_SHARDS; ++i) {
        std::shared_lock<std::shared_mutex> lock(shards_[i].mtx);
        for (const auto& [uid, fv] : shards_[i].store) {
            fv.print(std::cout, uid);
            std::cout << '\n';
        }
    }
}
