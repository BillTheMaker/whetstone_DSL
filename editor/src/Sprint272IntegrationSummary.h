#pragma once
#include <string>

namespace whetstone {

struct Sprint272IntegrationSummary {
    int stepsCompleted = 5;      // 1888-1892
    bool polyglotSpecActive = true;
    bool polySortActive = true;
    bool polyApiActive = true;
    bool polyParseActive = true;
    int testProjectCount = 3;
    bool success = true;

    std::string sprintName() const { return "Sprint 272: Fitness-Routed Polyglot Test Projects"; }
    std::string phase() const { return "Phase 2 - Polyglot Test Projects"; }
};

} // namespace whetstone
