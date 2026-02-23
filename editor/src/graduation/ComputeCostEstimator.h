#pragma once
// Step 871: Compute/runtime cost estimator from prior runs.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct PriorRunRecord {
    std::string pairId;
    int durationMs = 0;
    int tokenCount = 0;
};

struct ComputeCostEstimate {
    std::string pairId;
    float estimatedDurationMs = 0.0f;
    float estimatedTokens = 0.0f;
    int sampleCount = 0;
    bool hasHistory = false;
};

class ComputeCostEstimator {
public:
    static ComputeCostEstimate estimate(const std::string& pairId,
                                         const std::vector<PriorRunRecord>& history) {
        ComputeCostEstimate e;
        e.pairId = pairId;
        e.sampleCount = static_cast<int>(history.size());
        e.hasHistory = !history.empty();
        if (e.hasHistory) {
            float sumDur = 0, sumTok = 0;
            for (const auto& r : history) {
                sumDur += r.durationMs;
                sumTok += r.tokenCount;
            }
            e.estimatedDurationMs = sumDur / e.sampleCount;
            e.estimatedTokens = sumTok / e.sampleCount;
        }
        return e;
    }

    static nlohmann::json toJson(const ComputeCostEstimate& e) {
        return {{"pair_id", e.pairId},
                {"estimated_duration_ms", e.estimatedDurationMs},
                {"estimated_tokens", e.estimatedTokens},
                {"sample_count", e.sampleCount},
                {"has_history", e.hasHistory}};
    }
};
