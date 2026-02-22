#pragma once
// Step 1467: debugging metrics packet.

#include <string>

#include <nlohmann/json.hpp>

struct DebugMetricsPacket {
    std::string sessionId;
    int iterationsToGreen = 0;
    int timeToGreenMs = 0;
    int tokenCostPerFix = 0;
    int symptomToRootCauseCompression = 0;
    int patchAcceptanceRatePct = 0;
    int regressionEscapeRatePct = 0;
};

class DebugMetricsModel {
public:
    static DebugMetricsPacket compute(const std::string& sessionId,
                                      int iterations,
                                      int durationMs,
                                      int tokenCost,
                                      int symptomCount,
                                      int rootCauseCount,
                                      int acceptedPatches,
                                      int proposedPatches,
                                      int regressions,
                                      int totalRuns) {
        DebugMetricsPacket m;
        m.sessionId = sessionId;
        m.iterationsToGreen = iterations;
        m.timeToGreenMs = durationMs;
        m.tokenCostPerFix = tokenCost;
        m.symptomToRootCauseCompression = (rootCauseCount == 0) ? 0 : (100 * symptomCount / rootCauseCount);
        m.patchAcceptanceRatePct = (proposedPatches == 0) ? 0 : (100 * acceptedPatches / proposedPatches);
        m.regressionEscapeRatePct = (totalRuns == 0) ? 0 : (100 * regressions / totalRuns);
        return m;
    }

    static nlohmann::json toJson(const DebugMetricsPacket& m) {
        return {
            {"session_id", m.sessionId},
            {"iterations_to_green", m.iterationsToGreen},
            {"time_to_green_ms", m.timeToGreenMs},
            {"token_cost_per_fix", m.tokenCostPerFix},
            {"symptom_to_root_cause_compression", m.symptomToRootCauseCompression},
            {"patch_acceptance_rate_pct", m.patchAcceptanceRatePct},
            {"regression_escape_rate_pct", m.regressionEscapeRatePct}
        };
    }
};
