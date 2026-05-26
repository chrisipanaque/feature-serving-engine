#ifndef CORE_EVENTS_EVENT_HPP
#define CORE_EVENTS_EVENT_HPP

#include <cstdint>

enum class EventType : uint8_t {
    View,
    Click,
    Purchase
};

struct Event {
    uint64_t   user_id;
    EventType  type;
    uint64_t   sequence;
};

#endif
