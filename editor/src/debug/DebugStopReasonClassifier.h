#pragma once
// Step 1532: debug stop-reason classifier model.

#include <string>

#include <nlohmann/json.hpp>

struct DebugStopReason {
    std::string code = "unknown";
    std::string summary = "Unknown stop reason";
};

class DebugStopReasonClassifier {
public:
    static DebugStopReason classify(bool budgetExceeded,
                                    bool policyViolation,
                                    bool testsFailing,
                                    bool patchRejected) {
        DebugStopReason r;
        if (policyViolation) {
            r.code = "policy_violation";
            r.summary = "Policy constraint violated";
        } else if (budgetExceeded) {
            r.code = "budget_exhausted";
            r.summary = "Budget exhausted before green";
        } else if (patchRejected) {
            r.code = "patch_rejected";
            r.summary = "Patch proposal rejected by checks";
        } else if (testsFailing) {
            r.code = "tests_still_failing";
            r.summary = "Tests still failing after attempts";
        }
        return r;
    }

    static nlohmann::json toJson(const DebugStopReason& r) {
        return {{"code", r.code}, {"summary", r.summary}};
    }
};
