// Step 507: Full Test Suite Regression Gate Tests (12 tests)

#include "FullRegressionGate.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static FullRegressionReport runSample() {
    return FullRegressionGate::evaluate(FullRegressionGate::samplePhase25Dataset(), 60.0);
}

void test_sample_dataset_represents_four_suite_partitions() {
    TEST(sample_dataset_represents_four_suite_partitions);
    auto report = runSample();
    CHECK(report.suites == 4, "expected 4 suite partitions");
    PASS();
}

void test_sample_dataset_exceeds_five_thousand_tests() {
    TEST(sample_dataset_exceeds_five_thousand_tests);
    auto report = runSample();
    CHECK(report.totalTestsExecuted > 5000, "expected >5000 executed tests");
    PASS();
}

void test_all_runs_passing_sets_all_passing_true() {
    TEST(all_runs_passing_sets_all_passing_true);
    auto report = runSample();
    CHECK(report.allPassing, "allPassing should be true");
    CHECK(report.totalFailures == 0, "expected zero failures");
    PASS();
}

void test_three_run_stability_detects_no_flakes_in_sample() {
    TEST(three_run_stability_detects_no_flakes_in_sample);
    auto report = runSample();
    CHECK(report.noFlakes, "noFlakes should be true");
    CHECK(report.flakySuites.empty(), "expected no flaky suites");
    PASS();
}

void test_performance_budget_under_sixty_seconds_passes_for_sample() {
    TEST(performance_budget_under_sixty_seconds_passes_for_sample);
    auto report = runSample();
    CHECK(report.performanceTargetMet, "performance budget should pass");
    CHECK(report.totalDurationSeconds < 60.0, "total duration should be <60s");
    PASS();
}

void test_memory_leak_budget_detects_zero_leaks_in_sample() {
    TEST(memory_leak_budget_detects_zero_leaks_in_sample);
    auto report = runSample();
    CHECK(report.memoryStable, "memoryStable should be true");
    CHECK(report.totalLeaks == 0, "expected zero leaks");
    PASS();
}

void test_gate_pass_requires_conjunction_of_all_quality_signals() {
    TEST(gate_pass_requires_conjunction_of_all_quality_signals);
    auto report = runSample();
    CHECK(report.gatePass ==
          (report.allPassing && report.noFlakes && report.performanceTargetMet && report.memoryStable),
          "gate conjunction mismatch");
    PASS();
}

void test_flaky_suite_is_detected_when_one_run_fails() {
    TEST(flaky_suite_is_detected_when_one_run_fails);
    std::vector<RegressionSuiteExecution> suites = {
        {"suite_a", {{true, 10.0, 0, 100}, {false, 10.0, 0, 100}, {true, 10.0, 0, 100}}}
    };
    auto report = FullRegressionGate::evaluate(suites, 60.0);
    CHECK(!report.noFlakes, "flake should be detected");
    CHECK(!report.flakySuites.empty(), "expected flaky suite list");
    PASS();
}

void test_failure_suite_is_recorded_when_any_run_fails() {
    TEST(failure_suite_is_recorded_when_any_run_fails);
    std::vector<RegressionSuiteExecution> suites = {
        {"suite_b", {{true, 5.0, 0, 50}, {false, 5.0, 0, 50}, {false, 5.0, 0, 50}}}
    };
    auto report = FullRegressionGate::evaluate(suites, 60.0);
    CHECK(!report.allPassing, "allPassing should be false");
    CHECK(!report.failedSuites.empty(), "expected failed suite list");
    PASS();
}

void test_duration_budget_failure_is_reported() {
    TEST(duration_budget_failure_is_reported);
    std::vector<RegressionSuiteExecution> suites = {
        {"suite_c", {{true, 40.0, 0, 100}, {true, 40.0, 0, 100}, {true, 40.0, 0, 100}}}
    };
    auto report = FullRegressionGate::evaluate(suites, 60.0);
    CHECK(!report.performanceTargetMet, "duration budget should fail");
    PASS();
}

void test_memory_leak_failure_is_reported() {
    TEST(memory_leak_failure_is_reported);
    std::vector<RegressionSuiteExecution> suites = {
        {"suite_d", {{true, 10.0, 1, 100}, {true, 10.0, 0, 100}, {true, 10.0, 0, 100}}}
    };
    auto report = FullRegressionGate::evaluate(suites, 60.0);
    CHECK(!report.memoryStable, "memoryStable should fail with leaks");
    PASS();
}

void test_notes_include_gate_summary_and_test_count() {
    TEST(notes_include_gate_summary_and_test_count);
    auto report = runSample();
    CHECK(report.notes.size() >= 2, "expected summary notes");
    CHECK(report.notes[1].find("Total tests executed") != std::string::npos,
          "missing total-test-count note");
    PASS();
}

int main() {
    std::cout << "Step 507: Full Test Suite Regression Gate Tests\n";

    test_sample_dataset_represents_four_suite_partitions();            // 1
    test_sample_dataset_exceeds_five_thousand_tests();                 // 2
    test_all_runs_passing_sets_all_passing_true();                     // 3
    test_three_run_stability_detects_no_flakes_in_sample();            // 4
    test_performance_budget_under_sixty_seconds_passes_for_sample();   // 5
    test_memory_leak_budget_detects_zero_leaks_in_sample();            // 6
    test_gate_pass_requires_conjunction_of_all_quality_signals();      // 7
    test_flaky_suite_is_detected_when_one_run_fails();                 // 8
    test_failure_suite_is_recorded_when_any_run_fails();               // 9
    test_duration_budget_failure_is_reported();                        // 10
    test_memory_leak_failure_is_reported();                            // 11
    test_notes_include_gate_summary_and_test_count();                  // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
