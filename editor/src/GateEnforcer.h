#pragma once
// Step 1874: Gate enforcement — block-severity gates halt production.
// GateSeverityPolicy classifies gate severity; GateEnforcer evaluates a list
// of check results and returns a blocked decision when any "block" gate fails.

#include "gates/GateSeverityPolicy.h"

#include <string>
#include <vector>

struct GateCheckResult {
    std::string gate;
    bool passed;
};

struct GateEnforcementResult {
    bool blocked = false;
    std::vector<std::string> blockingGates;
};

class GateEnforcer {
public:
    static GateEnforcementResult evaluate(const std::vector<GateCheckResult>& checks) {
        GateEnforcementResult result;
        for (const auto& check : checks) {
            if (!check.passed && GateSeverityPolicy::severityFor(check.gate) == "block") {
                result.blocked = true;
                result.blockingGates.push_back(check.gate);
            }
        }
        return result;
    }
};
