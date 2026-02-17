// Step 523: Sprint 26 Integration + Summary (8 tests)

#include "Sprint26IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Sprint26Summary runNow() {
    return Sprint26IntegrationSummary::build();
}

void test_phase26a_gate_passes() {
    TEST(phase26a_gate_passes);
    auto s = runNow();
    CHECK(s.phase26aPass, "phase 26a should pass");
    PASS();
}

void test_visual_system_gates_pass() {
    TEST(visual_system_gates_pass);
    auto s = runNow();
    CHECK(s.tokenSystemPass, "token system should pass");
    CHECK(s.visualIdentityPass, "visual identity should pass");
    CHECK(s.controlsPass, "control restyle should pass");
    CHECK(s.typographyPass, "typography harmonization should pass");
    PASS();
}

void test_modifier_edge_system_passes() {
    TEST(modifier_edge_system_passes);
    auto s = runNow();
    CHECK(s.modifierEdgePass, "modifier-edge system should pass");
    PASS();
}

void test_overlay_hardening_passes() {
    TEST(overlay_hardening_passes);
    auto s = runNow();
    CHECK(s.overlayPass, "overlay hardening should pass");
    PASS();
}

void test_notification_refresh_passes() {
    TEST(notification_refresh_passes);
    auto s = runNow();
    CHECK(s.notificationPass, "notification refresh should pass");
    PASS();
}

void test_panel_consistency_passes() {
    TEST(panel_consistency_passes);
    auto s = runNow();
    CHECK(s.panelConsistencyPass, "panel consistency should pass");
    PASS();
}

void test_sprint_pass_flag_true() {
    TEST(sprint_pass_flag_true);
    auto s = runNow();
    CHECK(s.sprintPass, "sprint pass flag should be true");
    PASS();
}

void test_notes_include_completion_statement() {
    TEST(notes_include_completion_statement);
    auto s = runNow();
    CHECK(s.notes.size() >= 2, "expected summary notes");
    CHECK(s.notes[1].find("Sprint 26") != std::string::npos,
          "expected Sprint 26 completion statement");
    PASS();
}

int main() {
    std::cout << "Step 523: Sprint 26 Integration + Summary\n";

    test_phase26a_gate_passes();                   // 1
    test_visual_system_gates_pass();               // 2
    test_modifier_edge_system_passes();            // 3
    test_overlay_hardening_passes();               // 4
    test_notification_refresh_passes();            // 5
    test_panel_consistency_passes();               // 6
    test_sprint_pass_flag_true();                  // 7
    test_notes_include_completion_statement();     // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
