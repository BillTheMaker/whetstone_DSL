#pragma once
// Step 1333: Governance and safety invariant revalidation pass.
#include <string>
#include <nlohmann/json.hpp>

struct GovernanceAndSafetyInvariantRevalidationPass {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class GovernanceAndSafetyInvariantRevalidationPassFactory {
public:
    static GovernanceAndSafetyInvariantRevalidationPass make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const GovernanceAndSafetyInvariantRevalidationPass& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
