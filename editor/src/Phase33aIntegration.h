#pragma once
// Step 588: Phase 33a Integration

#include "CapabilityDiscoveryPanels.h"
#include "GuidedArchitectExecutionDemoMode.h"
#include "ValueForwardOnboardingFlow.h"
#include "WorkflowVisualizationV2.h"

#include <string>
#include <vector>

struct Phase33aResult {
    bool onboardingReady = false;
    bool visualizationReady = false;
    bool capabilityPanelReady = false;
    bool demoModeReady = false;
    bool phase33aPass = false;
    std::vector<std::string> notes;
};

class Phase33aIntegration {
public:
    static Phase33aResult run() {
        Phase33aResult result;
        std::string error;

        OnboardingFlowState onboarding;
        if (ValueForwardOnboardingFlow::start("default-profile", &onboarding, &error)) {
            ValueForwardOnboardingFlow::completeCurrent(&onboarding, &error);
            result.onboardingReady = !ValueForwardOnboardingFlow::valueHighlights(onboarding).empty();
        } else {
            result.notes.push_back("fail:onboarding");
            return finalize(result);
        }

        WorkflowVisualizationGraph graph;
        result.visualizationReady =
            WorkflowVisualizationV2::addNode(&graph, {"intake", WorkflowNodeKind::Intake, "Intake", "#2F7BFF"}, &error) &&
            WorkflowVisualizationV2::addNode(&graph, {"review", WorkflowNodeKind::ReviewGate, "Review", "#2F7BFF"}, &error) &&
            WorkflowVisualizationV2::addEdge(&graph, {"edge-1", "intake", "review", "Ambiguity triggers review gate"}, &error) &&
            !WorkflowVisualizationV2::plainLanguageRoutingSummary(graph).empty();
        if (!result.visualizationReady) {
            result.notes.push_back("fail:visualization");
            return finalize(result);
        }

        CapabilityDiscoveryPanels panels;
        result.capabilityPanelReady =
            panels.registerCategory("mcp", "MCP Tools", &error) &&
            panels.recordOperation("mcp", &error) &&
            panels.addRecommendation("mcp", "Try constrained execution demo", &error) &&
            panels.operationCountFor("mcp") == 1;
        if (!result.capabilityPanelReady) {
            result.notes.push_back("fail:capability_panels");
            return finalize(result);
        }

        DemoModeState demo;
        IntakeQueueSimulationInput input;
        input.markdown = R"(## Goals
- Show end-to-end value
## Constraints
- Enable deterministic parser behavior
## Dependencies
- CMake
## Acceptance Criteria
- Queue includes bound acceptance checks
)";
        if (!GuidedArchitectExecutionDemoMode::start(input, &demo, &error)) {
            result.notes.push_back("fail:demo_start");
            return finalize(result);
        }
        result.demoModeReady = GuidedArchitectExecutionDemoMode::advance(&demo, &error) &&
                               GuidedArchitectExecutionDemoMode::advance(&demo, &error) &&
                               GuidedArchitectExecutionDemoMode::advance(&demo, &error);
        if (!result.demoModeReady) {
            result.notes.push_back("fail:demo_progression");
            return finalize(result);
        }

        result.notes.push_back("phase33a:product_value_obvious_on_first_use");
        return finalize(result);
    }

private:
    static Phase33aResult finalize(Phase33aResult result) {
        result.phase33aPass = result.onboardingReady &&
                             result.visualizationReady &&
                             result.capabilityPanelReady &&
                             result.demoModeReady;
        if (!result.phase33aPass) result.notes.push_back("phase33a:blocked");
        return result;
    }
};
