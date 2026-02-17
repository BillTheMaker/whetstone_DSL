#pragma once
// Step 551: Review-Gate Policy Refinement

#include <string>
#include <vector>

struct ReviewGateInput {
    bool safetyCritical = false;
    bool securitySensitive = false;
    bool policyViolationDetected = false;
    int confidenceScore = 0;          // 0..100
    int postApplyFailureStreak = 0;
    bool deterministicTemplatePath = false;
    bool highCostOverage = false;
};

struct ReviewGateResult {
    bool requiresHumanReview = false;
    std::string decision; // auto_approve/auto_approve_with_audit/escalate_review/block
    std::vector<std::string> reasons;
};

class ReviewGatePolicyRefinement {
public:
    static ReviewGateResult evaluate(const ReviewGateInput& input) {
        ReviewGateResult out;

        if (input.policyViolationDetected) {
            out.requiresHumanReview = true;
            out.decision = "block";
            out.reasons.push_back("policy_violation_detected");
            return out;
        }

        if (input.safetyCritical || input.securitySensitive) {
            out.requiresHumanReview = true;
            out.decision = "escalate_review";
            if (input.safetyCritical) out.reasons.push_back("safety_critical_change");
            if (input.securitySensitive) out.reasons.push_back("security_sensitive_change");
            return out;
        }

        if (input.postApplyFailureStreak >= 3) {
            out.requiresHumanReview = true;
            out.decision = "escalate_review";
            out.reasons.push_back("repeated_post_apply_failures");
            return out;
        }

        if (input.confidenceScore >= 85 && input.deterministicTemplatePath && !input.highCostOverage) {
            out.requiresHumanReview = false;
            out.decision = "auto_approve";
            out.reasons.push_back("high_confidence_deterministic_path");
            return out;
        }

        if (input.confidenceScore >= 70 && !input.highCostOverage) {
            out.requiresHumanReview = false;
            out.decision = "auto_approve_with_audit";
            out.reasons.push_back("moderate_confidence_low_risk");
            return out;
        }

        out.requiresHumanReview = true;
        out.decision = "escalate_review";
        if (input.highCostOverage) out.reasons.push_back("high_cost_overage");
        if (input.confidenceScore < 70) out.reasons.push_back("low_confidence");
        return out;
    }
};
