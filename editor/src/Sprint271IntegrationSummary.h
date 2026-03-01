#pragma once
#include <string>

namespace whetstone {

struct Sprint271IntegrationSummary {
    int stepsCompleted = 5;      // 1883-1887
    bool featureExtractorActive = true;
    bool idiomProfilesActive = true;
    bool fitnessScorerActive = true;
    bool mcpToolActive = true;   // whetstone_score_language_fitness
    int languageProfileCount = 8;
    bool success = true;

    std::string sprintName() const { return "Sprint 271: LanguageFitnessScorer"; }
    std::string phase() const { return "Phase 1 - Language Fitness Scorer"; }
};

} // namespace whetstone
