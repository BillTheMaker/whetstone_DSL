#pragma once
// Step 877: Cost telemetry integration in metrics stack.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct CostTelemetryRecord {
    std::string pairId;
    std::string profile;
    float actualCost = 0.0f;
    float estimatedCost = 0.0f;
    float costError = 0.0f; // |actual - estimated|
    bool overBudget = false;
};

class CostTelemetryIntegration {
public:
    void record(const CostTelemetryRecord& r) {
        records_.push_back(r);
    }

    float averageCostError() const {
        if (records_.empty()) return 0.0f;
        float sum = 0.0f;
        for (const auto& r : records_) sum += r.costError;
        return sum / records_.size();
    }

    int overBudgetCount() const {
        int n = 0;
        for (const auto& r : records_) if (r.overBudget) ++n;
        return n;
    }

    static CostTelemetryRecord make(const std::string& pairId, const std::string& profile,
                                     float actual, float estimated) {
        CostTelemetryRecord r;
        r.pairId = pairId;
        r.profile = profile;
        r.actualCost = actual;
        r.estimatedCost = estimated;
        r.costError = std::abs(actual - estimated);
        r.overBudget = actual > 1.0f;
        return r;
    }

    static nlohmann::json toJson(const CostTelemetryRecord& r) {
        return {{"pair_id", r.pairId}, {"profile", r.profile},
                {"actual_cost", r.actualCost}, {"estimated_cost", r.estimatedCost},
                {"cost_error", r.costError}, {"over_budget", r.overBudget}};
    }

private:
    std::vector<CostTelemetryRecord> records_;
};
