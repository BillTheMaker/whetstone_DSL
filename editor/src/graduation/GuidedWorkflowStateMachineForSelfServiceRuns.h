#pragma once
// Step 1311: Guided workflow state machine for self-service runs.
#include <string>
#include <nlohmann/json.hpp>

struct GuidedWorkflowStateMachineForSelfServiceRuns {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class GuidedWorkflowStateMachineForSelfServiceRunsFactory {
public:
    static GuidedWorkflowStateMachineForSelfServiceRuns make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const GuidedWorkflowStateMachineForSelfServiceRuns& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
