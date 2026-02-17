// Step 563: Sprint 30 Integration + Summary (8 tests)

#include "Sprint30IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint30IntegrationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static Sprint30Signals allPass() {
    Sprint30Signals s;
    s.debugSessionModel = true;
    s.breakpointSystem = true;
    s.stepEngine = true;
    s.exceptionStackCapture = true;
    s.phase30aIntegration = true;
    s.callStackInspector = true;
    s.localsWatchesModel = true;
    s.traceTimelineModel = true;
    s.workflowAwareDebugHooks = true;
    return s;
}

void test_all_signals_pass_sprint() {
    TEST(all_signals_pass_sprint);
    auto r = Sprint30IntegrationSummary::summarize(allPass());
    CHECK(r.phase30aPass, "phase30a should pass");
    CHECK(r.phase30bPass, "phase30b should pass");
    CHECK(r.sprint30Pass, "sprint should pass");
    PASS();
}

void test_phase30a_failure_blocks_sprint() {
    TEST(phase30a_failure_blocks_sprint);
    auto s = allPass();
    s.stepEngine = false;
    auto r = Sprint30IntegrationSummary::summarize(s);
    CHECK(!r.phase30aPass, "phase30a should fail");
    CHECK(!r.sprint30Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step556_step_engine"), "step556 failure note expected");
    PASS();
}

void test_phase30b_failure_blocks_sprint() {
    TEST(phase30b_failure_blocks_sprint);
    auto s = allPass();
    s.traceTimelineModel = false;
    auto r = Sprint30IntegrationSummary::summarize(s);
    CHECK(!r.phase30bPass, "phase30b should fail");
    CHECK(!r.sprint30Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step561_trace_timeline_model"), "step561 failure note expected");
    PASS();
}

void test_success_notes_emitted_on_full_pass() {
    TEST(success_notes_emitted_on_full_pass);
    auto r = Sprint30IntegrationSummary::summarize(allPass());
    CHECK(hasNote(r, "sprint30:debugger_baseline_daily_use_ready"), "success note missing");
    CHECK(hasNote(r, "sprint30:breakpoint_step_exception_cycle_stable"), "success note missing");
    CHECK(hasNote(r, "sprint30:runtime_inspection_surfaces_connected"), "success note missing");
    PASS();
}

void test_blocked_phase_notes_emitted_on_failure() {
    TEST(blocked_phase_notes_emitted_on_failure);
    auto s = allPass();
    s.breakpointSystem = false;
    s.localsWatchesModel = false;
    auto r = Sprint30IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "phase30a:blocked"), "phase30a blocked note expected");
    CHECK(hasNote(r, "phase30b:blocked"), "phase30b blocked note expected");
    PASS();
}

void test_multiple_failure_notes_aggregate() {
    TEST(multiple_failure_notes_aggregate);
    auto s = allPass();
    s.debugSessionModel = false;
    s.phase30aIntegration = false;
    s.workflowAwareDebugHooks = false;
    auto r = Sprint30IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "fail:step554_debug_session_model"), "step554 failure note missing");
    CHECK(hasNote(r, "fail:step558_phase30a_integration"), "step558 failure note missing");
    CHECK(hasNote(r, "fail:step562_workflow_debug_hooks"), "step562 failure note missing");
    PASS();
}

void test_all_false_signals_fail_all_phases() {
    TEST(all_false_signals_fail_all_phases);
    Sprint30Signals s;
    auto r = Sprint30IntegrationSummary::summarize(s);
    CHECK(!r.phase30aPass && !r.phase30bPass && !r.sprint30Pass, "all phases should fail");
    PASS();
}

void test_single_phase30b_failure_keeps_phase30a_passed() {
    TEST(single_phase30b_failure_keeps_phase30a_passed);
    auto s = allPass();
    s.callStackInspector = false;
    auto r = Sprint30IntegrationSummary::summarize(s);
    CHECK(r.phase30aPass, "phase30a should remain pass");
    CHECK(!r.phase30bPass, "phase30b should fail");
    PASS();
}

int main() {
    std::cout << "Step 563: Sprint 30 Integration + Summary\n";

    test_all_signals_pass_sprint();                    // 1
    test_phase30a_failure_blocks_sprint();             // 2
    test_phase30b_failure_blocks_sprint();             // 3
    test_success_notes_emitted_on_full_pass();         // 4
    test_blocked_phase_notes_emitted_on_failure();     // 5
    test_multiple_failure_notes_aggregate();           // 6
    test_all_false_signals_fail_all_phases();          // 7
    test_single_phase30b_failure_keeps_phase30a_passed();// 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
