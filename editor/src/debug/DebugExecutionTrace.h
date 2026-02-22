#pragma once
// Step 1522: debug execution trace model.

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugTraceEvent {
    int index = 0;
    std::string action;
    std::string status;
};

class DebugExecutionTrace {
public:
    static std::vector<DebugTraceEvent> append(std::vector<DebugTraceEvent> events,
                                               const DebugTraceEvent& e) {
        events.push_back(e);
        std::sort(events.begin(), events.end(), [](const DebugTraceEvent& a, const DebugTraceEvent& b) {
            if (a.index != b.index) return a.index < b.index;
            if (a.action != b.action) return a.action < b.action;
            return a.status < b.status;
        });
        return events;
    }

    static nlohmann::json toJson(const std::vector<DebugTraceEvent>& events) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& e : events) arr.push_back({{"index", e.index}, {"action", e.action}, {"status", e.status}});
        return arr;
    }
};
