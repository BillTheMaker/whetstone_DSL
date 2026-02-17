// Step 588: Phase 33a Integration (8 tests)

#include "Phase33aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Phase33aResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

void test_full_phase33a_cycle_passes() {
    TEST(full_phase33a_cycle_passes);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(r.onboardingReady, "onboarding should be ready");
    CHECK(r.visualizationReady, "visualization should be ready");
    CHECK(r.capabilityPanelReady, "capability panel should be ready");
    CHECK(r.demoModeReady, "demo mode should be ready");
    CHECK(hasNote(r, "phase33a:product_value_obvious_on_first_use"), "success note missing");
    PASS();
}

void test_onboarding_ready_signal_true_on_pass() {
    TEST(onboarding_ready_signal_true_on_pass);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(r.onboardingReady, "onboarding should be true");
    PASS();
}

void test_visualization_ready_signal_true_on_pass() {
    TEST(visualization_ready_signal_true_on_pass);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(r.visualizationReady, "visualization should be true");
    PASS();
}

void test_capability_panel_ready_signal_true_on_pass() {
    TEST(capability_panel_ready_signal_true_on_pass);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(r.capabilityPanelReady, "capability panel should be true");
    PASS();
}

void test_demo_mode_ready_signal_true_on_pass() {
    TEST(demo_mode_ready_signal_true_on_pass);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(r.demoModeReady, "demo mode should be true");
    PASS();
}

void test_no_blocked_note_on_success() {
    TEST(no_blocked_note_on_success);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(!hasNote(r, "phase33a:blocked"), "blocked note should be absent");
    PASS();
}

void test_notes_include_only_success_path_marker() {
    TEST(notes_include_only_success_path_marker);
    auto r = Phase33aIntegration::run();
    CHECK(r.phase33aPass, "phase should pass");
    CHECK(r.notes.size() == 1, "exactly one success note expected");
    PASS();
}

void test_phase_flag_matches_component_conjunction() {
    TEST(phase_flag_matches_component_conjunction);
    auto r = Phase33aIntegration::run();
    const bool conjunction = r.onboardingReady && r.visualizationReady &&
                             r.capabilityPanelReady && r.demoModeReady;
    CHECK(r.phase33aPass == conjunction, "phase flag should equal component conjunction");
    PASS();
}

int main() {
    std::cout << "Step 588: Phase 33a Integration\n";

    test_full_phase33a_cycle_passes();                 // 1
    test_onboarding_ready_signal_true_on_pass();       // 2
    test_visualization_ready_signal_true_on_pass();    // 3
    test_capability_panel_ready_signal_true_on_pass(); // 4
    test_demo_mode_ready_signal_true_on_pass();        // 5
    test_no_blocked_note_on_success();                 // 6
    test_notes_include_only_success_path_marker();     // 7
    test_phase_flag_matches_component_conjunction();   // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
