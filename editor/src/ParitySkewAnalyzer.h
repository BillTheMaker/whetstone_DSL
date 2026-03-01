#pragma once
// Step 1876: Parity skew analyzer — detect cross-language readiness asymmetry.
// Closes GR-013: CrossLanguageConsistencyGate evaluates per-request but has no
// batch analysis or skew tracking. This adds both.

#include "CrossLanguageConsistencyGate.h"

#include <string>
#include <vector>

struct LanguageReadiness {
    std::string language;
    bool ready; // supported && allowed for the requested construct/operation
};

struct ParitySkewResult {
    int readyCount = 0;
    int totalCount = 0;
    float skew = 0.0f;              // 0 = all ready, 1 = none ready
    std::vector<std::string> notReadyLanguages;
    bool belowThreshold = false;    // true when skew > skewThreshold
};

class ParitySkewAnalyzer {
public:
    // Analyze a pre-built readiness list.
    static ParitySkewResult analyze(const std::vector<LanguageReadiness>& languages,
                                    float skewThreshold = 0.5f) {
        ParitySkewResult result;
        result.totalCount = static_cast<int>(languages.size());
        for (const auto& lang : languages) {
            if (lang.ready) {
                ++result.readyCount;
            } else {
                result.notReadyLanguages.push_back(lang.language);
            }
        }
        result.skew = (result.totalCount == 0)
            ? 0.0f
            : 1.0f - static_cast<float>(result.readyCount) / result.totalCount;
        result.belowThreshold = result.skew > skewThreshold;
        return result;
    }

    // Batch-evaluate a construct+operation across a set of languages via the gate.
    static ParitySkewResult evaluateConstruct(const std::vector<std::string>& languages,
                                              const std::string& constructKind,
                                              const std::string& operation,
                                              float skewThreshold = 0.5f) {
        std::vector<LanguageReadiness> readiness;
        for (const auto& lang : languages) {
            CrossLanguageRequest req;
            req.language = lang;
            req.constructKind = constructKind;
            req.operation = operation;
            auto r = CrossLanguageConsistencyGate::evaluate(req);
            readiness.push_back({lang, r.supported && r.allowed});
        }
        return analyze(readiness, skewThreshold);
    }
};
