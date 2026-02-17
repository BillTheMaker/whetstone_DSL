// Step 503: Phase 25b Integration Tests (8 tests)

#include "Phase25bIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Phase25bIntegrationResult runNow() {
    return Phase25bIntegration::run();
}

void test_all_four_scenarios_run_successfully() {
    TEST(all_four_scenarios_run_successfully);
    auto r = runNow();
    CHECK(r.scenarioPass.size() == 4, "expected 4 scenario pass flags");
    CHECK(r.allScenariosPassed, "all scenarios should pass");
    PASS();
}

void test_each_scenario_exercises_distinct_workflow_path_signals() {
    TEST(each_scenario_exercises_distinct_workflow_path_signals);
    auto r = runNow();
    CHECK(r.uniqueWorkflowPathsObserved, "expected unique workflow path coverage");
    PASS();
}

void test_cost_tracking_contains_all_scenarios_and_positive_estimates() {
    TEST(cost_tracking_contains_all_scenarios_and_positive_estimates);
    auto r = runNow();
    CHECK(r.costs.size() == 4, "expected 4 cost snapshots");
    for (const auto& c : r.costs) {
        CHECK(c.estimatedTokens > 0, "estimated tokens must be positive");
        CHECK(c.actualTokens >= 0, "actual tokens must be non-negative");
    }
    PASS();
}

void test_cost_tracking_accuracy_gate_passes_for_integration() {
    TEST(cost_tracking_accuracy_gate_passes_for_integration);
    auto r = runNow();
    CHECK(r.costTrackingAccurate, "cost tracking should be marked accurate");
    PASS();
}

void test_security_findings_are_aggregated_from_generated_code_paths() {
    TEST(security_findings_are_aggregated_from_generated_code_paths);
    auto r = runNow();
    CHECK(r.totalSecurityFindings > 0, "expected aggregated security findings");
    PASS();
}

void test_event_stream_captures_start_and_complete_for_each_scenario() {
    TEST(event_stream_captures_start_and_complete_for_each_scenario);
    auto r = runNow();
    CHECK(r.eventStreamComplete, "expected complete start/complete event pairs");
    CHECK(r.events.size() >= 8, "expected at least start+complete for 4 scenarios");
    PASS();
}

void test_multi_model_scenario_keeps_review_gates_exercised_in_integration() {
    TEST(multi_model_scenario_keeps_review_gates_exercised_in_integration);
    auto r = runNow();
    CHECK(r.multiModel.reviewGatesExercised, "multi-model review gates should be exercised");
    CHECK(r.multiModel.reviewGateCount >= 5, "expected review gates from llm + human tasks");
    PASS();
}

void test_integration_notes_include_execution_summary_and_status() {
    TEST(integration_notes_include_execution_summary_and_status);
    auto r = runNow();
    CHECK(r.notes.size() >= 2, "expected integration notes");
    CHECK(r.notes[0].find("integration pipeline executed") != std::string::npos,
          "missing execution summary");
    PASS();
}

int main() {
    std::cout << "Step 503: Phase 25b Integration Tests\n";

    test_all_four_scenarios_run_successfully();                            // 1
    test_each_scenario_exercises_distinct_workflow_path_signals();         // 2
    test_cost_tracking_contains_all_scenarios_and_positive_estimates();    // 3
    test_cost_tracking_accuracy_gate_passes_for_integration();             // 4
    test_security_findings_are_aggregated_from_generated_code_paths();     // 5
    test_event_stream_captures_start_and_complete_for_each_scenario();     // 6
    test_multi_model_scenario_keeps_review_gates_exercised_in_integration(); // 7
    test_integration_notes_include_execution_summary_and_status();         // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
