#pragma once
// Step 899: Migration path candidate — source/target runtime pair with feasibility score.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct MigrationPathCandidate {
    std::string pairId;
    std::string sourceRuntime;
    std::string targetRuntime;
    std::vector<std::string> steps;
    float feasibility = 0.0f;   // 0.0 (impossible) to 1.0 (fully feasible)
    std::string rationale;
};

class MigrationPathCandidateFactory {
public:
    static MigrationPathCandidate make(const std::string& pairId,
                                       const std::string& sourceRuntime,
                                       const std::string& targetRuntime,
                                       const std::vector<std::string>& steps,
                                       float feasibility,
                                       const std::string& rationale) {
        MigrationPathCandidate c;
        c.pairId        = pairId;
        c.sourceRuntime = sourceRuntime;
        c.targetRuntime = targetRuntime;
        c.steps         = steps;
        c.feasibility   = feasibility < 0.0f ? 0.0f : (feasibility > 1.0f ? 1.0f : feasibility);
        c.rationale     = rationale;
        return c;
    }

    static nlohmann::json toJson(const MigrationPathCandidate& c) {
        nlohmann::json stepArr = nlohmann::json::array();
        for (const auto& s : c.steps) stepArr.push_back(s);
        return {
            {"pair_id",        c.pairId},
            {"source_runtime", c.sourceRuntime},
            {"target_runtime", c.targetRuntime},
            {"steps",          stepArr},
            {"feasibility",    c.feasibility},
            {"rationale",      c.rationale}
        };
    }
};
