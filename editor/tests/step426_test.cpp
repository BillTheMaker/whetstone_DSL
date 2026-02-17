// Step 426: Cost Tracking + Reporting Tests (12 tests)

#include "WorkflowCostTracker.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ModelProfileRegistry makeRegistry() {
    ModelProfileRegistry reg;
    reg.setWorkerProfileMapping("llm", "claude-sonnet");
    reg.setWorkerProfileMapping("slm", "claude-haiku");
    return reg;
}

void test_record_estimate_creates_record() {
    TEST(record_estimate_creates_record);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 1000, 200);
    CHECK(tracker.hasRecord("w1"), "record missing");
    CHECK(tracker.getRecord("w1").estimatedInputTokens == 1000, "estimate not stored");
    PASS();
}

void test_record_actual_updates_existing_record() {
    TEST(record_actual_updates_existing_record);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 1000, 200);
    tracker.recordActual("w1", "llm", 1200, 250);
    auto rec = tracker.getRecord("w1");
    CHECK(rec.actualInputTokens == 1200, "actual input mismatch");
    CHECK(rec.actualOutputTokens == 250, "actual output mismatch");
    PASS();
}

void test_record_actual_creates_new_record() {
    TEST(record_actual_creates_new_record);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordActual("w2", "slm", 300, 100);
    CHECK(tracker.hasRecord("w2"), "actual-only record missing");
    PASS();
}

void test_profile_resolves_from_worker_mapping() {
    TEST(profile_resolves_from_worker_mapping);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 10, 10);
    CHECK(tracker.getRecord("w1").profileName == "claude-sonnet",
          "llm should map to sonnet");
    PASS();
}

void test_explicit_profile_overrides_mapping() {
    TEST(explicit_profile_overrides_mapping);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordActual("w1", "llm", 100, 100, "claude-opus");
    CHECK(tracker.getRecord("w1").profileName == "claude-opus",
          "explicit profile should win");
    PASS();
}

void test_summary_token_totals() {
    TEST(summary_token_totals);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 100, 20);
    tracker.recordEstimate("w2", "slm", 200, 30);
    tracker.recordActual("w1", "llm", 110, 25);
    tracker.recordActual("w2", "slm", 180, 28);
    auto s = tracker.summarize();
    CHECK(s.estimatedInputTokens == 300, "estimated input sum mismatch");
    CHECK(s.actualOutputTokens == 53, "actual output sum mismatch");
    PASS();
}

void test_summary_costs_non_negative() {
    TEST(summary_costs_non_negative);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 1500, 500);
    tracker.recordActual("w1", "llm", 1500, 500);
    auto s = tracker.summarize();
    CHECK(s.estimatedCost >= 0.0, "estimated cost negative");
    CHECK(s.actualCost >= 0.0, "actual cost negative");
    PASS();
}

void test_summary_cost_by_worker_present() {
    TEST(summary_cost_by_worker_present);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordActual("w1", "llm", 1000, 100);
    tracker.recordActual("w2", "slm", 1000, 100);
    auto s = tracker.summarize();
    CHECK(s.costByWorker.count("llm") == 1, "missing llm worker bucket");
    CHECK(s.costByWorker.count("slm") == 1, "missing slm worker bucket");
    PASS();
}

void test_summary_cost_by_profile_present() {
    TEST(summary_cost_by_profile_present);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordActual("w1", "llm", 1000, 100);
    auto s = tracker.summarize();
    CHECK(s.costByProfile.count("claude-sonnet") == 1, "missing sonnet profile bucket");
    PASS();
}

void test_variance_positive_when_actual_exceeds_estimate() {
    TEST(variance_positive_when_actual_exceeds_estimate);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 100, 50);
    tracker.recordActual("w1", "llm", 300, 200);
    auto s = tracker.summarize();
    CHECK(s.varianceCost() > 0.0, "expected positive variance");
    PASS();
}

void test_report_json_contains_core_fields() {
    TEST(report_json_contains_core_fields);
    auto reg = makeRegistry();
    WorkflowCostTracker tracker(&reg);
    tracker.recordEstimate("w1", "llm", 100, 50);
    tracker.recordActual("w1", "llm", 100, 50);
    auto j = tracker.reportJson();
    CHECK(j.contains("recordCount"), "missing recordCount");
    CHECK(j.contains("estimatedCost"), "missing estimatedCost");
    CHECK(j.contains("actualCost"), "missing actualCost");
    CHECK(j.contains("records"), "missing records");
    PASS();
}

void test_tracker_without_registry_uses_zero_costs() {
    TEST(tracker_without_registry_uses_zero_costs);
    WorkflowCostTracker tracker(nullptr);
    tracker.recordEstimate("w1", "llm", 1000, 1000);
    tracker.recordActual("w1", "llm", 1000, 1000);
    auto s = tracker.summarize();
    CHECK(s.estimatedCost == 0.0, "expected zero cost without registry");
    CHECK(s.actualCost == 0.0, "expected zero cost without registry");
    PASS();
}

int main() {
    std::cout << "Step 426: Cost Tracking + Reporting Tests\n";

    test_record_estimate_creates_record();                // 1
    test_record_actual_updates_existing_record();         // 2
    test_record_actual_creates_new_record();              // 3
    test_profile_resolves_from_worker_mapping();          // 4
    test_explicit_profile_overrides_mapping();            // 5
    test_summary_token_totals();                          // 6
    test_summary_costs_non_negative();                    // 7
    test_summary_cost_by_worker_present();                // 8
    test_summary_cost_by_profile_present();               // 9
    test_variance_positive_when_actual_exceeds_estimate();// 10
    test_report_json_contains_core_fields();              // 11
    test_tracker_without_registry_uses_zero_costs();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
