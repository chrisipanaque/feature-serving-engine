#include "generator.hpp"

EventGenerator::EventGenerator(uint64_t seed)
    : rng_(seed)
    , user_count_(100)
    , interval_ms_(100)
    , sequence_(0)
{}

Event EventGenerator::next_event() {
    uint64_t uid = rng_() % user_count_;
    EventType type;
    uint64_t roll = rng_() % 100;
    if (roll < 60)
        type = EventType::View;
    else if (roll < 85)
        type = EventType::Click;
    else
        type = EventType::Purchase;

    return Event{uid, type, sequence_++};
}

void EventGenerator::set_user_count(uint64_t count) { user_count_ = count; }
void EventGenerator::set_event_interval_ms(uint64_t ms) { interval_ms_ = ms; }
uint64_t EventGenerator::user_count() const { return user_count_; }
uint64_t EventGenerator::event_interval_ms() const { return interval_ms_; }
