#pragma once
// Step 595: Approval Escalation Planner

#include "PolicyGuardrailCatalog.h"

#include <string>
#include <vector>

enum class EscalationLevel {
    None,
    Reviewer,
    Admin
};

struct EscalationPlan {
    std::string operation;
    GuardrailDecision decision = GuardrailDecision::RequireReview;
    EscalationLevel level = EscalationLevel::Reviewer;
    bool requiresJustification = true;
    std::vector<std::string> approverRoles;
    std::string reason;
};

class ApprovalEscalationPlanner {
public:
    static EscalationPlan planFor(const PolicyGuardrailCatalog& catalog,
                                  const std::string& operation) {
        EscalationPlan plan;
        plan.operation = operation;
        const auto decision = catalog.evaluate(operation);
        plan.decision = decision.decision;
        plan.reason = decision.reason;

        if (decision.decision == GuardrailDecision::Allow) {
            plan.level = EscalationLevel::None;
            plan.requiresJustification = false;
            plan.approverRoles = {};
            return plan;
        }
        if (decision.decision == GuardrailDecision::Deny) {
            plan.level = EscalationLevel::Admin;
            plan.requiresJustification = true;
            plan.approverRoles = {"security-admin", "platform-owner"};
            return plan;
        }

        plan.level = EscalationLevel::Reviewer;
        plan.requiresJustification = true;
        plan.approverRoles = {"reviewer"};
        return plan;
    }

    static bool isExecutable(const EscalationPlan& plan) {
        if (plan.decision == GuardrailDecision::Deny) return false;
        return plan.level == EscalationLevel::None ||
               plan.level == EscalationLevel::Reviewer;
    }

    static int riskScore(const EscalationPlan& plan) {
        int score = 20;
        if (plan.decision == GuardrailDecision::RequireReview) score += 30;
        if (plan.decision == GuardrailDecision::Deny) score += 60;
        if (plan.requiresJustification) score += 10;
        if (plan.level == EscalationLevel::Admin) score += 10;
        if (score > 100) score = 100;
        return score;
    }
};
