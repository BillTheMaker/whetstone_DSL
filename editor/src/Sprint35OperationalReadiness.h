#pragma once
// Step 613: Sprint 35 Operational Readiness

#include <string>
#include <vector>

#include "ScoreClampUtil.h"
#include "ValidationErrorUtil.h"

struct Sprint35ReadinessInput {
    int freezeViolations = 0;
    int highRiskDependencies = 0;
    int onCallCoverageGaps = 0;
    int blockedCanaries = 0;
    int incompletePostmortems = 0;
};

class Sprint35OperationalReadiness {
public:
    static bool validate(const Sprint35ReadinessInput& in, std::string* error) {
        if (!error) return false;
        error->clear();
        if (in.freezeViolations < 0) return failWith(error, "freeze_violations_invalid");
        if (in.highRiskDependencies < 0) return failWith(error, "high_risk_dependencies_invalid");
        if (in.onCallCoverageGaps < 0) return failWith(error, "oncall_gaps_invalid");
        if (in.blockedCanaries < 0) return failWith(error, "blocked_canaries_invalid");
        if (in.incompletePostmortems < 0) return failWith(error, "incomplete_postmortems_invalid");
        return true;
    }

    static int readinessScore(const Sprint35ReadinessInput& in) {
        int score = 100;
        score -= in.freezeViolations * 12;
        score -= in.highRiskDependencies * 8;
        score -= in.onCallCoverageGaps;
        score -= in.blockedCanaries * 15;
        score -= in.incompletePostmortems * 10;
        return clampToPercent(score);
    }

    static std::vector<std::string> blockers(const Sprint35ReadinessInput& in) {
        std::vector<std::string> out;
        if (in.freezeViolations > 0) out.push_back("freeze_violations_present");
        if (in.highRiskDependencies > 2) out.push_back("dependency_risk_elevated");
        if (in.onCallCoverageGaps > 0) out.push_back("oncall_coverage_incomplete");
        if (in.blockedCanaries > 0) out.push_back("canary_promotions_blocked");
        if (in.incompletePostmortems > 0) out.push_back("postmortems_incomplete");
        return out;
    }

    static bool releaseAllowed(const Sprint35ReadinessInput& in) {
        return blockers(in).empty() && readinessScore(in) >= 90;
    }
};
