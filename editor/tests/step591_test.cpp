// Step 591: Benchmark and Comparison Harness (12 tests)

#include "BenchmarkComparisonHarness.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static BenchmarkSample sample(const std::string& runId,
                              const std::string& scenario,
                              int t,
                              int r,
                              int e) {
    return {runId, scenario, t, r, e};
}

void test_add_sample_success() {
    TEST(add_sample_success);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("r1", "intake", 100, 95, 80), &error), "add should succeed");
    CHECK(h.aggregates().size() == 1, "aggregate count mismatch");
    PASS();
}

void test_add_sample_rejects_missing_run_id() {
    TEST(add_sample_rejects_missing_run_id);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(!h.addSample(sample("", "intake", 100, 95, 80), &error), "add should fail");
    CHECK(error == "run_id_missing", "wrong error");
    PASS();
}

void test_add_sample_rejects_missing_scenario() {
    TEST(add_sample_rejects_missing_scenario);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(!h.addSample(sample("r1", "", 100, 95, 80), &error), "add should fail");
    CHECK(error == "scenario_missing", "wrong error");
    PASS();
}

void test_add_sample_rejects_negative_metric() {
    TEST(add_sample_rejects_negative_metric);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(!h.addSample(sample("r1", "intake", -1, 95, 80), &error), "add should fail");
    CHECK(error == "metric_invalid", "wrong error");
    PASS();
}

void test_add_sample_rejects_duplicate_run_id() {
    TEST(add_sample_rejects_duplicate_run_id);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("r1", "intake", 100, 95, 80), &error), "first add failed");
    CHECK(!h.addSample(sample("r1", "intake", 101, 96, 81), &error), "duplicate should fail");
    CHECK(error == "run_duplicate", "wrong error");
    PASS();
}

void test_aggregates_average_metrics_per_scenario() {
    TEST(aggregates_average_metrics_per_scenario);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("r1", "intake", 100, 90, 80), &error), "add r1 failed");
    CHECK(h.addSample(sample("r2", "intake", 200, 100, 60), &error), "add r2 failed");
    const auto a = h.aggregates();
    CHECK(a.size() == 1, "one aggregate expected");
    CHECK(a[0].avgThroughput == 150, "throughput avg mismatch");
    CHECK(a[0].avgReliability == 95, "reliability avg mismatch");
    CHECK(a[0].avgTokenEfficiency == 70, "efficiency avg mismatch");
    PASS();
}

void test_aggregates_include_multiple_scenarios() {
    TEST(aggregates_include_multiple_scenarios);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("r1", "intake", 100, 90, 80), &error), "add r1 failed");
    CHECK(h.addSample(sample("r2", "verify", 200, 95, 70), &error), "add r2 failed");
    CHECK(h.aggregates().size() == 2, "two aggregates expected");
    PASS();
}

void test_compare_runs_returns_deltas() {
    TEST(compare_runs_returns_deltas);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("base", "intake", 100, 90, 80), &error), "add base failed");
    CHECK(h.addSample(sample("cand", "intake", 130, 92, 78), &error), "add cand failed");
    const auto c = h.compareRuns("base", "cand");
    CHECK(c.size() == 1, "comparison should exist");
    CHECK(c[0].throughputDelta == 30, "throughput delta mismatch");
    CHECK(c[0].reliabilityDelta == 2, "reliability delta mismatch");
    CHECK(c[0].tokenEfficiencyDelta == -2, "efficiency delta mismatch");
    PASS();
}

void test_compare_runs_empty_when_missing_run() {
    TEST(compare_runs_empty_when_missing_run);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("base", "intake", 100, 90, 80), &error), "add base failed");
    CHECK(h.compareRuns("base", "missing").empty(), "comparison should be empty");
    PASS();
}

void test_compare_runs_empty_when_scenario_mismatch() {
    TEST(compare_runs_empty_when_scenario_mismatch);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("base", "intake", 100, 90, 80), &error), "add base failed");
    CHECK(h.addSample(sample("cand", "verify", 130, 92, 78), &error), "add cand failed");
    CHECK(h.compareRuns("base", "cand").empty(), "comparison should be empty");
    PASS();
}

void test_top_by_throughput_sorted_desc() {
    TEST(top_by_throughput_sorted_desc);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("r1", "intake", 100, 90, 80), &error), "add r1 failed");
    CHECK(h.addSample(sample("r2", "intake", 300, 90, 80), &error), "add r2 failed");
    CHECK(h.addSample(sample("r3", "intake", 200, 90, 80), &error), "add r3 failed");
    const auto top = h.topByThroughput("intake", 2);
    CHECK(top.size() == 2, "top count mismatch");
    CHECK(top[0].runId == "r2", "top run mismatch");
    CHECK(top[1].runId == "r3", "second run mismatch");
    PASS();
}

void test_top_by_throughput_empty_limit() {
    TEST(top_by_throughput_empty_limit);
    BenchmarkComparisonHarness h;
    std::string error;
    CHECK(h.addSample(sample("r1", "intake", 100, 90, 80), &error), "add failed");
    CHECK(h.topByThroughput("intake", 0).empty(), "zero limit should return empty");
    PASS();
}

int main() {
    std::cout << "Step 591: Benchmark and Comparison Harness\n";

    test_add_sample_success();                        // 1
    test_add_sample_rejects_missing_run_id();        // 2
    test_add_sample_rejects_missing_scenario();      // 3
    test_add_sample_rejects_negative_metric();       // 4
    test_add_sample_rejects_duplicate_run_id();      // 5
    test_aggregates_average_metrics_per_scenario();  // 6
    test_aggregates_include_multiple_scenarios();    // 7
    test_compare_runs_returns_deltas();              // 8
    test_compare_runs_empty_when_missing_run();      // 9
    test_compare_runs_empty_when_scenario_mismatch();// 10
    test_top_by_throughput_sorted_desc();            // 11
    test_top_by_throughput_empty_limit();            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
