// Step 573: Sprint 31 Integration + Summary (8 tests)

#include "Sprint31IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint31IntegrationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static Sprint31Signals allPass() {
    Sprint31Signals s;
    s.memorySnapshotModel = true;
    s.memoryInspectorUiModel = true;
    s.allocationOwnershipTraceHooks = true;
    s.leakCorruptionSignalBridge = true;
    s.phase31aIntegration = true;
    s.disassemblyRegisterView = true;
    s.watchEvaluatorHardening = true;
    s.timeTravelEventBuffer = true;
    s.performanceProbeOverlay = true;
    return s;
}

void test_all_signals_pass_sprint() {
    TEST(all_signals_pass_sprint);
    auto r = Sprint31IntegrationSummary::summarize(allPass());
    CHECK(r.phase31aPass, "phase31a should pass");
    CHECK(r.phase31bPass, "phase31b should pass");
    CHECK(r.sprint31Pass, "sprint31 should pass");
    PASS();
}

void test_phase31a_failure_blocks_sprint() {
    TEST(phase31a_failure_blocks_sprint);
    auto s = allPass();
    s.phase31aIntegration = false;
    auto r = Sprint31IntegrationSummary::summarize(s);
    CHECK(!r.phase31aPass, "phase31a should fail");
    CHECK(!r.sprint31Pass, "sprint31 should fail");
    CHECK(hasNote(r, "fail:step568_phase31a_integration"), "step568 failure note expected");
    PASS();
}

void test_phase31b_failure_blocks_sprint() {
    TEST(phase31b_failure_blocks_sprint);
    auto s = allPass();
    s.watchEvaluatorHardening = false;
    auto r = Sprint31IntegrationSummary::summarize(s);
    CHECK(!r.phase31bPass, "phase31b should fail");
    CHECK(!r.sprint31Pass, "sprint31 should fail");
    CHECK(hasNote(r, "fail:step570_watch_evaluator_hardening"), "step570 failure note expected");
    PASS();
}

void test_success_notes_emitted_on_full_pass() {
    TEST(success_notes_emitted_on_full_pass);
    auto r = Sprint31IntegrationSummary::summarize(allPass());
    CHECK(hasNote(r, "sprint31:memory_reflection_workflow_ready"), "success note missing");
    CHECK(hasNote(r, "sprint31:advanced_debug_views_stable"), "success note missing");
    CHECK(hasNote(r, "sprint31:observability_surfaces_integrated"), "success note missing");
    PASS();
}

void test_blocked_phase_notes_emitted_on_failure() {
    TEST(blocked_phase_notes_emitted_on_failure);
    auto s = allPass();
    s.memorySnapshotModel = false;
    s.disassemblyRegisterView = false;
    auto r = Sprint31IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "phase31a:blocked"), "phase31a blocked note expected");
    CHECK(hasNote(r, "phase31b:blocked"), "phase31b blocked note expected");
    PASS();
}

void test_multiple_failure_notes_aggregate() {
    TEST(multiple_failure_notes_aggregate);
    auto s = allPass();
    s.memoryInspectorUiModel = false;
    s.leakCorruptionSignalBridge = false;
    s.performanceProbeOverlay = false;
    auto r = Sprint31IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "fail:step565_memory_inspector_ui_model"), "step565 failure note missing");
    CHECK(hasNote(r, "fail:step567_leak_corruption_signal_bridge"), "step567 failure note missing");
    CHECK(hasNote(r, "fail:step572_performance_probe_overlay"), "step572 failure note missing");
    PASS();
}

void test_all_false_signals_fail_all_phases() {
    TEST(all_false_signals_fail_all_phases);
    Sprint31Signals s;
    auto r = Sprint31IntegrationSummary::summarize(s);
    CHECK(!r.phase31aPass && !r.phase31bPass && !r.sprint31Pass, "all should fail");
    PASS();
}

void test_single_phase31b_failure_keeps_phase31a_passed() {
    TEST(single_phase31b_failure_keeps_phase31a_passed);
    auto s = allPass();
    s.timeTravelEventBuffer = false;
    auto r = Sprint31IntegrationSummary::summarize(s);
    CHECK(r.phase31aPass, "phase31a should remain pass");
    CHECK(!r.phase31bPass, "phase31b should fail");
    PASS();
}

int main() {
    std::cout << "Step 573: Sprint 31 Integration + Summary\n";

    test_all_signals_pass_sprint();                      // 1
    test_phase31a_failure_blocks_sprint();               // 2
    test_phase31b_failure_blocks_sprint();               // 3
    test_success_notes_emitted_on_full_pass();           // 4
    test_blocked_phase_notes_emitted_on_failure();       // 5
    test_multiple_failure_notes_aggregate();             // 6
    test_all_false_signals_fail_all_phases();            // 7
    test_single_phase31b_failure_keeps_phase31a_passed();// 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
