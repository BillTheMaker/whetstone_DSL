#pragma once
// Step 860: Pair-specific hint model interface.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct AdapterHint {
    std::string hintId;
    std::string pairId;
    std::string suggestion;
    float confidence = 0.0f;
    bool nonAuthoritative = true;
};

struct HintModelResult {
    std::string pairId;
    std::vector<AdapterHint> hints;
    bool modelAvailable = false;
};

class HintModelInterface {
public:
    static HintModelResult query(const std::string& pairId,
                                  const std::vector<std::string>& features) {
        HintModelResult r;
        r.pairId = pairId;
        r.modelAvailable = !pairId.empty();
        if (r.modelAvailable) {
            int idx = 0;
            for (const auto& f : features) {
                AdapterHint h;
                h.hintId = "H-" + std::to_string(++idx);
                h.pairId = pairId;
                h.suggestion = "consider_" + f;
                h.confidence = 0.75f;
                h.nonAuthoritative = true;
                r.hints.push_back(h);
            }
        }
        return r;
    }

    static nlohmann::json toJson(const HintModelResult& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& h : r.hints)
            arr.push_back({{"hint_id", h.hintId}, {"suggestion", h.suggestion},
                           {"confidence", h.confidence}, {"non_authoritative", h.nonAuthoritative}});
        return {{"pair_id", r.pairId}, {"model_available", r.modelAvailable}, {"hints", arr}};
    }
};
