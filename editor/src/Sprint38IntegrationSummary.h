#pragma once
// Step 643: Sprint 38 integration + summary

#include "JobDispatchTableGenerator.h"
#include "MqttBoilerplateGenerator.h"
#include "ProjectSkeletonGenerator.h"
#include "SQLiteDataLayerGenerator.h"

#include <string>

struct Sprint38IntegrationResult {
    bool projectGenerated = false;
    bool mqttGenerated = false;
    bool sqliteGenerated = false;
    bool dispatchGenerated = false;
    bool cmakeReady = false;
    bool hasMainEntry = false;
    bool hasTopicHandlers = false;
    bool compileReady = false;
};

class Sprint38IntegrationSummary {
public:
    static Sprint38IntegrationResult run() {
        Sprint38IntegrationResult out;

        auto project = ProjectSkeletonGenerator::generate(
            "hivemind_drone",
            "HiveMind drone bootstrap",
            {"nlohmann_json", "paho-mqttpp3", "SQLite3"});
        out.projectGenerated = project.success;
        out.cmakeReady = project.cmakeLists.find("add_executable(hivemind_drone") != std::string::npos;
        out.hasMainEntry = project.mainCpp.find("int main") != std::string::npos;

        auto mqtt = MqttBoilerplateGenerator::generate("DroneMqttClient", {
            {"borg/energy/context", "EnergyContext", 1},
            {"borg/jobs/pending", "PendingJob", 1},
            {"borg/registry/commit", "RegistryCommit", 2}
        });
        out.mqttGenerated = mqtt.success;
        out.hasTopicHandlers = mqtt.sourceCode.find("borg/jobs/pending") != std::string::npos;

        auto sqlite = SQLiteDataLayerGenerator::generate({
            {"jobs", {"id", "status", "payload"}},
            {"executions", {"id", "job_id", "started_at"}}
        });
        out.sqliteGenerated = sqlite.success;

        auto dispatch = JobDispatchTableGenerator::generate({
            {"compile", {"cpp", "build"}, "CompilePayload", "runCompile"},
            {"test", {"cpp", "test"}, "TestPayload", "runTests"}
        });
        out.dispatchGenerated = dispatch.success;

        out.compileReady = out.projectGenerated && out.mqttGenerated &&
                           out.sqliteGenerated && out.dispatchGenerated &&
                           out.cmakeReady && out.hasMainEntry && out.hasTopicHandlers;
        return out;
    }
};
