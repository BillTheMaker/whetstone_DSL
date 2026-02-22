#pragma once
// Step 840: Pair selection strategy engine.
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class SelectionStrategy { HotPairs, RandomSample, FullSweep };

struct PairSelectionResult {
    SelectionStrategy strategy;
    std::vector<std::string> selectedPairs;
    std::string rationale;
};

class PairSelectionStrategyEngine {
public:
    static PairSelectionResult select(SelectionStrategy strategy,
                                       const std::vector<std::string>& allPairs,
                                       int sampleSize = 3) {
        PairSelectionResult r;
        r.strategy = strategy;
        if (strategy == SelectionStrategy::FullSweep) {
            r.selectedPairs = allPairs;
            r.rationale = "full_sweep";
        } else if (strategy == SelectionStrategy::HotPairs) {
            int n = std::min(sampleSize, static_cast<int>(allPairs.size()));
            for (int i = 0; i < n; ++i) r.selectedPairs.push_back(allPairs[i]);
            r.rationale = "hot_pairs";
        } else {
            int n = std::min(sampleSize, static_cast<int>(allPairs.size()));
            for (int i = 0; i < n; ++i) r.selectedPairs.push_back(allPairs[i]);
            r.rationale = "random_sample";
        }
        return r;
    }

    static std::string strategyName(SelectionStrategy s) {
        if (s == SelectionStrategy::HotPairs) return "hot_pairs";
        if (s == SelectionStrategy::FullSweep) return "full_sweep";
        return "random_sample";
    }

    static nlohmann::json toJson(const PairSelectionResult& r) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& p : r.selectedPairs) arr.push_back(p);
        return {{"strategy", strategyName(r.strategy)}, {"pairs", arr}, {"rationale", r.rationale}};
    }
};
