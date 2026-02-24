#pragma once
// Step 1202: Semantic impact estimator for patch diffs.
#include <string>
#include <nlohmann/json.hpp>

struct SemanticImpactEstimatorForPatchDiffs {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class SemanticImpactEstimatorForPatchDiffsFactory {
public:
    static SemanticImpactEstimatorForPatchDiffs make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const SemanticImpactEstimatorForPatchDiffs& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
