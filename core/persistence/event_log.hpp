#ifndef CORE_PERSISTENCE_EVENT_LOG_HPP
#define CORE_PERSISTENCE_EVENT_LOG_HPP

#include <cstdint>
#include <string>
#include <vector>

#include "core/events/event.hpp"

class EventLogWriter {
public:
    explicit EventLogWriter(const std::string& path);
    ~EventLogWriter();

    EventLogWriter(const EventLogWriter&) = delete;
    EventLogWriter& operator=(const EventLogWriter&) = delete;

    void append(const Event& event);
    void flush();

    uint64_t count() const;

private:
    void write_header();

    std::string   path_;
    int           fd_;
    uint64_t      count_;
};

class EventLogReader {
public:
    explicit EventLogReader(const std::string& path);
    ~EventLogReader();

    EventLogReader(const EventLogReader&) = delete;
    EventLogReader& operator=(const EventLogReader&) = delete;

    bool read_next(Event& event);
    std::vector<Event> read_all();

    uint64_t event_count() const;

private:
    bool read_header();

    std::string path_;
    int         fd_;
    uint64_t    count_;
    uint64_t    pos_;
    bool        header_read_;
};

#endif
