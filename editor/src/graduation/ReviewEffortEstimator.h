#pragma once
// Step 870: Review effort estimator from ambiguity/risk packets.
#include <string>
#include <nlohmann/json.hpp>

struct ReviewEffortPacket {
    std::string pairId;
    float ambiguity = 0.0f;
    float riskScore = 0.0f;
    int estimatedReviewMinutes = 0;
    std::string complexityLabel; // "trivial", "simple", "moderate", "complex"
};

class ReviewEffortEstimator {
public:
    static ReviewEffortPacket estimate(const std::string& pairId,
                                        float ambiguity, float riskScore) {
        ReviewEffortPacket p;
        p.pairId = pairId;
        p.ambiguity = ambiguity;
        p.riskScore = riskScore;
        float combined = (ambiguity + riskScore) / 2.0f;
        p.estimatedReviewMinutes = static_cast<int>(combined * 60.0f) + 5;
        if (combined < 0.2f)      p.complexityLabel = "trivial";
        else if (combined < 0.4f) p.complexityLabel = "simple";
        else if (combined < 0.7f) p.complexityLabel = "moderate";
        else                      p.complexityLabel = "complex";
        return p;
    }

    static nlohmann::json toJson(const ReviewEffortPacket& p) {
        return {{"pair_id", p.pairId}, {"ambiguity", p.ambiguity},
                {"risk_score", p.riskScore},
                {"estimated_review_minutes", p.estimatedReviewMinutes},
                {"complexity_label", p.complexityLabel}};
    }
};
