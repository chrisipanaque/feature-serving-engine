#ifndef CORE_EVENTS_GENERATOR_HPP
#define CORE_EVENTS_GENERATOR_HPP

#include "event.hpp"
#include <random>

class EventGenerator {
public:
    explicit EventGenerator(uint64_t seed = 42);

    Event next_event();

    void set_user_count(uint64_t count);
    void set_event_interval_ms(uint64_t ms);

    uint64_t user_count() const;
    uint64_t event_interval_ms() const;

private:
    std::mt19937_64 rng_;
    uint64_t        user_count_;
    uint64_t        interval_ms_;
    uint64_t        sequence_;
};

#endif
