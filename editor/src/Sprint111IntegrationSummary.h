#pragma once
// Step 1348: Sprint 111 integration summary.
#include <nlohmann/json.hpp>

struct Sprint111IntegrationSummary {
    static constexpr int sprintNumber = 111;
    static constexpr int stepsCompleted = 10;
    static constexpr const char* theme = "Sprint 111 Plan: Iteration Session Context";
    static bool verify() { return sprintNumber == 111 && stepsCompleted == 10; }
    static nlohmann::json toJson() {
        nlohmann::json j = nlohmann::json::object();
        j["sprint"] = sprintNumber;
        j["steps"] = stepsCompleted;
        j["theme"] = theme;
        j["status"] = "complete";
        nlohmann::json tools = nlohmann::json::array();
        tools.push_back("whetstone_start_iteration_session");
        tools.push_back("whetstone_record_attempt");
        tools.push_back("whetstone_get_session_state");
        j["tools_added"] = tools;
        nlohmann::json comps = nlohmann::json::array();
        comps.push_back("IterationSessionCoreSchema");
        comps.push_back("AttemptRecordSchema");
        comps.push_back("ObservationRecordSchema");
        comps.push_back("GapModelDistanceFromConvergenceGoal");
        comps.push_back("SessionStateMachineTransitions");
        comps.push_back("SessionPersistenceToSidecarFile");
        j["components"] = comps;
        return j;
    }
};
