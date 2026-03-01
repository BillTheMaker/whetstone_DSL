#pragma once
// Step 1881c: C++ constrained refactor policy (GR-018).
// Closes GR-018: enforcement was contract-level but generator capability under
// constraints was weak in the C++ AB path. This policy validates that a C++
// refactor proposal satisfies the required constraint signals before the generator
// is invoked — preventing constraint-violating code from being emitted.

#include <string>
#include <vector>

struct RefactorConstraints {
    bool requiresRollbackPlan = false;
    bool requiresSecurityReview = false;
    bool requiresSloP95 = false;
    bool requiresStagedRollout = false;
};

struct CppRefactorPolicyResult {
    bool allowed = false;
    std::vector<std::string> violations;
};

class CppConstraintRefactorPolicy {
public:
    static CppRefactorPolicyResult evaluate(
        const RefactorConstraints& required,
        bool hasRollbackPlan,
        bool hasSecurityReview,
        bool hasSloP95,
        bool hasStagedRollout)
    {
        CppRefactorPolicyResult result;

        if (required.requiresRollbackPlan && !hasRollbackPlan)
            result.violations.push_back("missing_rollback_plan");

        if (required.requiresSecurityReview && !hasSecurityReview)
            result.violations.push_back("missing_security_review");

        if (required.requiresSloP95 && !hasSloP95)
            result.violations.push_back("missing_slo_p95");

        if (required.requiresStagedRollout && !hasStagedRollout)
            result.violations.push_back("missing_staged_rollout");

        result.allowed = result.violations.empty();
        return result;
    }
};
