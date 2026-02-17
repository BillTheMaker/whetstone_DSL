#pragma once
// Step 548: Phase 29a Integration

#include <map>
#include <string>
#include <vector>

#include "ConstrainedRoutingRulesetExtension.h"
#include "ContextBundleMinimizer.h"
#include "ConstrainedConfidenceCalibrator.h"
#include "CostPolicyGuard.h"

struct Phase29aRequest {
    ConstrainedRoutingInput routing;
    ContextBundleInput context;
    double baseConfidence = 0.0;
    std::string preferredRoute;
    std::map<std::string, int> successes;
    std::map<std::string, int> attempts;
    std::map<std::string, int> postApplyFailures;
    CostPolicyInput cost;
};

struct Phase29aResult {
    bool passed = false;
    ConstrainedRoutingDecision routingDecision;
    ContextBundleOutput minimizedContext;
    CalibratedConfidence calibrated;
    CostPolicyResult costPolicy;
    std::vector<std::string> errors;
};

class Phase29aIntegration {
public:
    static Phase29aResult run(const Phase29aRequest& request) {
        Phase29aResult out;
        out.routingDecision = ConstrainedRoutingRulesetExtension::decide(request.routing);
        out.minimizedContext = ContextBundleMinimizer::minimize(request.context);
        out.calibrated = ConstrainedConfidenceCalibrator::calibrate(
            request.baseConfidence,
            request.successes,
            request.attempts,
            request.postApplyFailures,
            request.preferredRoute);
        out.costPolicy = CostPolicyGuard::evaluate(request.cost);

        if (!out.costPolicy.allowed) {
            out.errors.push_back("cost_policy_blocked");
        }

        if (out.routingDecision.route == "escalate") {
            out.errors.push_back("routing_escalated");
        }

        if (request.context.narrowOperation &&
            out.minimizedContext.minimalContext.size() >
                request.context.fileContext.size() + 2) {
            out.errors.push_back("context_not_sufficiently_minimized");
        }

        if (out.calibrated.adjustedConfidence < 0.40 &&
            out.routingDecision.route == "deterministic_template") {
            out.errors.push_back("low_calibrated_confidence_for_template_route");
        }

        out.passed = out.errors.empty();
        return out;
    }
};
