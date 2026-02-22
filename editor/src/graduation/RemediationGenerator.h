#pragma once
// Step 854: Recommended remediation generator.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct RemediationRecommendation {
    std::string code;
    std::string action;
    std::string description;
    int estimatedEffort = 0; // story points
    std::string priority;
};

class RemediationGenerator {
public:
    static RemediationRecommendation generate(const std::string& code, const std::string& priority) {
        RemediationRecommendation r;
        r.code = code;
        r.priority = priority;
        if (code == "T001") {
            r.action = "update_semantic_adapter";
            r.description = "Review and update semantic mapping rules";
            r.estimatedEffort = 8;
        } else if (code == "T004") {
            r.action = "add_ownership_annotations";
            r.description = "Add memory ownership annotation hints";
            r.estimatedEffort = 5;
        } else {
            r.action = "review_and_patch";
            r.description = "Manual review and adapter patch required";
            r.estimatedEffort = 3;
        }
        return r;
    }

    static nlohmann::json toJson(const RemediationRecommendation& r) {
        return {{"code", r.code}, {"action", r.action},
                {"description", r.description}, {"effort", r.estimatedEffort},
                {"priority", r.priority}};
    }
};
