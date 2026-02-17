#pragma once
// Step 587: Guided Architect-to-Execution Demo Mode

#include "IntakeToQueueSimulationHarness.h"

#include <string>
#include <vector>

enum class DemoStage {
    NotStarted,
    Intake,
    Review,
    Queue,
    ConstrainedEdits,
    Verification,
    Complete
};

struct DemoModeState {
    DemoStage stage = DemoStage::NotStarted;
    bool active = false;
    IntakeQueueSimulationResult simulation;
    std::vector<std::string> timeline;
};

class GuidedArchitectExecutionDemoMode {
public:
    static bool start(const IntakeQueueSimulationInput& input,
                      DemoModeState* state,
                      std::string* error) {
        if (!state || !error) return false;
        error->clear();
        state->simulation = IntakeToQueueSimulationHarness::run(input);
        state->stage = DemoStage::Intake;
        state->active = true;
        state->timeline.clear();
        state->timeline.push_back("stage:intake");
        if (!state->simulation.ok) return fail(error, "simulation_failed");
        return true;
    }

    static bool advance(DemoModeState* state, std::string* error) {
        if (!state || !error) return false;
        error->clear();
        if (!state->active) return fail(error, "demo_inactive");

        if (state->stage == DemoStage::Intake) {
            state->stage = DemoStage::Review;
            state->timeline.push_back("stage:review");
            return true;
        }
        if (state->stage == DemoStage::Review) {
            state->stage = DemoStage::Queue;
            state->timeline.push_back("stage:queue");
            return true;
        }
        if (state->stage == DemoStage::Queue) {
            if (state->simulation.queuedCount == 0) return fail(error, "queue_empty");
            state->stage = DemoStage::ConstrainedEdits;
            state->timeline.push_back("stage:constrained_edits");
            return true;
        }
        if (state->stage == DemoStage::ConstrainedEdits) {
            state->stage = DemoStage::Verification;
            state->timeline.push_back("stage:verification");
            return true;
        }
        if (state->stage == DemoStage::Verification) {
            state->stage = DemoStage::Complete;
            state->active = false;
            state->timeline.push_back("stage:complete");
            return true;
        }
        return fail(error, "demo_already_complete");
    }

    static float completionFraction(const DemoModeState& state) {
        switch (state.stage) {
            case DemoStage::NotStarted: return 0.0f;
            case DemoStage::Intake: return 0.2f;
            case DemoStage::Review: return 0.4f;
            case DemoStage::Queue: return 0.6f;
            case DemoStage::ConstrainedEdits: return 0.8f;
            case DemoStage::Verification: return 0.9f;
            case DemoStage::Complete: return 1.0f;
        }
        return 0.0f;
    }

private:
    static bool fail(std::string* error, const char* code) {
        *error = code;
        return false;
    }
};
