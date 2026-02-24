#pragma once
// Step 1163: Scenario analysis model (`aggressive`, `balanced`, `conservative`).
#include <string>
#include <nlohmann/json.hpp>

struct ScenarioAnalysisModelAggressiveBalancedConservative {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class ScenarioAnalysisModelAggressiveBalancedConservativeFactory {
public:
    static ScenarioAnalysisModelAggressiveBalancedConservative make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const ScenarioAnalysisModelAggressiveBalancedConservative& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
