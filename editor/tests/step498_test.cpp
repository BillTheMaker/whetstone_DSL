// Step 498: Phase 25a Integration Tests (8 tests)

#include "SelfHostingPhase25aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Phase25aIntegrationResult runNow() {
    return SelfHostingPhase25aIntegration::runCurrentState();
}

void test_full_pipeline_runs_parse_annotate_transpile_workflow_metrics() {
    TEST(full_pipeline_runs_parse_annotate_transpile_workflow_metrics);
    auto r = runNow();
    CHECK(r.parseAudit.totalFiles > 0, "parse audit missing");
    CHECK(r.annotationAudit.totalFiles > 0, "annotation audit missing");
    CHECK(!r.transpileAudit.files.empty(), "transpile audit missing");
    CHECK(!r.workflowAudit.tasks.empty(), "workflow audit missing");
    CHECK(r.metrics.filesTotal > 0, "metrics missing");
    PASS();
}

void test_parse_coverage_gate_uses_95_percent_threshold_semantics() {
    TEST(parse_coverage_gate_uses_95_percent_threshold_semantics);
    auto r = runNow();
    bool expected = r.parseAudit.parseRate() >= 0.95;
    CHECK(r.parseCoverageGate == expected, "parse gate mismatch");
    PASS();
}

void test_transpile_gate_requires_at_least_three_headers() {
    TEST(transpile_gate_requires_at_least_three_headers);
    auto r = runNow();
    CHECK(r.transpileAudit.files.size() >= 3, "expected >=3 transpiled headers");
    CHECK(r.transpileGate, "transpile gate should pass");
    PASS();
}

void test_workflow_gate_requires_all_deterministic_outputs_valid() {
    TEST(workflow_gate_requires_all_deterministic_outputs_valid);
    auto r = runNow();
    bool expected = (r.workflowAudit.deterministicCount == r.workflowAudit.deterministicValidCount);
    CHECK(r.workflowGate == expected, "workflow gate mismatch");
    PASS();
}

void test_phase_pass_equals_conjunction_of_all_gates() {
    TEST(phase_pass_equals_conjunction_of_all_gates);
    auto r = runNow();
    CHECK(r.phasePass == (r.parseCoverageGate && r.transpileGate && r.workflowGate),
          "phase pass logic mismatch");
    PASS();
}

void test_metrics_consistency_with_underlying_reports() {
    TEST(metrics_consistency_with_underlying_reports);
    auto r = runNow();
    CHECK(r.metrics.filesTotal == r.parseAudit.totalFiles, "metrics parse total mismatch");
    CHECK(r.metrics.transpileReviewRequired == r.transpileAudit.reviewRequiredCount,
          "metrics transpile review mismatch");
    CHECK(r.metrics.workflowTasks == (int)r.workflowAudit.tasks.size(), "metrics workflow task mismatch");
    PASS();
}

void test_notes_include_pipeline_execution_summary() {
    TEST(notes_include_pipeline_execution_summary);
    auto r = runNow();
    CHECK(!r.notes.empty(), "expected notes");
    CHECK(r.notes[0].find("pipeline executed") != std::string::npos, "missing execution note");
    PASS();
}

void test_phase_status_note_reflects_pass_or_gap_state() {
    TEST(phase_status_note_reflects_pass_or_gap_state);
    auto r = runNow();
    CHECK(r.notes.size() >= 2, "missing status note");
    if (r.phasePass) {
        CHECK(r.notes[1].find("passed") != std::string::npos, "missing pass note");
    } else {
        CHECK(r.notes[1].find("outstanding gaps") != std::string::npos, "missing gap note");
    }
    PASS();
}

int main() {
    std::cout << "Step 498: Phase 25a Integration Tests\n";

    test_full_pipeline_runs_parse_annotate_transpile_workflow_metrics(); // 1
    test_parse_coverage_gate_uses_95_percent_threshold_semantics();      // 2
    test_transpile_gate_requires_at_least_three_headers();               // 3
    test_workflow_gate_requires_all_deterministic_outputs_valid();       // 4
    test_phase_pass_equals_conjunction_of_all_gates();                   // 5
    test_metrics_consistency_with_underlying_reports();                  // 6
    test_notes_include_pipeline_execution_summary();                     // 7
    test_phase_status_note_reflects_pass_or_gap_state();                // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
