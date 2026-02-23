#pragma once
// Step 897: Runtime compatibility risk report generator.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct RuntimeRiskItem {
    std::string itemId;
    std::string category;
    std::string description;
    float severity = 0.0f;  // 0-1
};

struct RuntimeCompatibilityReport {
    std::string pairId;
    std::string sourceRuntime;
    std::string targetRuntime;
    std::vector<RuntimeRiskItem> riskItems;
    float overallRisk = 0.0f;
    std::string riskLabel;  // "low","medium","high"
    bool missingPackData = false;
};

class RuntimeCompatibilityRiskReport {
public:
    static RuntimeCompatibilityReport generate(
            const std::string& pairId,
            const std::string& sourceRuntime,
            const std::string& targetRuntime,
            bool sourcePackAvailable,
            bool targetPackAvailable) {
        RuntimeCompatibilityReport r;
        r.pairId = pairId;
        r.sourceRuntime = sourceRuntime;
        r.targetRuntime = targetRuntime;
        r.missingPackData = !sourcePackAvailable || !targetPackAvailable;
        if (r.missingPackData) {
            r.riskItems.push_back({"R-1", "pack_data", "missing_runtime_pack", 0.6f});
            r.overallRisk = 0.6f;
        }
        if (!r.riskItems.empty()) {
            float sum = 0.0f;
            for (const auto& ri : r.riskItems) sum += ri.severity;
            r.overallRisk = sum / r.riskItems.size();
        }
        if (r.overallRisk < 0.33f)      r.riskLabel = "low";
        else if (r.overallRisk < 0.66f) r.riskLabel = "medium";
        else                            r.riskLabel = "high";
        return r;
    }

    static nlohmann::json toJson(const RuntimeCompatibilityReport& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& ri : r.riskItems)
            arr.push_back({{"item_id", ri.itemId}, {"category", ri.category},
                           {"severity", ri.severity}});
        return {{"pair_id", r.pairId}, {"source_runtime", r.sourceRuntime},
                {"target_runtime", r.targetRuntime},
                {"overall_risk", r.overallRisk}, {"risk_label", r.riskLabel},
                {"missing_pack_data", r.missingPackData}, {"risk_items", arr}};
    }
};
