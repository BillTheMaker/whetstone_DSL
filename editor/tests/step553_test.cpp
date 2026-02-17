// Step 553: Sprint 29 Integration + Summary (8 tests)

#include "Sprint29IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint29IntegrationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static Sprint29Signals allPass() {
    Sprint29Signals s;
    s.constrainedRoutingRuleset = true;
    s.contextBundleMinimizer = true;
    s.confidenceCalibrator = true;
    s.costPolicyGuard = true;
    s.phase29aIntegration = true;
    s.costSuggestionEngine = true;
    s.workerEfficiencyDashboard = true;
    s.reviewGateRefinement = true;
    s.costQualityRegressionSuite = true;
    return s;
}

void test_all_signals_pass_sprint() {
    TEST(all_signals_pass_sprint);
    auto r = Sprint29IntegrationSummary::summarize(allPass());
    CHECK(r.phase29aPass, "phase29a should pass");
    CHECK(r.phase29bPass, "phase29b should pass");
    CHECK(r.sprint29Pass, "sprint should pass");
    PASS();
}

void test_phase29a_failure_blocks_sprint() {
    TEST(phase29a_failure_blocks_sprint);
    auto s = allPass();
    s.contextBundleMinimizer = false;
    auto r = Sprint29IntegrationSummary::summarize(s);
    CHECK(!r.phase29aPass, "phase29a should fail");
    CHECK(!r.sprint29Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step545_context_minimizer"), "step545 failure note expected");
    PASS();
}

void test_phase29b_failure_blocks_sprint() {
    TEST(phase29b_failure_blocks_sprint);
    auto s = allPass();
    s.costQualityRegressionSuite = false;
    auto r = Sprint29IntegrationSummary::summarize(s);
    CHECK(!r.phase29bPass, "phase29b should fail");
    CHECK(!r.sprint29Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step552_cost_quality_regression"), "step552 failure note expected");
    PASS();
}

void test_success_notes_emitted_on_full_pass() {
    TEST(success_notes_emitted_on_full_pass);
    auto r = Sprint29IntegrationSummary::summarize(allPass());
    CHECK(hasNote(r, "sprint29:routing_and_cost_policy_coherent"), "success note missing");
    CHECK(hasNote(r, "sprint29:context_width_reduced_without_quality_regression"), "success note missing");
    CHECK(hasNote(r, "sprint29:cost_controls_active_with_safety_preserved"), "success note missing");
    PASS();
}

void test_blocked_phase_notes_emitted_on_failure() {
    TEST(blocked_phase_notes_emitted_on_failure);
    auto s = allPass();
    s.costPolicyGuard = false;
    s.workerEfficiencyDashboard = false;
    auto r = Sprint29IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "phase29a:blocked"), "phase29a blocked note expected");
    CHECK(hasNote(r, "phase29b:blocked"), "phase29b blocked note expected");
    PASS();
}

void test_multiple_failure_notes_aggregate() {
    TEST(multiple_failure_notes_aggregate);
    auto s = allPass();
    s.constrainedRoutingRuleset = false;
    s.reviewGateRefinement = false;
    s.costSuggestionEngine = false;
    auto r = Sprint29IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "fail:step544_routing_ruleset"), "step544 note missing");
    CHECK(hasNote(r, "fail:step551_review_gate_refinement"), "step551 note missing");
    CHECK(hasNote(r, "fail:step549_cost_suggestions"), "step549 note missing");
    PASS();
}

void test_all_false_signals_fail_all_phases() {
    TEST(all_false_signals_fail_all_phases);
    Sprint29Signals s;
    auto r = Sprint29IntegrationSummary::summarize(s);
    CHECK(!r.phase29aPass && !r.phase29bPass && !r.sprint29Pass, "all phases should fail");
    PASS();
}

void test_single_phase29b_failure_keeps_phase29a_passed() {
    TEST(single_phase29b_failure_keeps_phase29a_passed);
    auto s = allPass();
    s.workerEfficiencyDashboard = false;
    auto r = Sprint29IntegrationSummary::summarize(s);
    CHECK(r.phase29aPass, "phase29a should remain pass");
    CHECK(!r.phase29bPass, "phase29b should fail");
    PASS();
}

int main() {
    std::cout << "Step 553: Sprint 29 Integration + Summary\n";

    test_all_signals_pass_sprint();                    // 1
    test_phase29a_failure_blocks_sprint();             // 2
    test_phase29b_failure_blocks_sprint();             // 3
    test_success_notes_emitted_on_full_pass();         // 4
    test_blocked_phase_notes_emitted_on_failure();     // 5
    test_multiple_failure_notes_aggregate();           // 6
    test_all_false_signals_fail_all_phases();          // 7
    test_single_phase29b_failure_keeps_phase29a_passed(); // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
