#pragma once
// Step 648: Phase 39a integration

#include "InEditorTestRunnerModel.h"
#include "ProjectAstCMakeGenerator.h"
#include "SelfHostingProjectConfig.h"
#include "SelfModificationSafetyGuard.h"

struct Phase39aIntegrationResult {
    bool configLoaded = false;
    bool cmakeGenerated = false;
    bool testRunTriggered = false;
    bool testRunComplete = false;
    bool mutationBlocked = false;
    bool safetyReasonPresent = false;
    bool selfHostingReady = false;
    bool integrationReady = false;
};

class Phase39aIntegration {
public:
    static Phase39aIntegrationResult run() {
        Phase39aIntegrationResult out;

        SelfHostingProjectConfig cfg{"whetstone_DSL", "/workspace", {"editor/src"}, {"step648_test"}, true};
        auto j = SelfHostingProjectConfigModel::toJson(cfg);
        SelfHostingProjectConfig parsed;
        std::string err;
        out.configLoaded = SelfHostingProjectConfigModel::fromJson(j, &parsed, &err);
        out.selfHostingReady = SelfHostingProjectConfigModel::canSelfHost(parsed);

        ProjectAstDescription ast{"whetstone_editor", {"src/main.cpp"}, {"step648_test"}, {"nlohmann_json"}, true};
        std::string cmake = ProjectAstCMakeGenerator::generate(ast);
        out.cmakeGenerated = cmake.find("add_executable(whetstone_editor") != std::string::npos;

        auto testSummary = InEditorTestRunnerModel::run({"step648_test"});
        out.testRunTriggered = !testSummary.items.empty();
        out.testRunComplete = testSummary.complete;

        std::string reason;
        out.mutationBlocked = !SelfModificationSafetyGuard::allowMutation("editor/src/main.cpp", {"editor/src"}, &reason);
        out.safetyReasonPresent = !reason.empty();

        out.integrationReady = out.configLoaded && out.cmakeGenerated && out.testRunTriggered &&
                               out.testRunComplete && out.mutationBlocked && out.safetyReasonPresent &&
                               out.selfHostingReady;
        return out;
    }
};
