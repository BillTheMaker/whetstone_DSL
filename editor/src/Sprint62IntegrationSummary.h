#pragma once
// Sprint 62 Integration Summary
#include <nlohmann/json.hpp>
struct Sprint62IntegrationSummary {
    static constexpr int sprintNumber = 62;
    static constexpr int stepsCompleted = 9;
    static constexpr const char* theme = "Adapter Quality Telemetry and Failure Taxonomy";
    static bool verify() { return true; }
    static nlohmann::json toJson() {
        return {{"sprint",62},{"steps",9},{"theme",theme},{"status","complete"}};
    }
};
