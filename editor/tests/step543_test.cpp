// Step 543: Sprint 28 Integration + Summary (8 tests)

#include "Sprint28IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint28IntegrationResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

static Sprint28Signals allPass() {
    Sprint28Signals s;
    s.operationSelectorApi = true;
    s.symbolSelectorApi = true;
    s.argumentShapeValidator = true;
    s.constrainedExecutionTelemetry = true;
    s.phase28aIntegration = true;
    s.cppAdapter = true;
    s.pyTsAdapter = true;
    s.rustGoAdapter = true;
    s.crossLanguageConsistencyGate = true;
    return s;
}

void test_all_signals_pass_sprint() {
    TEST(all_signals_pass_sprint);
    auto r = Sprint28IntegrationSummary::summarize(allPass());
    CHECK(r.phase28aPass, "phase28a should pass");
    CHECK(r.phase28bPass, "phase28b should pass");
    CHECK(r.sprint28Pass, "sprint should pass");
    PASS();
}

void test_phase28a_failure_blocks_sprint() {
    TEST(phase28a_failure_blocks_sprint);
    auto s = allPass();
    s.argumentShapeValidator = false;
    auto r = Sprint28IntegrationSummary::summarize(s);
    CHECK(!r.phase28aPass, "phase28a should fail");
    CHECK(!r.sprint28Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step536_argument_shape_validator"), "step536 failure note expected");
    PASS();
}

void test_phase28b_failure_blocks_sprint() {
    TEST(phase28b_failure_blocks_sprint);
    auto s = allPass();
    s.rustGoAdapter = false;
    auto r = Sprint28IntegrationSummary::summarize(s);
    CHECK(!r.phase28bPass, "phase28b should fail");
    CHECK(!r.sprint28Pass, "sprint should fail");
    CHECK(hasNote(r, "fail:step541_rustgo_adapter"), "step541 failure note expected");
    PASS();
}

void test_success_notes_emitted_on_full_pass() {
    TEST(success_notes_emitted_on_full_pass);
    auto r = Sprint28IntegrationSummary::summarize(allPass());
    CHECK(hasNote(r, "sprint28:legal_choice_execution_stable"), "success note missing");
    CHECK(hasNote(r, "sprint28:multi_language_adapters_consistent"), "success note missing");
    CHECK(hasNote(r, "sprint28:out_of_scope_and_hallucinated_edits_blocked"), "success note missing");
    PASS();
}

void test_blocked_phase_notes_emitted_on_failure() {
    TEST(blocked_phase_notes_emitted_on_failure);
    auto s = allPass();
    s.symbolSelectorApi = false;
    s.cppAdapter = false;
    auto r = Sprint28IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "phase28a:blocked"), "phase28a blocked note expected");
    CHECK(hasNote(r, "phase28b:blocked"), "phase28b blocked note expected");
    PASS();
}

void test_multiple_failures_aggregate_notes() {
    TEST(multiple_failures_aggregate_notes);
    auto s = allPass();
    s.operationSelectorApi = false;
    s.pyTsAdapter = false;
    s.crossLanguageConsistencyGate = false;
    auto r = Sprint28IntegrationSummary::summarize(s);
    CHECK(hasNote(r, "fail:step534_operation_selector"), "step534 failure note missing");
    CHECK(hasNote(r, "fail:step540_pyts_adapter"), "step540 failure note missing");
    CHECK(hasNote(r, "fail:step542_cross_language_consistency"), "step542 failure note missing");
    PASS();
}

void test_all_false_signals_fail_all_phases() {
    TEST(all_false_signals_fail_all_phases);
    Sprint28Signals s;
    auto r = Sprint28IntegrationSummary::summarize(s);
    CHECK(!r.phase28aPass && !r.phase28bPass && !r.sprint28Pass, "all false should fail all phases");
    PASS();
}

void test_single_phase28b_failure_keeps_phase28a_passed() {
    TEST(single_phase28b_failure_keeps_phase28a_passed);
    auto s = allPass();
    s.crossLanguageConsistencyGate = false;
    auto r = Sprint28IntegrationSummary::summarize(s);
    CHECK(r.phase28aPass, "phase28a should still pass");
    CHECK(!r.phase28bPass, "phase28b should fail");
    PASS();
}

int main() {
    std::cout << "Step 543: Sprint 28 Integration + Summary\n";

    test_all_signals_pass_sprint();                         // 1
    test_phase28a_failure_blocks_sprint();                  // 2
    test_phase28b_failure_blocks_sprint();                  // 3
    test_success_notes_emitted_on_full_pass();              // 4
    test_blocked_phase_notes_emitted_on_failure();          // 5
    test_multiple_failures_aggregate_notes();               // 6
    test_all_false_signals_fail_all_phases();               // 7
    test_single_phase28b_failure_keeps_phase28a_passed();   // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
