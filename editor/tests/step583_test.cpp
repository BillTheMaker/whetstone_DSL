// Step 583: Sprint 32 Integration + Summary (8 tests)

#include "Sprint32IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint32IntegrationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static Sprint32Signals allPass() {
    Sprint32Signals s;
    s.markdownSpecParser = true;
    s.requirementNormalization = true;
    s.scopeMilestoneDecomposer = true;
    s.architectReviewSurface = true;
    s.phase32aIntegration = true;
    s.taskitemGeneratorV2 = true;
    s.confidenceAmbiguityAnnotation = true;
    s.acceptanceCriteriaBinding = true;
    s.intakeQueueSimulationHarness = true;
    return s;
}

void test_all_signals_pass_sprint() {
    TEST(all_signals_pass_sprint);
    auto r = Sprint32IntegrationSummary::summarize(allPass());
    CHECK(r.phase32aPass, "phase32a should pass");
    CHECK(r.phase32bPass, "phase32b should pass");
    CHECK(r.sprint32Pass, "sprint32 should pass");
    PASS();
}

void test_phase32a_failure_blocks_sprint() {
    TEST(phase32a_failure_blocks_sprint);
    auto s = allPass();
    s.architectReviewSurface = false;
    auto r = Sprint32IntegrationSummary::summarize(s);
    CHECK(!r.phase32aPass, "phase32a should fail");
    CHECK(!r.sprint32Pass, "sprint32 should fail");
    CHECK(hasNote(r, "fail:step577_architect_review_surface"), "step577 failure note expected");
    PASS();
}

void test_phase32b_failure_blocks_sprint() {
    TEST(phase32b_failure_blocks_sprint);
    auto s = allPass();
    s.acceptanceCriteriaBinding = false;
    auto r = Sprint32IntegrationSummary::summarize(s);
    CHECK(!r.phase32bPass, "phase32b should fail");
    CHECK(!r.sprint32Pass, "sprint32 should fail");
    CHECK(hasNote(r, "fail:step581_acceptance_criteria_binding"), "step581 failure note expected");
    PASS();
}

void test_success_notes_emitted_on_full_pass() {
    TEST(success_notes_emitted_on_full_pass);
    auto r = Sprint32IntegrationSummary::summarize(allPass());
    CHECK(hasNote(r, "sprint32:architect_intake_pipeline_ready"), "success note missing");
    CHECK(hasNote(r, "sprint32:taskitem_queue_quality_signals_ready"), "success note missing");
    CHECK(hasNote(r, "sprint32:markdown_to_constrained_execution_stable"), "success note missing");
    PASS();
}

void test_blocked_phase_notes_emitted_on_failure() {
    TEST(blocked_phase_notes_emitted_on_failure);
    auto s = allPass();
    s.markdownSpecParser = false;
    s.taskitemGeneratorV2 = false;
    auto r = Sprint32IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "phase32a:blocked"), "phase32a blocked note expected");
    CHECK(hasNote(r, "phase32b:blocked"), "phase32b blocked note expected");
    PASS();
}

void test_multiple_failure_notes_aggregate() {
    TEST(multiple_failure_notes_aggregate);
    auto s = allPass();
    s.requirementNormalization = false;
    s.phase32aIntegration = false;
    s.intakeQueueSimulationHarness = false;
    auto r = Sprint32IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "fail:step575_requirement_normalization"), "step575 failure note missing");
    CHECK(hasNote(r, "fail:step578_phase32a_integration"), "step578 failure note missing");
    CHECK(hasNote(r, "fail:step582_intake_queue_simulation_harness"), "step582 failure note missing");
    PASS();
}

void test_all_false_signals_fail_all_phases() {
    TEST(all_false_signals_fail_all_phases);
    Sprint32Signals s;
    auto r = Sprint32IntegrationSummary::summarize(s);
    CHECK(!r.phase32aPass && !r.phase32bPass && !r.sprint32Pass, "all phases should fail");
    PASS();
}

void test_single_phase32b_failure_keeps_phase32a_passed() {
    TEST(single_phase32b_failure_keeps_phase32a_passed);
    auto s = allPass();
    s.confidenceAmbiguityAnnotation = false;
    auto r = Sprint32IntegrationSummary::summarize(s);
    CHECK(r.phase32aPass, "phase32a should remain pass");
    CHECK(!r.phase32bPass, "phase32b should fail");
    PASS();
}

int main() {
    std::cout << "Step 583: Sprint 32 Integration + Summary\n";

    test_all_signals_pass_sprint();                      // 1
    test_phase32a_failure_blocks_sprint();               // 2
    test_phase32b_failure_blocks_sprint();               // 3
    test_success_notes_emitted_on_full_pass();           // 4
    test_blocked_phase_notes_emitted_on_failure();       // 5
    test_multiple_failure_notes_aggregate();             // 6
    test_all_false_signals_fail_all_phases();            // 7
    test_single_phase32b_failure_keeps_phase32a_passed();// 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
