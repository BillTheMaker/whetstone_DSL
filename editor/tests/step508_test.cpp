// Step 508: Sprint 25 Summary + Readiness Tests (8 tests)

#include "Sprint25SummaryReadiness.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Sprint25Summary runNow() {
    return Sprint25SummaryReadiness::generate();
}

void test_language_and_tooling_metrics_meet_sprint_targets() {
    TEST(language_and_tooling_metrics_meet_sprint_targets);
    auto s = runNow();
    CHECK(s.languageParsersAndGenerators >= 19, "expected 19+ language parsers/generators");
    CHECK(s.mcpTools >= 90, "expected 90+ MCP tools");
    PASS();
}

void test_annotation_metrics_meet_target_scale() {
    TEST(annotation_metrics_meet_target_scale);
    auto s = runNow();
    CHECK(s.annotationTypes >= 80, "expected 80+ annotation types");
    CHECK(s.annotationSubjects >= 10, "expected 10 annotation subjects");
    PASS();
}

void test_step_and_test_totals_cross_project_thresholds() {
    TEST(step_and_test_totals_cross_project_thresholds);
    auto s = runNow();
    CHECK(s.totalSteps >= 500, "expected 500+ steps");
    CHECK(s.totalTests >= 5000, "expected 5000+ tests");
    PASS();
}

void test_self_hosting_coverage_meets_or_exceeds_ninety_five_percent() {
    TEST(self_hosting_coverage_meets_or_exceeds_ninety_five_percent);
    auto s = runNow();
    CHECK(s.selfHostingCoverage >= 0.95, "expected self-hosting coverage >=95%");
    PASS();
}

void test_all_four_end_to_end_scenarios_are_validated() {
    TEST(all_four_end_to_end_scenarios_are_validated);
    auto s = runNow();
    CHECK(s.validatedScenarios >= 4, "expected 4 validated scenarios");
    PASS();
}

void test_event_stream_and_workflow_logging_readiness_signals_are_true() {
    TEST(event_stream_and_workflow_logging_readiness_signals_are_true);
    auto s = runNow();
    CHECK(s.eventStreamDataReady, "event stream data should be ready");
    CHECK(s.workflowDecisionsLogged, "workflow decisions should be logged");
    CHECK(s.transpilationPairsLogged, "transpilation pairs should be logged");
    PASS();
}

void test_sprint_twenty_six_readiness_gate_passes() {
    TEST(sprint_twenty_six_readiness_gate_passes);
    auto s = runNow();
    CHECK(s.sprint26Ready, "Sprint 26 readiness gate should pass");
    PASS();
}

void test_notes_include_summary_and_readiness_statement() {
    TEST(notes_include_summary_and_readiness_statement);
    auto s = runNow();
    CHECK(s.notes.size() >= 2, "expected summary notes");
    CHECK(s.notes[1].find("Sprint 26") != std::string::npos, "missing Sprint 26 note");
    PASS();
}

int main() {
    std::cout << "Step 508: Sprint 25 Summary + Readiness Tests\n";

    test_language_and_tooling_metrics_meet_sprint_targets();                 // 1
    test_annotation_metrics_meet_target_scale();                             // 2
    test_step_and_test_totals_cross_project_thresholds();                    // 3
    test_self_hosting_coverage_meets_or_exceeds_ninety_five_percent();      // 4
    test_all_four_end_to_end_scenarios_are_validated();                      // 5
    test_event_stream_and_workflow_logging_readiness_signals_are_true();     // 6
    test_sprint_twenty_six_readiness_gate_passes();                          // 7
    test_notes_include_summary_and_readiness_statement();                    // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
