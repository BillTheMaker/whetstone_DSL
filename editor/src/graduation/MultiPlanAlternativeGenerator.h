#pragma once
// Step 872: Multi-plan alternative generator.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct TranspilationPlanAlternative {
    std::string planId;
    std::string profile; // "cheap", "balanced", "rigorous"
    float estimatedCost = 0.0f;
    float estimatedQuality = 0.0f;
    bool requiresReview = false;
};

class MultiPlanAlternativeGenerator {
public:
    static std::vector<TranspilationPlanAlternative> generate(
            const std::string& pairId, float baseCost) {
        (void)pairId;
        std::vector<TranspilationPlanAlternative> plans;
        plans.push_back({"plan-cheap",     "cheap",     baseCost * 0.5f, 0.6f, false});
        plans.push_back({"plan-balanced",  "balanced",  baseCost,        0.8f, false});
        plans.push_back({"plan-rigorous",  "rigorous",  baseCost * 2.0f, 0.95f, true});
        return plans;
    }

    static nlohmann::json toJson(const TranspilationPlanAlternative& p) {
        return {{"plan_id", p.planId}, {"profile", p.profile},
                {"estimated_cost", p.estimatedCost},
                {"estimated_quality", p.estimatedQuality},
                {"requires_review", p.requiresReview}};
    }
};
