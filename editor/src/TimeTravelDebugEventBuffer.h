#pragma once
// Step 571: Time-Travel Debug Event Buffer

#include "DebugValidationUtil.h"

#include <cstdint>
#include <deque>
#include <string>
#include <vector>

struct DebugReplayEvent {
    std::string eventId;
    std::string sessionId;
    std::string type;
    std::uint64_t instructionPointer = 0;
    std::uint64_t timestamp = 0;
    std::string payload;
};

class TimeTravelDebugEventBuffer {
public:
    explicit TimeTravelDebugEventBuffer(std::size_t capacity = 128)
        : capacity_(capacity == 0 ? 1 : capacity) {}

    bool append(const DebugReplayEvent& event, std::string* error) {
        if (!error) return false;
        error->clear();
        if (!hasRequiredDebugIds(event.eventId, event.sessionId)) return fail(error, "event_or_session_missing");
        if (event.type.empty()) return fail(error, "event_type_missing");
        if (!isNonZeroU64(event.timestamp)) return fail(error, "event_timestamp_invalid");
        if (!isNonZeroU64(event.instructionPointer)) return fail(error, "event_ip_invalid");
        if (containsId(event.eventId)) return fail(error, "event_duplicate");

        events_.push_back(event);
        while (events_.size() > capacity_) {
            events_.pop_front();
            ++dropped_;
        }
        return true;
    }

    std::size_t size() const { return events_.size(); }
    std::size_t capacity() const { return capacity_; }
    std::size_t droppedCount() const { return dropped_; }

    std::vector<DebugReplayEvent> latest(std::size_t count) const {
        std::vector<DebugReplayEvent> out;
        if (count == 0 || events_.empty()) return out;
        const std::size_t begin = events_.size() > count ? events_.size() - count : 0;
        for (std::size_t i = begin; i < events_.size(); ++i) out.push_back(events_[i]);
        return out;
    }

    std::vector<DebugReplayEvent> forSession(const std::string& sessionId) const {
        std::vector<DebugReplayEvent> out;
        for (const auto& event : events_) {
            if (sessionId.empty() || event.sessionId == sessionId) out.push_back(event);
        }
        return out;
    }

    std::vector<DebugReplayEvent> replayWindow(std::size_t startIndex, std::size_t count) const {
        std::vector<DebugReplayEvent> out;
        if (count == 0 || startIndex >= events_.size()) return out;
        const std::size_t end = (startIndex + count) > events_.size() ? events_.size() : (startIndex + count);
        for (std::size_t i = startIndex; i < end; ++i) out.push_back(events_[i]);
        return out;
    }

private:
    std::size_t capacity_ = 128;
    std::size_t dropped_ = 0;
    std::deque<DebugReplayEvent> events_;

    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }

    bool containsId(const std::string& eventId) const {
        for (const auto& event : events_) {
            if (event.eventId == eventId) return true;
        }
        return false;
    }
};
