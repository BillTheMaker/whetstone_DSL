#pragma once
// Step 901: Migration feasibility engine — scores a runtime pair and emits a verdict.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct FeasibilityResult {
    std::string pairId;
    float score = 0.0f;                 // 0.0 – 1.0
    std::string verdict;                // "feasible" | "marginal" | "infeasible"
    std::vector<std::string> blockers;  // reasons that reduce feasibility
};

class MigrationFeasibilityEngine {
public:
    // Assess migration feasibility for a runtime pair.
    //   riskScore     : normalised risk [0.0, 1.0]; higher means more risk
    //   hasSourcePack : true when a RuntimeSemanticsPack exists for srcRuntime
    //   hasTargetPack : true when a RuntimeSemanticsPack exists for tgtRuntime
    static FeasibilityResult assess(const std::string& pairId,
                                    const std::string& srcRuntime,
                                    const std::string& tgtRuntime,
                                    float riskScore,
                                    bool hasSourcePack,
                                    bool hasTargetPack) {
        FeasibilityResult r;
        r.pairId = pairId;

        // Base score starts at 1.0 and is reduced by risk and missing data.
        float score = 1.0f;

        // Clamp riskScore to [0, 1].
        if (riskScore < 0.0f) riskScore = 0.0f;
        if (riskScore > 1.0f) riskScore = 1.0f;
        score -= riskScore * 0.5f;    // risk contributes up to 0.5 penalty

        if (!hasSourcePack) {
            score -= 0.2f;
            r.blockers.push_back("missing_pack:" + srcRuntime);
        }
        if (!hasTargetPack) {
            score -= 0.2f;
            r.blockers.push_back("missing_pack:" + tgtRuntime);
        }
        if (srcRuntime.empty()) {
            score -= 0.1f;
            r.blockers.push_back("empty_source_runtime");
        }
        if (tgtRuntime.empty()) {
            score -= 0.1f;
            r.blockers.push_back("empty_target_runtime");
        }

        if (score < 0.0f) score = 0.0f;
        r.score = score;

        if (score >= 0.6f)       r.verdict = "feasible";
        else if (score >= 0.35f) r.verdict = "marginal";
        else                     r.verdict = "infeasible";

        return r;
    }

    static nlohmann::json toJson(const FeasibilityResult& r) {
        nlohmann::json blockerArr = nlohmann::json::array();
        for (const auto& b : r.blockers) blockerArr.push_back(b);
        return {
            {"pair_id",  r.pairId},
            {"score",    r.score},
            {"verdict",  r.verdict},
            {"blockers", blockerArr}
        };
    }
};
