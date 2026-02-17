// Step 587: Guided Architect-to-Execution Demo Mode (12 tests)

#include "GuidedArchitectExecutionDemoMode.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static IntakeQueueSimulationInput validInput() {
    IntakeQueueSimulationInput input;
    input.markdown = R"(## Goals
- Improve intake quality
## Constraints
- Enable deterministic parser behavior
## Dependencies
- CMake
## Acceptance Criteria
- Queue includes bound acceptance checks
)";
    return input;
}

void test_start_success() {
    TEST(start_success);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start should succeed");
    CHECK(state.active, "demo should be active");
    CHECK(state.stage == DemoStage::Intake, "initial stage mismatch");
    PASS();
}

void test_start_fails_when_simulation_fails() {
    TEST(start_fails_when_simulation_fails);
    DemoModeState state;
    std::string error;
    auto bad = validInput();
    bad.markdown = "";
    CHECK(!GuidedArchitectExecutionDemoMode::start(bad, &state, &error), "start should fail");
    CHECK(error == "simulation_failed", "wrong error");
    PASS();
}

void test_advance_intake_to_review() {
    TEST(advance_intake_to_review);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance failed");
    CHECK(state.stage == DemoStage::Review, "stage should be review");
    PASS();
}

void test_advance_review_to_queue() {
    TEST(advance_review_to_queue);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 1 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 2 failed");
    CHECK(state.stage == DemoStage::Queue, "stage should be queue");
    PASS();
}

void test_queue_stage_requires_non_empty_queue() {
    TEST(queue_stage_requires_non_empty_queue);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    state.simulation.queuedCount = 0;
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance to review failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance to queue failed");
    CHECK(!GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance to constrained edits should fail");
    CHECK(error == "queue_empty", "wrong error");
    PASS();
}

void test_advance_queue_to_constrained_edits() {
    TEST(advance_queue_to_constrained_edits);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 1 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 2 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 3 failed");
    CHECK(state.stage == DemoStage::ConstrainedEdits, "stage should be constrained edits");
    PASS();
}

void test_advance_to_verification() {
    TEST(advance_to_verification);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 1 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 2 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 3 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 4 failed");
    CHECK(state.stage == DemoStage::Verification, "stage should be verification");
    PASS();
}

void test_advance_verification_to_complete_deactivates() {
    TEST(advance_verification_to_complete_deactivates);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 1 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 2 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 3 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 4 failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance 5 failed");
    CHECK(state.stage == DemoStage::Complete, "stage should be complete");
    CHECK(!state.active, "demo should deactivate");
    PASS();
}

void test_advance_fails_when_inactive() {
    TEST(advance_fails_when_inactive);
    DemoModeState state;
    std::string error;
    CHECK(!GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance should fail");
    CHECK(error == "demo_inactive", "wrong error");
    PASS();
}

void test_completion_fraction_increases_by_stage() {
    TEST(completion_fraction_increases_by_stage);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    const float intake = GuidedArchitectExecutionDemoMode::completionFraction(state);
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance failed");
    const float review = GuidedArchitectExecutionDemoMode::completionFraction(state);
    CHECK(review > intake, "completion should increase");
    PASS();
}

void test_timeline_records_stage_transitions() {
    TEST(timeline_records_stage_transitions);
    DemoModeState state;
    std::string error;
    CHECK(GuidedArchitectExecutionDemoMode::start(validInput(), &state, &error), "start failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance failed");
    CHECK(GuidedArchitectExecutionDemoMode::advance(&state, &error), "advance failed");
    CHECK(state.timeline.size() == 3, "timeline size mismatch");
    CHECK(state.timeline[0] == "stage:intake", "timeline intake missing");
    CHECK(state.timeline[2] == "stage:queue", "timeline queue missing");
    PASS();
}

void test_complete_stage_returns_full_fraction() {
    TEST(complete_stage_returns_full_fraction);
    DemoModeState state;
    state.stage = DemoStage::Complete;
    CHECK(GuidedArchitectExecutionDemoMode::completionFraction(state) == 1.0f, "complete fraction should be 1.0");
    PASS();
}

int main() {
    std::cout << "Step 587: Guided Architect-to-Execution Demo Mode\n";

    test_start_success();                               // 1
    test_start_fails_when_simulation_fails();           // 2
    test_advance_intake_to_review();                    // 3
    test_advance_review_to_queue();                     // 4
    test_queue_stage_requires_non_empty_queue();        // 5
    test_advance_queue_to_constrained_edits();          // 6
    test_advance_to_verification();                     // 7
    test_advance_verification_to_complete_deactivates();// 8
    test_advance_fails_when_inactive();                 // 9
    test_completion_fraction_increases_by_stage();      // 10
    test_timeline_records_stage_transitions();          // 11
    test_complete_stage_returns_full_fraction();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
