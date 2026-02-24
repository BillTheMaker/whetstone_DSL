#pragma once
// Step 1153: Divergence triage model for interop failures.
#include <string>
#include <nlohmann/json.hpp>

struct DivergenceTriageModelForInteropFailures {
    std::string triageId;
    std::string failureClass;
    std::string severity;
    std::string recommendedAction;
    bool requiresEscalation = false;
    bool valid = false;
};

class DivergenceTriageModelForInteropFailuresFactory {
public:
    static DivergenceTriageModelForInteropFailures make(const std::string& triageId,
                                                        const std::string& failureClass,
                                                        const std::string& severity,
                                                        const std::string& recommendedAction) {
        bool requiresEscalation = severity == "critical" || severity == "high";
        bool valid = !triageId.empty() && !failureClass.empty() && !severity.empty() && !recommendedAction.empty();
        return {triageId, failureClass, severity, recommendedAction, requiresEscalation, valid};
    }

    static nlohmann::json toJson(const DivergenceTriageModelForInteropFailures& t) {
        return {{"triage_id", t.triageId},
                {"failure_class", t.failureClass},
                {"severity", t.severity},
                {"recommended_action", t.recommendedAction},
                {"requires_escalation", t.requiresEscalation},
                {"valid", t.valid}};
    }
};
