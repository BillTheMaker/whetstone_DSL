#pragma once
// Step 1280: Auto-tuning proposal generator from benchmark deltas.
#include <string>
#include <nlohmann/json.hpp>

struct AutoTuningProposalGeneratorFromBenchmarkDeltas {
    std::string id;
    std::string detail;
    int score = 0;
    bool enabled = false;
    bool valid = false;
};

class AutoTuningProposalGeneratorFromBenchmarkDeltasFactory {
public:
    static AutoTuningProposalGeneratorFromBenchmarkDeltas make(const std::string& id,
                     const std::string& detail,
                     int score,
                     bool enabled) {
        bool valid = !id.empty() && !detail.empty() && score >= 0;
        return {id, detail, score, enabled, valid};
    }

    static nlohmann::json toJson(const AutoTuningProposalGeneratorFromBenchmarkDeltas& v) {
        nlohmann::json j = nlohmann::json::object();
        j["id"] = v.id;
        j["detail"] = v.detail;
        j["score"] = v.score;
        j["enabled"] = v.enabled;
        j["valid"] = v.valid;
        return j;
    }
};
