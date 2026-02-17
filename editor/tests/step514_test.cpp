// Step 514: Phase 26a Integration (8 tests)

#include "Phase26aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Phase26aIntegrationResult runNow() {
    return Phase26aIntegration::run();
}

void test_docking_stability_gate_passes() {
    TEST(docking_stability_gate_passes);
    auto r = runNow();
    CHECK(r.dockingStable, "docking gate should pass");
    PASS();
}

void test_hit_target_reliability_gate_passes() {
    TEST(hit_target_reliability_gate_passes);
    auto r = runNow();
    CHECK(r.hitTargetsReliable, "hit-target gate should pass");
    PASS();
}

void test_interaction_state_gate_passes() {
    TEST(interaction_state_gate_passes);
    auto r = runNow();
    CHECK(r.interactionStatesValid, "interaction-state gate should pass");
    PASS();
}

void test_hierarchy_gate_passes() {
    TEST(hierarchy_gate_passes);
    auto r = runNow();
    CHECK(r.hierarchyValid, "hierarchy gate should pass");
    PASS();
}

void test_accessibility_gate_passes() {
    TEST(accessibility_gate_passes);
    auto r = runNow();
    CHECK(r.accessibilityBaselineValid, "accessibility gate should pass");
    PASS();
}

void test_editor_flow_safe_signal_is_true() {
    TEST(editor_flow_safe_signal_is_true);
    auto r = runNow();
    CHECK(r.editorFlowSafe, "editor flow should be safe");
    PASS();
}

void test_phase_pass_flag_is_true() {
    TEST(phase_pass_flag_is_true);
    auto r = runNow();
    CHECK(r.pass, "phase integration should pass");
    PASS();
}

void test_notes_include_pass_marker() {
    TEST(notes_include_pass_marker);
    auto r = runNow();
    bool found = false;
    for (const auto& n : r.notes) {
        if (n == "phase-26a-integration-pass") found = true;
    }
    CHECK(found, "pass marker note missing");
    PASS();
}

int main() {
    std::cout << "Step 514: Phase 26a Integration\n";

    test_docking_stability_gate_passes();      // 1
    test_hit_target_reliability_gate_passes(); // 2
    test_interaction_state_gate_passes();      // 3
    test_hierarchy_gate_passes();              // 4
    test_accessibility_gate_passes();          // 5
    test_editor_flow_safe_signal_is_true();    // 6
    test_phase_pass_flag_is_true();            // 7
    test_notes_include_pass_marker();          // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
