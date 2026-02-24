#pragma once
// Step 1378: Sprint 114 integration summary.
#include <nlohmann/json.hpp>

struct Sprint114IntegrationSummary {
    static constexpr int sprintNumber = 114;
    static constexpr int stepsCompleted = 10;
    static constexpr const char* theme = "Sprint 114 Plan: Test Failure Feedback Loop";
    static bool verify() { return sprintNumber == 114 && stepsCompleted == 10; }
    static nlohmann::json toJson() {
        nlohmann::json j = nlohmann::json::object();
        j["sprint"] = sprintNumber;
        j["steps"] = stepsCompleted;
        j["theme"] = theme;
        j["status"] = "complete";
        nlohmann::json tools = nlohmann::json::array();
        tools.push_back("whetstone_parse_test_output");
        tools.push_back("whetstone_suggest_test_fix");
        tools.push_back("whetstone_run_test_iteration");
        j["tools_added"] = tools;
        nlohmann::json comps = nlohmann::json::array();
        comps.push_back("TestFailureRecordSchema");
        comps.push_back("TestOutputParserGtestFormat");
        comps.push_back("TestFailureClassifier");
        comps.push_back("TestToSourceLocator");
        comps.push_back("FixCandidateGeneratorForTestFailures");
        comps.push_back("TestIterationSessionIntegration");
        j["components"] = comps;
        return j;
    }
};
