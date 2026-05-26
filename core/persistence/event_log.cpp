#include "event_log.hpp"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

static constexpr uint32_t MAGIC = 0x454C4F47; // "EVLG"
static constexpr size_t   HEADER_SIZE = 12;    // magic(4) + count(8)
static constexpr size_t   RECORD_SIZE = 17;    // user_id(8) + type(1) + sequence(8)

static void write_all(int fd, const void* buf, size_t n) {
    const char* ptr = static_cast<const char*>(buf);
    size_t remaining = n;
    while (remaining > 0) {
        ssize_t written = ::write(fd, ptr, remaining);
        if (written < 0)
            throw std::runtime_error("EventLog write failed");
        ptr += written;
        remaining -= static_cast<size_t>(written);
    }
}

static void read_all(int fd, void* buf, size_t n) {
    char* ptr = static_cast<char*>(buf);
    size_t remaining = n;
    while (remaining > 0) {
        ssize_t got = ::read(fd, ptr, remaining);
        if (got <= 0)
            throw std::runtime_error("EventLog read failed or truncated");
        ptr += got;
        remaining -= static_cast<size_t>(got);
    }
}

// ── EventLogWriter ──────────────────────────────────────────────

EventLogWriter::EventLogWriter(const std::string& path)
    : path_(path)
    , count_(0)
{
    fd_ = ::open(path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_ < 0)
        throw std::runtime_error("Cannot open event log for writing: " + path);
    write_header();
}

EventLogWriter::~EventLogWriter() {
    if (fd_ >= 0) {
        // Rewind and write final count
        ::lseek(fd_, 4, SEEK_SET);
        ::write(fd_, &count_, sizeof(count_));
        ::close(fd_);
    }
}

void EventLogWriter::append(const Event& event) {
    uint8_t buf[RECORD_SIZE];
    std::memcpy(buf, &event.user_id, 8);
    buf[8] = static_cast<uint8_t>(event.type);
    std::memcpy(buf + 9, &event.sequence, 8);
    write_all(fd_, buf, RECORD_SIZE);
    ++count_;
}

void EventLogWriter::flush() {
    // no-op: writes are unbuffered (O_SYNC would be too slow)
    // fsync on close is sufficient for this demo
}

uint64_t EventLogWriter::count() const {
    return count_;
}

void EventLogWriter::write_header() {
    uint8_t header[HEADER_SIZE];
    std::memcpy(header, &MAGIC, 4);
    std::memset(header + 4, 0, 8); // count placeholder
    write_all(fd_, header, HEADER_SIZE);
}

// ── EventLogReader ──────────────────────────────────────────────

EventLogReader::EventLogReader(const std::string& path)
    : path_(path)
    , fd_(-1)
    , count_(0)
    , pos_(HEADER_SIZE)
    , header_read_(false)
{
    fd_ = ::open(path.c_str(), O_RDONLY);
    if (fd_ < 0)
        throw std::runtime_error("Cannot open event log for reading: " + path);
}

EventLogReader::~EventLogReader() {
    if (fd_ >= 0)
        ::close(fd_);
}

bool EventLogReader::read_next(Event& event) {
    if (!header_read_)
        read_header();

    if (pos_ >= HEADER_SIZE + count_ * RECORD_SIZE)
        return false;

    uint8_t buf[RECORD_SIZE];
    ::read_all(fd_, buf, RECORD_SIZE);
    pos_ += RECORD_SIZE;

    std::memcpy(&event.user_id, buf, 8);
    event.type = static_cast<EventType>(buf[8]);
    std::memcpy(&event.sequence, buf + 9, 8);
    return true;
}

std::vector<Event> EventLogReader::read_all() {
    std::vector<Event> events;
    Event ev;
    while (read_next(ev))
        events.push_back(ev);
    return events;
}

uint64_t EventLogReader::event_count() const {
    return count_;
}

bool EventLogReader::read_header() {
    uint8_t header[HEADER_SIZE];
    ::read_all(fd_, header, HEADER_SIZE);
    uint32_t magic;
    std::memcpy(&magic, header, 4);
    if (magic != MAGIC)
        throw std::runtime_error("Invalid event log magic");
    std::memcpy(&count_, header + 4, 8);
    header_read_ = true;
    return true;
}
