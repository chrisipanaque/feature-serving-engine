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

uint64_t Consumer::consumed_count() const {
    return consumed_.load();
}

void Consumer::run() {
    while (running_) {
        Event ev;
        if (queue_.pop(ev)) {
            store_.ingest(ev);
            if (writer_)
                writer_->append(ev);
            ++consumed_;
        }
    }
}
