#pragma once
// Step 867: Hint safety guardrails and audit fields.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct HintAuditRecord {
    std::string hintId;
    std::string pairId;
    std::string appliedAt;
    bool guardPassed = false;
    std::string guardReason;
    bool nonAuthoritative = true;
};

struct GuardrailCheckResult {
    std::string hintId;
    bool passed = false;
    std::vector<std::string> violations;
};

class HintSafetyGuardrails {
public:
    static GuardrailCheckResult check(const std::string& hintId,
                                       float confidence,
                                       bool nonAuthoritative,
                                       bool deterministicPolicyExists) {
        GuardrailCheckResult r;
        r.hintId = hintId;
        r.passed = true;
        if (!nonAuthoritative) {
            r.violations.push_back("hint_must_be_non_authoritative");
            r.passed = false;
        }
        if (!deterministicPolicyExists) {
            r.violations.push_back("deterministic_policy_required");
            r.passed = false;
        }
        if (confidence < 0.0f || confidence > 1.0f) {
            r.violations.push_back("invalid_confidence_range");
            r.passed = false;
        }
        return r;
    }

    static HintAuditRecord audit(const std::string& hintId, const std::string& pairId,
                                  const GuardrailCheckResult& check) {
        HintAuditRecord rec;
        rec.hintId = hintId;
        rec.pairId = pairId;
        rec.guardPassed = check.passed;
        rec.guardReason = check.passed ? "all_guards_passed" :
            (check.violations.empty() ? "" : check.violations[0]);
        rec.nonAuthoritative = true;
        return rec;
    }

    static nlohmann::json toJson(const HintAuditRecord& r) {
        return {{"hint_id", r.hintId}, {"pair_id", r.pairId},
                {"guard_passed", r.guardPassed}, {"guard_reason", r.guardReason},
                {"non_authoritative", r.nonAuthoritative}};
    }
};
