// Step 533: Sprint 27 Integration + Summary (8 tests)

#include "Sprint27IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Sprint27IntegrationResult& result,
                    const std::string& note) {
    for (const auto& n : result.notes) {
        if (n == note) return true;
    }
    return false;
}

static Sprint27Signals allPassSignals() {
    Sprint27Signals s;
    s.taskitemContractSchema = true;
    s.legalOperationGraph = true;
    s.symbolScopeExtractor = true;
    s.constraintDiagnostics = true;
    s.phase27aIntegrationGate = true;
    s.preApplyValidationGate = true;
    s.postApplyStructuralGate = true;
    s.contractDeltaChecker = true;
    s.retryEscalationProtocol = true;
    return s;
}

void test_all_signals_pass_yields_sprint_pass() {
    TEST(all_signals_pass_yields_sprint_pass);
    auto result = Sprint27IntegrationSummary::summarize(allPassSignals());
    CHECK(result.phase27aPass, "phase27a should pass");
    CHECK(result.phase27bPass, "phase27b should pass");
    CHECK(result.sprint27Pass, "sprint27 should pass");
    PASS();
}

void test_phase27a_failure_blocks_sprint() {
    TEST(phase27a_failure_blocks_sprint);
    auto s = allPassSignals();
    s.symbolScopeExtractor = false;
    auto result = Sprint27IntegrationSummary::summarize(s);
    CHECK(!result.phase27aPass, "phase27a should fail");
    CHECK(!result.sprint27Pass, "sprint should fail");
    CHECK(hasNote(result, "fail:step526_symbol_scope_extractor"), "missing failure note");
    PASS();
}

void test_phase27b_failure_blocks_sprint() {
    TEST(phase27b_failure_blocks_sprint);
    auto s = allPassSignals();
    s.contractDeltaChecker = false;
    auto result = Sprint27IntegrationSummary::summarize(s);
    CHECK(!result.phase27bPass, "phase27b should fail");
    CHECK(!result.sprint27Pass, "sprint should fail");
    CHECK(hasNote(result, "fail:step531_contract_delta_checker"), "missing failure note");
    PASS();
}

void test_pass_notes_are_emitted_on_success() {
    TEST(pass_notes_are_emitted_on_success);
    auto result = Sprint27IntegrationSummary::summarize(allPassSignals());
    CHECK(hasNote(result, "sprint27:constrained_pipeline_pass"), "missing pass note");
    CHECK(hasNote(result, "sprint27:out_of_scope_edits_prevented_by_construction"),
          "missing prevention note");
    CHECK(hasNote(result, "sprint27:hallucinated_symbol_calls_fail_fast"),
          "missing fail-fast note");
    PASS();
}

void test_blocked_phase_notes_are_emitted_on_failure() {
    TEST(blocked_phase_notes_are_emitted_on_failure);
    auto s = allPassSignals();
    s.legalOperationGraph = false;
    s.preApplyValidationGate = false;
    auto result = Sprint27IntegrationSummary::summarize(s);
    CHECK(hasNote(result, "phase27a:blocked"), "phase27a blocked note expected");
    CHECK(hasNote(result, "phase27b:blocked"), "phase27b blocked note expected");
    PASS();
}

void test_multiple_failure_notes_are_aggregated() {
    TEST(multiple_failure_notes_are_aggregated);
    auto s = allPassSignals();
    s.taskitemContractSchema = false;
    s.legalOperationGraph = false;
    s.retryEscalationProtocol = false;
    auto result = Sprint27IntegrationSummary::summarize(s);
    CHECK(hasNote(result, "fail:step524_taskitem_contract_schema"), "missing step524 note");
    CHECK(hasNote(result, "fail:step525_legal_operation_graph"), "missing step525 note");
    CHECK(hasNote(result, "fail:step532_retry_escalation_protocol"), "missing step532 note");
    PASS();
}

void test_all_false_signals_fail_every_phase() {
    TEST(all_false_signals_fail_every_phase);
    Sprint27Signals s;
    auto result = Sprint27IntegrationSummary::summarize(s);
    CHECK(!result.phase27aPass, "phase27a should fail");
    CHECK(!result.phase27bPass, "phase27b should fail");
    CHECK(!result.sprint27Pass, "sprint should fail");
    PASS();
}

void test_single_failure_preserves_other_phase_status() {
    TEST(single_failure_preserves_other_phase_status);
    auto s = allPassSignals();
    s.postApplyStructuralGate = false;
    auto result = Sprint27IntegrationSummary::summarize(s);
    CHECK(result.phase27aPass, "phase27a should remain pass");
    CHECK(!result.phase27bPass, "phase27b should fail");
    PASS();
}

int main() {
    std::cout << "Step 533: Sprint 27 Integration + Summary\n";

    test_all_signals_pass_yields_sprint_pass();              // 1
    test_phase27a_failure_blocks_sprint();                   // 2
    test_phase27b_failure_blocks_sprint();                   // 3
    test_pass_notes_are_emitted_on_success();                // 4
    test_blocked_phase_notes_are_emitted_on_failure();       // 5
    test_multiple_failure_notes_are_aggregated();            // 6
    test_all_false_signals_fail_every_phase();               // 7
    test_single_failure_preserves_other_phase_status();      // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
