#pragma once
// Step 1537: debug guidance packet model.

#include <nlohmann/json.hpp>

#include "DebugPolicyConstraint.h"
#include "DebugActionBudget.h"
#include "DebugRecoveryAdvisor.h"
#include "DebugStopReasonClassifier.h"

struct DebugGuidancePacket {
    DebugPolicyConstraint constraints;
    DebugActionBudget budget;
    DebugRecoveryAdvice recovery;
    DebugStopReason stopReason;
};

class DebugGuidancePacketModel {
public:
    static DebugGuidancePacket build(const DebugPolicyConstraint& constraints,
                                     const DebugActionBudget& budget,
                                     const DebugRecoveryAdvice& recovery,
                                     const DebugStopReason& stopReason) {
        return {constraints, budget, recovery, stopReason};
    }

    static nlohmann::json toJson(const DebugGuidancePacket& p) {
        return {
            {"constraints", DebugPolicyConstraintModel::toJson(p.constraints)},
            {"budget", DebugActionBudgetModel::toJson(p.budget)},
            {"recovery", DebugRecoveryAdvisor::toJson(p.recovery)},
            {"stop_reason", DebugStopReasonClassifier::toJson(p.stopReason)}
        };
    }
};
