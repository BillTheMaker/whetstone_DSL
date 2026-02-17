#pragma once
// Step 544: Constrained Routing Ruleset Extension

#include <string>
#include <vector>

struct ConstrainedRoutingInput {
    std::string taskClass;
    double legalChoiceConfidence = 0.0;
    bool deterministicTemplateAvailable = false;
    bool constrainedMode = true;
    bool hasRecentPostApplyFailures = false;
};

struct ConstrainedRoutingDecision {
    std::string route; // deterministic_template/constrained_worker/general_worker/escalate
    double confidence = 0.0;
    std::vector<std::string> reasons;
};

class ConstrainedRoutingRulesetExtension {
public:
    static ConstrainedRoutingDecision decide(const ConstrainedRoutingInput& input) {
        ConstrainedRoutingDecision out;

        if (!input.constrainedMode) {
            out.route = "general_worker";
            out.confidence = 0.70;
            out.reasons.push_back("constrained_mode_disabled");
            return out;
        }

        if (input.hasRecentPostApplyFailures) {
            out.route = "escalate";
            out.confidence = 0.55;
            out.reasons.push_back("recent_post_apply_failures");
            return out;
        }

        if (input.deterministicTemplateAvailable && input.legalChoiceConfidence >= 0.85) {
            out.route = "deterministic_template";
            out.confidence = input.legalChoiceConfidence;
            out.reasons.push_back("high_legal_choice_confidence");
            out.reasons.push_back("template_available");
            return out;
        }

        if (input.legalChoiceConfidence >= 0.60) {
            out.route = "constrained_worker";
            out.confidence = input.legalChoiceConfidence;
            out.reasons.push_back("moderate_legal_choice_confidence");
            return out;
        }

        out.route = "general_worker";
        out.confidence = input.legalChoiceConfidence;
        out.reasons.push_back("low_legal_choice_confidence");
        return out;
    }
};
