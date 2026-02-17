#pragma once
// Step 605: Deployment Promotion Gate

#include <map>
#include <string>
#include <vector>

#include "ValidationErrorUtil.h"

struct EnvironmentGateState {
    std::string environmentId;
    int requiredChecks = 0;
    int passedChecks = 0;
    int manualApprovals = 0;
    int blockingIssues = 0;
};

class DeploymentPromotionGate {
public:
    bool upsert(const EnvironmentGateState& state, std::string* error) {
        if (!error) return false;
        error->clear();
        if (state.environmentId.empty()) return failWith(error, "environment_id_missing");
        if (state.requiredChecks < 0) return failWith(error, "required_checks_invalid");
        if (state.passedChecks < 0) return failWith(error, "passed_checks_invalid");
        if (state.manualApprovals < 0) return failWith(error, "manual_approvals_invalid");
        if (state.blockingIssues < 0) return failWith(error, "blocking_issues_invalid");
        if (state.passedChecks > state.requiredChecks) return failWith(error, "passed_checks_exceed_required");
        if (states_.count(state.environmentId) == 0) order_.push_back(state.environmentId);
        states_[state.environmentId] = state;
        return true;
    }

    bool promotable(const std::string& environmentId, std::string* error) const {
        if (!error) return false;
        error->clear();
        auto it = states_.find(environmentId);
        if (it == states_.end()) return failWith(error, "environment_missing");
        const auto& state = it->second;
        return state.requiredChecks > 0 &&
               state.passedChecks == state.requiredChecks &&
               state.manualApprovals > 0 &&
               state.blockingIssues == 0;
    }

    int promotionScore(const std::string& environmentId, std::string* error) const {
        if (!error) return 0;
        error->clear();
        auto it = states_.find(environmentId);
        if (it == states_.end()) {
            failWith(error, "environment_missing");
            return 0;
        }
        const auto& state = it->second;
        if (state.requiredChecks == 0) return 0;
        int score = (state.passedChecks * 100) / state.requiredChecks;
        score += state.manualApprovals * 5;
        score -= state.blockingIssues * 20;
        if (score < 0) return 0;
        if (score > 100) return 100;
        return score;
    }

    std::vector<std::string> blockedEnvironments() const {
        std::vector<std::string> blocked;
        for (const auto& id : order_) {
            const auto& state = states_.at(id);
            if (state.blockingIssues > 0 || state.passedChecks < state.requiredChecks) blocked.push_back(id);
        }
        return blocked;
    }

private:
    std::map<std::string, EnvironmentGateState> states_;
    std::vector<std::string> order_;
};
