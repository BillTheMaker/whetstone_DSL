#pragma once
// Step 547: Cost Policy Guard

#include <string>
#include <vector>

#include "PolicyDecisionUtil.h"

struct CostPolicyInput {
    int stepTokens = 0;
    int workflowTokens = 0;
    int stepTokenCeiling = 0;
    int workflowTokenCeiling = 0;
    bool hasRationale = false;
    bool hasEscalationApproval = false;
};

struct CostPolicyResult {
    bool allowed = false;
    bool requiresEscalation = false;
    std::vector<std::string> violations;
    std::string action = "allow"; // allow/request_rationale/escalate/block
};

class CostPolicyGuard {
public:
    static CostPolicyResult evaluate(const CostPolicyInput& input) {
        CostPolicyResult out;

        const bool stepOver = input.stepTokenCeiling > 0 && input.stepTokens > input.stepTokenCeiling;
        const bool workflowOver = input.workflowTokenCeiling > 0 &&
                                  input.workflowTokens > input.workflowTokenCeiling;

        if (!stepOver && !workflowOver) {
            setPolicyAction(out.allowed, out.requiresEscalation, out.action, "allow");
            return out;
        }

        if (stepOver) out.violations.push_back("step_token_ceiling_exceeded");
        if (workflowOver) out.violations.push_back("workflow_token_ceiling_exceeded");

        if (!input.hasRationale) {
            setPolicyAction(out.allowed, out.requiresEscalation, out.action, "request_rationale");
            return out;
        }

        // With rationale, require explicit escalation approval for overage execution.
        if (!input.hasEscalationApproval) {
            setPolicyAction(out.allowed, out.requiresEscalation, out.action, "escalate");
            return out;
        }

        setPolicyAction(out.allowed, out.requiresEscalation, out.action, "allow");
        out.requiresEscalation = true;
        return out;
    }
};
