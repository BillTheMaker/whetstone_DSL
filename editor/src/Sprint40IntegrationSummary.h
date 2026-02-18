#pragma once
// Step 663: Sprint 40 integration + final summary

#include "CrossSessionContextBridge.h"
#include "EnergyContextStatusModel.h"
#include "EntropyScannerModel.h"
#include "HiveMindJobPublisher.h"
#include "InferenceJobGenerator.h"
#include "Phase40aIntegration.h"
#include "PilotQueuePanelModel.h"

struct Sprint40IntegrationResult {
    bool phase40aReady = false;
    bool entropyDetected = false;
    bool inferenceJobGenerated = false;
    bool pilotQueueReady = false;
    bool bridgeReady = false;
    bool finalSystemReady = false;
};

class Sprint40IntegrationSummary {
public:
    static Sprint40IntegrationResult run() {
        Sprint40IntegrationResult out;
        auto phase40a = Phase40aIntegration::run();
        out.phase40aReady = phase40a.integrationReady;

        auto entropy = EntropyScannerModel::scan({"a()","a()"}, {"modA","modB"}, {"x"}, {});
        out.entropyDetected = EntropyScannerModel::shouldSuggestRefactor(entropy, 2);

        auto inference = InferenceJobGenerator::generate("extract duplicates", EntropyScannerModel::entropyScore(entropy), {"a.cpp"});
        out.inferenceJobGenerated = inference.value("success", false);

        PilotQueueItem item{"job-1", "review", true, 0.2};
        out.pilotQueueReady = PilotQueuePanelModel::needsHumanReview(item);

        CrossSessionBridgeRequest req{"job-1", {"a.cpp"}, "/workspace"};
        auto bridge = CrossSessionContextBridge::run(req);
        out.bridgeReady = bridge.launchedMcp && bridge.intakeCalled && bridge.fedBackToEditor;

        out.finalSystemReady = out.phase40aReady && out.entropyDetected && out.inferenceJobGenerated &&
                               out.pilotQueueReady && out.bridgeReady;
        return out;
    }
};
