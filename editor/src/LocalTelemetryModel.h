#pragma once
// Step 652: Privacy-preserving local telemetry

#include <map>
#include <string>

struct TelemetryCounters {
    int sessionMinutes = 0;
    int toolCalls = 0;
    int errorCount = 0;
    std::map<std::string, int> generatorUsage;
};

class LocalTelemetryModel {
public:
    static void recordSessionMinutes(TelemetryCounters* c, int minutes) {
        if (!c || minutes < 0) return;
        c->sessionMinutes += minutes;
    }

    static void recordToolCall(TelemetryCounters* c) {
        if (!c) return;
        ++c->toolCalls;
    }

    static void recordError(TelemetryCounters* c) {
        if (!c) return;
        ++c->errorCount;
    }

    static void recordGeneratorUse(TelemetryCounters* c, const std::string& generatorName) {
        if (!c || generatorName.empty()) return;
        ++c->generatorUsage[generatorName];
    }

    static std::string weeklyReport(const TelemetryCounters& c) {
        return "session_minutes=" + std::to_string(c.sessionMinutes) +
               ";tool_calls=" + std::to_string(c.toolCalls) +
               ";errors=" + std::to_string(c.errorCount);
    }
};
