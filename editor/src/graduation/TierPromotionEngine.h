#pragma once
// Step 830: Tier promotion engine (beta->stable).
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct PromotionRequest {
    std::string pairId;  // "python->cpp"
    std::string currentTier;
    std::string targetTier;
    int passingTests = 0;
    int requiredTests = 0;
};

struct PromotionResult {
    std::string pairId;
    bool promoted = false;
    std::string newTier;
    std::string reason;
};

class TierPromotionEngine {
public:
    static PromotionResult evaluate(const PromotionRequest& req, std::string* error = nullptr) {
        PromotionResult r;
        r.pairId = req.pairId;
        if (req.pairId.empty()) {
            if (error) *error = "pair_id_missing";
            return r;
        }
        if (req.currentTier == "experimental" && req.targetTier == "beta") {
            r.promoted = req.passingTests >= req.requiredTests && req.requiredTests > 0;
        } else if (req.currentTier == "beta" && req.targetTier == "stable") {
            r.promoted = req.passingTests >= req.requiredTests && req.requiredTests > 0;
        } else {
            r.promoted = false;
            r.reason = "invalid_tier_transition";
        }
        r.newTier = r.promoted ? req.targetTier : req.currentTier;
        if (r.promoted) r.reason = "criteria_met";
        else if (r.reason.empty()) r.reason = "insufficient_tests";
        return r;
    }

    static nlohmann::json toJson(const PromotionResult& r) {
        return {{"pair_id", r.pairId}, {"promoted", r.promoted},
                {"new_tier", r.newTier}, {"reason", r.reason}};
    }
};
