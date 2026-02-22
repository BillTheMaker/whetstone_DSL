#pragma once
// Step 1476: SLM debug readiness scoring.

#include <string>

#include <nlohmann/json.hpp>

struct SLMDebugReadinessScore {
    double score = 0.0;
    std::string grade; // blocked|pilot|ready
    std::string rationale;
};

class SLMDebugReadinessModel {
public:
    static SLMDebugReadinessScore score(const nlohmann::json& metrics) {
        SLMDebugReadinessScore out;
        int acceptance = metrics.value("patch_acceptance_rate_pct", 0);
        int regression = metrics.value("regression_escape_rate_pct", 100);
        int compression = metrics.value("symptom_to_root_cause_compression", 0);

        out.score = 0.5 * acceptance + 0.3 * (100 - regression) + 0.2 * std::min(100, compression / 4);
        if (out.score >= 75) out.grade = "ready";
        else if (out.score >= 50) out.grade = "pilot";
        else out.grade = "blocked";
        out.rationale = "acceptance=" + std::to_string(acceptance) + ", regression=" + std::to_string(regression);
        return out;
    }

    static nlohmann::json toJson(const SLMDebugReadinessScore& r) {
        return {{"score", r.score}, {"grade", r.grade}, {"rationale", r.rationale}};
    }
};
