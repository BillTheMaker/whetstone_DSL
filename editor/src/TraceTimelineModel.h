#pragma once
// Step 561: Trace Timeline Model

#include <algorithm>
#include <string>
#include <vector>

struct TraceEvent {
    long long ts = 0;
    std::string type;   // pause/step/continue/exception/resume/stop
    std::string sessionId;
    std::string detail;
};

class TraceTimelineModel {
public:
    bool append(const TraceEvent& event) {
        if (!isValid(event)) return false;
        events_.push_back(event);
        std::sort(events_.begin(), events_.end(),
                  [](const TraceEvent& a, const TraceEvent& b) {
                      if (a.ts != b.ts) return a.ts < b.ts;
                      return a.type < b.type;
                  });
        return true;
    }

    std::vector<TraceEvent> all() const { return events_; }

    std::vector<TraceEvent> byType(const std::string& type) const {
        std::vector<TraceEvent> out;
        for (const auto& e : events_) if (e.type == type) out.push_back(e);
        return out;
    }

    std::vector<TraceEvent> bySession(const std::string& sessionId) const {
        std::vector<TraceEvent> out;
        for (const auto& e : events_) if (e.sessionId == sessionId) out.push_back(e);
        return out;
    }

    std::vector<TraceEvent> window(long long startTs, long long endTs) const {
        std::vector<TraceEvent> out;
        for (const auto& e : events_) {
            if (e.ts >= startTs && e.ts <= endTs) out.push_back(e);
        }
        return out;
    }

private:
    std::vector<TraceEvent> events_;

    static bool isValid(const TraceEvent& event) {
        if (event.ts < 0) return false;
        if (event.type.empty() || event.sessionId.empty()) return false;
        return true;
    }
};
