#include "producer.hpp"

Producer::Producer(EventGenerator& gen, ThreadSafeQueue<Event>& queue)
    : gen_(gen)
    , queue_(queue)
{}

void Producer::start() {
    running_ = true;
    thread_ = std::thread(&Producer::run, this);
}

void Producer::stop() {
    running_ = false;
    if (thread_.joinable())
        thread_.join();
}

uint64_t Producer::produced_count() const {
    return produced_.load();
}

void Producer::run() {
    while (running_) {
        Event ev = gen_.next_event();
        queue_.push(ev);
        ++produced_;
        std::this_thread::sleep_for(
            std::chrono::milliseconds(gen_.event_interval_ms()));
    }
}
