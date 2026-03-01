#pragma once
#include "ASTFeatureExtractor.h"
#include "LanguageFitnessScorer.h"
#include <string>
#include <vector>

namespace whetstone {

struct PolyglotInterface {
    std::string fromComponent;
    std::string toComponent;
    std::string description;
};

struct PolyglotSection {
    std::string componentName;
    ASTFeatures features;           // feature vector for fitness scoring
    std::string explicitLanguage;   // empty = auto-route via scorer
    std::string assignedLanguage;   // filled by PolyglotFitnessRouter
};

struct PolyglotProjectSpec {
    std::string projectName;
    std::vector<PolyglotSection> sections;
    std::vector<PolyglotInterface> interfaces;
};

class PolyglotFitnessRouter {
public:
    // Route each unassigned section to the best-scoring language.
    // Explicit assignments (explicitLanguage non-empty) are preserved as-is.
    static void route(PolyglotProjectSpec& spec) {
        for (auto& section : spec.sections) {
            if (!section.explicitLanguage.empty()) {
                section.assignedLanguage = section.explicitLanguage;
            } else {
                auto ranked = LanguageFitnessScorer::score(section.features);
                if (!ranked.empty()) {
                    section.assignedLanguage = ranked[0]["language"].get<std::string>();
                }
            }
        }
    }
};

} // namespace whetstone
