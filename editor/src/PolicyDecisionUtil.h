#pragma once
// Sprint 29 refactor: shared policy decision helpers

#include <string>
#include <vector>

inline void setEscalateDecision(bool& requiresHumanReview,
                                std::string& decision,
                                std::vector<std::string>& reasons,
                                const std::string& reason) {
    requiresHumanReview = true;
    decision = "escalate_review";
    if (!reason.empty()) reasons.push_back(reason);
}

inline void setPolicyAction(bool& allowed,
                            bool& requiresEscalation,
                            std::string& action,
                            const std::string& nextAction) {
    action = nextAction;
    if (nextAction == "allow") {
        allowed = true;
        return;
    }
    allowed = false;
    requiresEscalation = (nextAction == "escalate");
}
