#include "consumer.hpp"

Consumer::Consumer(ThreadSafeQueue<Event>& queue, FeatureStore& store,
                   EventLogWriter* writer)
    : queue_(queue)
    , store_(store)
    , writer_(writer)
{}

void Consumer::start() {
    running_ = true;
    thread_ = std::thread(&Consumer::run, this);
}

void Consumer::stop() {
    running_ = false;
    queue_.stop();
    if (thread_.joinable())
        thread_.join();
}

void Consumer::set_batch_size(size_t n) {
    batch_size_ = n;
}

uint64_t Consumer::consumed_count() const {
    return consumed_.load();
}

void Consumer::run() {
    while (running_) {
        if (batch_size_ <= 1) {
            Event ev;
            if (queue_.pop(ev)) {
                store_.ingest(ev);
                if (writer_)
                    writer_->append(ev);
                ++consumed_;
            }
        } else {
            std::vector<Event> batch;
            batch.reserve(batch_size_);
            size_t n = queue_.pop_batch(batch, batch_size_);
            if (n == 0)
                continue;
            store_.ingest_batch(batch);
            if (writer_) {
                for (const auto& ev : batch)
                    writer_->append(ev);
            }
            consumed_ += n;
        }
    }
}
