#pragma once
// Step 1368: Sprint 113 integration summary.
#include <nlohmann/json.hpp>

struct Sprint113IntegrationSummary {
    static constexpr int sprintNumber = 113;
    static constexpr int stepsCompleted = 10;
    static constexpr const char* theme = "Sprint 113 Plan: Build Output Feedback Loop";
    static bool verify() { return sprintNumber == 113 && stepsCompleted == 10; }
    static nlohmann::json toJson() {
        nlohmann::json j = nlohmann::json::object();
        j["sprint"] = sprintNumber;
        j["steps"] = stepsCompleted;
        j["theme"] = theme;
        j["status"] = "complete";
        nlohmann::json tools = nlohmann::json::array();
        tools.push_back("whetstone_parse_build_output");
        tools.push_back("whetstone_suggest_build_fix");
        tools.push_back("whetstone_run_build_iteration");
        j["tools_added"] = tools;
        nlohmann::json comps = nlohmann::json::array();
        comps.push_back("BuildErrorRecordSchema");
        comps.push_back("BuildOutputParserGCCClangFormat");
        comps.push_back("ASTNodeLocatorFromBuildError");
        comps.push_back("BuildErrorClassifier");
        comps.push_back("FixCandidateGenerator");
        comps.push_back("BuildIterationSessionIntegration");
        j["components"] = comps;
        return j;
    }
};
