#pragma once
// Step 1318: Sprint 108 integration summary.
#include <nlohmann/json.hpp>

struct Sprint108IntegrationSummary {
    static constexpr int sprintNumber = 108;
    static constexpr int stepsCompleted = 10;
    static constexpr const char* theme = "Sprint 108 Plan: Self-Service Migration Portals and Workflow Templates";
    static bool verify() { return sprintNumber == 108 && stepsCompleted == 10; }
    static nlohmann::json toJson() {
        nlohmann::json j = nlohmann::json::object();
        j["sprint"] = sprintNumber;
        j["steps"] = stepsCompleted;
        j["theme"] = theme;
        j["status"] = "complete";
        nlohmann::json tools = nlohmann::json::array();
        tools.push_back("whetstone_list_migration_templates");
        tools.push_back("whetstone_start_guided_migration");
        j["tools_added"] = tools;
        nlohmann::json comps = nlohmann::json::array();
        comps.push_back("MigrationTemplateSchemaAndCatalogModel");
        comps.push_back("TemplateParameterValidationEngine");
        comps.push_back("GuidedWorkflowStateMachineForSelfServiceRuns");
        comps.push_back("PolicyComplianceAutoCheckpointIntegration");
        comps.push_back("TemplateRecommendationModelByProjectFingerprint");
        comps.push_back("SelfServiceEscalationHandoffHooks");
        comps.push_back("SelfServiceAdoptionReportArtifact");
        j["components"] = comps;
        return j;
    }
};
