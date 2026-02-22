#pragma once
// Step 1531: debug recovery advisor model.

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct DebugRecoveryAdvice {
    std::string category = "generic";
    std::vector<std::string> actions;
};

class DebugRecoveryAdvisor {
public:
    static DebugRecoveryAdvice advise(const std::string& stopReason) {
        DebugRecoveryAdvice a;
        a.category = stopReason;
        if (stopReason == "budget_exhausted") {
            a.actions = {
                "Reduce context budget to tiny",
                "Replay last checkpoint",
                "Prioritize highest-severity target"
            };
        } else if (stopReason == "policy_violation") {
            a.actions = {
                "Rollback last patch",
                "Constrain file scope",
                "Re-run regression guard"
            };
        } else {
            a.actions = {
                "Capture fresh failure packet",
                "Re-cluster failures",
                "Escalate to guarded review"
            };
        }
        return a;
    }

    static nlohmann::json toJson(const DebugRecoveryAdvice& a) {
        return {{"category", a.category}, {"actions", a.actions}};
    }
};
