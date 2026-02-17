#pragma once
// Step 612: Canary Promotion Judge

#include <map>
#include <string>
#include <vector>

#include "ScoreClampUtil.h"
#include "ValidationErrorUtil.h"

struct CanaryAssessment {
    std::string canaryId;
    int requestSuccessPercent = 0;
    int latencyP95Ms = 0;
    int errorBudgetBurn = 0;
    bool manualOverride = false;
};

class CanaryPromotionJudge {
public:
    bool record(const CanaryAssessment& assessment, std::string* error) {
        if (!error) return false;
        error->clear();
        if (assessment.canaryId.empty()) return failWith(error, "canary_id_missing");
        if (assessment.requestSuccessPercent < 0 || assessment.requestSuccessPercent > 100) {
            return failWith(error, "success_percent_invalid");
        }
        if (assessment.latencyP95Ms < 0) return failWith(error, "latency_invalid");
        if (assessment.errorBudgetBurn < 0) return failWith(error, "error_budget_burn_invalid");
        if (items_.count(assessment.canaryId) == 0) order_.push_back(assessment.canaryId);
        items_[assessment.canaryId] = assessment;
        return true;
    }

    bool promotable(const std::string& canaryId, std::string* error) const {
        if (!error) return false;
        error->clear();
        auto it = items_.find(canaryId);
        if (it == items_.end()) return failWith(error, "canary_missing");
        const auto& a = it->second;
        if (a.manualOverride) return true;
        return a.requestSuccessPercent >= 99 && a.latencyP95Ms <= 300 && a.errorBudgetBurn <= 100;
    }

    int riskScore(const std::string& canaryId, std::string* error) const {
        if (!error) return 0;
        error->clear();
        auto it = items_.find(canaryId);
        if (it == items_.end()) {
            failWith(error, "canary_missing");
            return 100;
        }
        const auto& a = it->second;
        int score = 0;
        if (a.requestSuccessPercent < 99) score += (99 - a.requestSuccessPercent) * 5;
        if (a.latencyP95Ms > 300) score += (a.latencyP95Ms - 300) / 5;
        if (a.errorBudgetBurn > 100) score += (a.errorBudgetBurn - 100) / 2;
        if (a.manualOverride) score = 0;
        return clampToPercent(score);
    }

    int promotableCount() const {
        int count = 0;
        std::string error;
        for (const auto& id : order_) if (promotable(id, &error)) ++count;
        return count;
    }

private:
    std::map<std::string, CanaryAssessment> items_;
    std::vector<std::string> order_;
};
