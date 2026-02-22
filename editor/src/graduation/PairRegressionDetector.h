#pragma once
// Step 842: Pair regression detector with severity levels.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class RegressionSeverity { None, Low, Medium, High, Critical };

struct RegressionEvent {
    std::string pairId;
    std::string featureId;
    RegressionSeverity severity = RegressionSeverity::None;
    std::string description;
    std::string detectedAt;
};

struct RegressionDetectionResult {
    std::string pairId;
    std::vector<RegressionEvent> events;
    RegressionSeverity worstSeverity = RegressionSeverity::None;
    bool hasRegression = false;
};

class PairRegressionDetector {
public:
    static RegressionDetectionResult detect(const std::string& pairId,
                                             const std::vector<RegressionEvent>& events) {
        RegressionDetectionResult r;
        r.pairId = pairId;
        r.events = events;
        r.hasRegression = !events.empty();
        for (const auto& e : events)
            if (static_cast<int>(e.severity) > static_cast<int>(r.worstSeverity))
                r.worstSeverity = e.severity;
        return r;
    }

    static std::string severityName(RegressionSeverity s) {
        switch (s) {
            case RegressionSeverity::Critical: return "critical";
            case RegressionSeverity::High:     return "high";
            case RegressionSeverity::Medium:   return "medium";
            case RegressionSeverity::Low:      return "low";
            default:                           return "none";
        }
    }

    static nlohmann::json toJson(const RegressionDetectionResult& r) {
        return {{"pair_id", r.pairId}, {"has_regression", r.hasRegression},
                {"worst_severity", severityName(r.worstSeverity)},
                {"event_count", static_cast<int>(r.events.size())}};
    }
};
