// Step 593: Sprint 33 Integration + Program Summary (8 tests)

#include "Sprint33IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint33IntegrationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static Sprint33Signals allPass() {
    Sprint33Signals s;
    s.onboardingFlow = true;
    s.workflowVisualization = true;
    s.capabilityDiscovery = true;
    s.guidedDemoMode = true;
    s.phase33aIntegration = true;
    s.releaseReadinessPack = true;
    s.crashRecoverySweep = true;
    s.benchmarkHarness = true;
    s.docsPlaybooks = true;
    return s;
}

void test_all_signals_pass_sprint() {
    TEST(all_signals_pass_sprint);
    auto r = Sprint33IntegrationSummary::summarize(allPass());
    CHECK(r.phase33aPass, "phase33a should pass");
    CHECK(r.phase33bPass, "phase33b should pass");
    CHECK(r.sprint33Pass, "sprint33 should pass");
    PASS();
}

void test_phase33a_failure_blocks_sprint() {
    TEST(phase33a_failure_blocks_sprint);
    auto s = allPass();
    s.guidedDemoMode = false;
    auto r = Sprint33IntegrationSummary::summarize(s);
    CHECK(!r.phase33aPass, "phase33a should fail");
    CHECK(!r.sprint33Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step587_guided_demo_mode"), "step587 note expected");
    PASS();
}

void test_phase33b_failure_blocks_sprint() {
    TEST(phase33b_failure_blocks_sprint);
    auto s = allPass();
    s.benchmarkHarness = false;
    auto r = Sprint33IntegrationSummary::summarize(s);
    CHECK(!r.phase33bPass, "phase33b should fail");
    CHECK(!r.sprint33Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step591_benchmark_harness"), "step591 note expected");
    PASS();
}

void test_success_notes_emitted_on_full_pass() {
    TEST(success_notes_emitted_on_full_pass);
    auto r = Sprint33IntegrationSummary::summarize(allPass());
    CHECK(hasNote(r, "sprint33:product_experience_clear"), "success note missing");
    CHECK(hasNote(r, "sprint33:operational_hardening_ready"), "success note missing");
    CHECK(hasNote(r, "sprint33:competitive_readiness_established"), "success note missing");
    PASS();
}

void test_blocked_phase_notes_emitted_on_failure() {
    TEST(blocked_phase_notes_emitted_on_failure);
    auto s = allPass();
    s.onboardingFlow = false;
    s.releaseReadinessPack = false;
    auto r = Sprint33IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "phase33a:blocked"), "phase33a blocked expected");
    CHECK(hasNote(r, "phase33b:blocked"), "phase33b blocked expected");
    PASS();
}

void test_multiple_failure_notes_aggregate() {
    TEST(multiple_failure_notes_aggregate);
    auto s = allPass();
    s.workflowVisualization = false;
    s.phase33aIntegration = false;
    s.docsPlaybooks = false;
    auto r = Sprint33IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "fail:step585_workflow_visualization"), "step585 note missing");
    CHECK(hasNote(r, "fail:step588_phase33a_integration"), "step588 note missing");
    CHECK(hasNote(r, "fail:step592_docs_playbooks"), "step592 note missing");
    PASS();
}

void test_all_false_signals_fail_all_phases() {
    TEST(all_false_signals_fail_all_phases);
    Sprint33Signals s;
    auto r = Sprint33IntegrationSummary::summarize(s);
    CHECK(!r.phase33aPass && !r.phase33bPass && !r.sprint33Pass, "all should fail");
    PASS();
}

void test_single_phase33b_failure_keeps_phase33a_passed() {
    TEST(single_phase33b_failure_keeps_phase33a_passed);
    auto s = allPass();
    s.crashRecoverySweep = false;
    auto r = Sprint33IntegrationSummary::summarize(s);
    CHECK(r.phase33aPass, "phase33a should pass");
    CHECK(!r.phase33bPass, "phase33b should fail");
    PASS();
}

int main() {
    std::cout << "Step 593: Sprint 33 Integration + Program Summary\n";

    test_all_signals_pass_sprint();                      // 1
    test_phase33a_failure_blocks_sprint();               // 2
    test_phase33b_failure_blocks_sprint();               // 3
    test_success_notes_emitted_on_full_pass();           // 4
    test_blocked_phase_notes_emitted_on_failure();       // 5
    test_multiple_failure_notes_aggregate();             // 6
    test_all_false_signals_fail_all_phases();            // 7
    test_single_phase33b_failure_keeps_phase33a_passed();// 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
