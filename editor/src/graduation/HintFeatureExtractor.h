#pragma once
// Step 859: Hint feature extraction from decision ledgers.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct HintFeature {
    std::string featureId;
    std::string source;    // "decision_ledger", "review_board"
    std::string pairId;
    std::string attribute;
    std::string value;
    float weight = 1.0f;
};

class HintFeatureExtractor {
public:
    static std::vector<HintFeature> extract(const std::string& pairId,
                                             const std::vector<nlohmann::json>& decisions) {
        std::vector<HintFeature> features;
        int idx = 0;
        for (const auto& d : decisions) {
            HintFeature f;
            f.featureId = "HF-" + std::to_string(++idx);
            f.pairId = pairId;
            f.source = "decision_ledger";
            f.attribute = d.value("decision", "unknown");
            f.value = d.value("rationale", "");
            f.weight = d.value("weight", 1.0f);
            features.push_back(f);
        }
        return features;
    }

    static nlohmann::json toJson(const HintFeature& f) {
        return {{"feature_id", f.featureId}, {"pair_id", f.pairId},
                {"source", f.source}, {"attribute", f.attribute},
                {"value", f.value}, {"weight", f.weight}};
    }
};
