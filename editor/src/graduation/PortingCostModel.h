#pragma once
// Step 869: Porting cost model v1.
#include <string>
#include <nlohmann/json.hpp>

struct PortingCostEstimate {
    std::string pairId;
    float computeCost = 0.0f;   // normalized 0-1
    float reviewCost = 0.0f;    // normalized 0-1
    float totalCost = 0.0f;
    std::string tier;           // "low", "medium", "high"
};

class PortingCostModel {
public:
    static PortingCostEstimate estimate(const std::string& pairId,
                                        int linesOfCode, float ambiguityScore) {
        PortingCostEstimate e;
        e.pairId = pairId;
        e.computeCost = std::min(1.0f, linesOfCode / 1000.0f);
        e.reviewCost = std::min(1.0f, ambiguityScore);
        e.totalCost = (e.computeCost + e.reviewCost) / 2.0f;
        if (e.totalCost < 0.33f)      e.tier = "low";
        else if (e.totalCost < 0.66f) e.tier = "medium";
        else                          e.tier = "high";
        return e;
    }

    static nlohmann::json toJson(const PortingCostEstimate& e) {
        return {{"pair_id", e.pairId}, {"compute_cost", e.computeCost},
                {"review_cost", e.reviewCost}, {"total_cost", e.totalCost},
                {"tier", e.tier}};
    }
};
