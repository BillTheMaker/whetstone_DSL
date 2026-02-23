#pragma once
// Step 873: Budget policy enforcer.
#include <string>
#include <nlohmann/json.hpp>

struct BudgetPolicy {
    float maxTotalCost = 1.0f;
    bool requireApprovalTokenForOverage = true;
};

struct BudgetEnforcementResult {
    std::string pairId;
    bool withinBudget = false;
    bool requiresApprovalToken = false;
    float requestedCost = 0.0f;
    float budgetLimit = 0.0f;
    std::string decision; // "approved", "requires_token", "rejected"
};

class BudgetPolicyEnforcer {
public:
    static BudgetEnforcementResult enforce(const std::string& pairId,
                                             float requestedCost,
                                             const BudgetPolicy& policy,
                                             bool hasApprovalToken = false) {
        BudgetEnforcementResult r;
        r.pairId = pairId;
        r.requestedCost = requestedCost;
        r.budgetLimit = policy.maxTotalCost;
        r.withinBudget = requestedCost <= policy.maxTotalCost;
        if (r.withinBudget) {
            r.decision = "approved";
        } else if (policy.requireApprovalTokenForOverage && hasApprovalToken) {
            r.requiresApprovalToken = false;
            r.decision = "approved";
        } else if (policy.requireApprovalTokenForOverage) {
            r.requiresApprovalToken = true;
            r.decision = "requires_token";
        } else {
            r.decision = "rejected";
        }
        return r;
    }

    static nlohmann::json toJson(const BudgetEnforcementResult& r) {
        return {{"pair_id", r.pairId}, {"within_budget", r.withinBudget},
                {"requires_approval_token", r.requiresApprovalToken},
                {"requested_cost", r.requestedCost},
                {"budget_limit", r.budgetLimit},
                {"decision", r.decision}};
    }
};
