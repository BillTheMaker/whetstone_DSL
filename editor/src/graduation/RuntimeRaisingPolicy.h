#pragma once
// Step 893: Runtime-aware raising policy selectors.
#include <string>
#include <nlohmann/json.hpp>

struct RaisingPolicyResult {
    std::string pairId;
    std::string sourceRuntime;
    std::string policy;     // "strict", "relaxed", "conservative"
    bool requiresReview = false;
    std::string rationale;
};

class RuntimeRaisingPolicy {
public:
    static RaisingPolicyResult select(const std::string& pairId,
                                       const std::string& sourceRuntime,
                                       bool hasRuntimePack,
                                       float riskScore) {
        RaisingPolicyResult r;
        r.pairId = pairId;
        r.sourceRuntime = sourceRuntime;
        if (!hasRuntimePack) {
            r.policy = "conservative";
            r.requiresReview = true;
            r.rationale = "no_runtime_pack";
        } else if (riskScore >= 0.7f) {
            r.policy = "strict";
            r.requiresReview = true;
            r.rationale = "high_risk";
        } else if (riskScore >= 0.4f) {
            r.policy = "relaxed";
            r.requiresReview = false;
            r.rationale = "medium_risk";
        } else {
            r.policy = "relaxed";
            r.requiresReview = false;
            r.rationale = "low_risk";
        }
        return r;
    }

    static nlohmann::json toJson(const RaisingPolicyResult& r) {
        return {{"pair_id", r.pairId}, {"source_runtime", r.sourceRuntime},
                {"policy", r.policy}, {"requires_review", r.requiresReview},
                {"rationale", r.rationale}};
    }
};
