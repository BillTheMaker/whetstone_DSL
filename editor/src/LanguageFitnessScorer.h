#pragma once
#include "ASTFeatureExtractor.h"
#include "LanguageIdiomProfile.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace whetstone {

struct FitnessResult {
    std::string language;
    float score;
    std::string rationale;
};

class LanguageFitnessScorer {
public:
    // Returns JSON array [{language, score, rationale}, ...] sorted descending
    static nlohmann::json score(const ASTFeatures& features) {
        auto profiles = LanguageIdiomProfile::allProfiles();
        std::vector<FitnessResult> results;
        results.reserve(profiles.size());
        for (const auto& prof : profiles) {
            results.push_back(scoreOne(features, prof));
        }
        std::sort(results.begin(), results.end(),
                  [](const FitnessResult& a, const FitnessResult& b){
                      return a.score > b.score;
                  });
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& r : results) {
            arr.push_back({{"language", r.language},
                           {"score",    r.score},
                           {"rationale", r.rationale}});
        }
        return arr;
    }

private:
    static FitnessResult scoreOne(const ASTFeatures& f, const LanguageIdiomProfile& p) {
        float s = 100.0f;
        std::ostringstream rat;

        float mutDelta = std::abs(f.mutationRatio - p.idealMutationRatio) * 30.0f;
        s -= mutDelta;
        if (mutDelta > 0.5f) rat << "mutation-" << static_cast<int>(mutDelta) << " ";

        if (f.recursionShape != p.idealRecursion) {
            s -= 20.0f;
            rat << "recursion-mismatch ";
        }
        if (f.concurrencyPrimitive != p.idealConcurrency) {
            s -= 25.0f;
            rat << "concurrency-mismatch ";
        }
        if (f.ioPattern != p.idealIO) {
            s -= 20.0f;
            rat << "io-mismatch ";
        }
        if (f.typeComplexity != p.idealTypeComplexity) {
            s -= 15.0f;
            rat << "type-mismatch ";
        }

        s = std::max(0.0f, std::min(100.0f, s));

        std::string ratStr = rat.str();
        if (ratStr.empty()) ratStr = "ideal match";
        if (!ratStr.empty() && ratStr.back() == ' ') ratStr.pop_back();

        return {p.language, s, ratStr};
    }
};

} // namespace whetstone
