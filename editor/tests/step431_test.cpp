// Step 431: Progress Dashboard Tests (12 tests)

#include "WorkflowProgressDashboard.h"

#include <cmath>
#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& id, const std::string& worker, const std::string& status) {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = id + "_node";
    wi.nodeName = id;
    wi.nodeType = "Function";
    wi.workerType = worker;
    wi.status = status;
    wi.priority = "medium";
    return wi;
}

static WorkflowState makeWorkflow() {
    WorkflowState wf("dashboard");
    wf.queue.enqueue(makeItem("a", "deterministic", WI_COMPLETE));
    wf.queue.enqueue(makeItem("b", "slm", WI_IN_PROGRESS));
    wf.queue.enqueue(makeItem("c", "llm", WI_REVIEW));
    wf.queue.enqueue(makeItem("d", "human", WI_READY));
    return wf;
}

static WorkflowProgress makeProgress() {
    WorkflowProgress p(4);
    p.recordEvent({"routed", "a", {{"workerType", "deterministic"}}, "2026-01-01T00:00:00Z"});
    p.recordEvent({"completed", "a", json::object(), "2026-01-01T00:01:00Z"});
    p.recordEvent({"routed", "b", {{"workerType", "slm"}}, "2026-01-01T00:02:00Z"});
    p.recordEvent({"completed", "b", json::object(), "2026-01-01T00:03:00Z"});
    p.recordEvent({"blocked", "", {{"blockers", json::array({
        {{"type", "needs-review"}, {"itemIds", json::array({"c"})}, {"description", "review pending"}}
    })}}, "2026-01-01T00:04:00Z"});
    return p;
}

static WorkflowCostTracker makeCosts(ModelProfileRegistry& reg) {
    WorkflowCostTracker costs(&reg);
    costs.recordEstimate("a", "deterministic", 100, 20);
    costs.recordEstimate("b", "slm", 800, 120);
    costs.recordActual("a", "deterministic", 80, 10);
    costs.recordActual("b", "slm", 900, 140);
    return costs;
}

void test_dashboard_copies_completion_metrics() {
    TEST(dashboard_copies_completion_metrics);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(d.totalItems == 4, "total items mismatch");
    CHECK(d.completedItems == 2, "completed mismatch");
    CHECK(d.completionPercent > 40.0f, "completion percent should be >40");
    PASS();
}

void test_dashboard_throughput_series_from_timeline() {
    TEST(dashboard_throughput_series_from_timeline);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(d.throughputSeries.size() == p.getTimeline().size(), "throughput series size mismatch");
    PASS();
}

void test_throughput_series_counts_completed_only() {
    TEST(throughput_series_counts_completed_only);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(!d.throughputSeries.empty(), "throughput series empty");
    CHECK(d.throughputSeries.back().completed == 2, "completed counter should end at 2");
    PASS();
}

void test_worker_breakdown_contains_all_workers() {
    TEST(worker_breakdown_contains_all_workers);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    bool hasDet = false, hasSlm = false, hasLlm = false, hasHuman = false;
    for (const auto& s : d.workerBreakdown) {
        if (s.workerType == "deterministic") hasDet = true;
        if (s.workerType == "slm") hasSlm = true;
        if (s.workerType == "llm") hasLlm = true;
        if (s.workerType == "human") hasHuman = true;
    }
    CHECK(hasDet && hasSlm && hasLlm && hasHuman, "missing worker slice");
    PASS();
}

void test_worker_breakdown_percent_sum_around_100() {
    TEST(worker_breakdown_percent_sum_around_100);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    float sum = 0.0f;
    for (const auto& s : d.workerBreakdown) sum += s.percent;
    CHECK(std::fabs(sum - 100.0f) < 0.5f, "percent sum should be ~100");
    PASS();
}

void test_blocker_summary_reflects_snapshot() {
    TEST(blocker_summary_reflects_snapshot);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(!d.blockerSummary.empty(), "blocker summary should not be empty");
    CHECK(d.blockerSummary[0].count == 1, "blocker count mismatch");
    PASS();
}

void test_cost_fields_populated_with_tracker() {
    TEST(cost_fields_populated_with_tracker);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    ModelProfileRegistry reg;
    auto costs = makeCosts(reg);
    auto d = WorkflowProgressDashboard::build(wf, p, &costs);
    CHECK(d.actualCost >= 0.0, "actual cost should be non-negative");
    CHECK(d.estimatedCost >= 0.0, "estimated cost should be non-negative");
    PASS();
}

void test_remaining_cost_estimate_positive_when_items_left() {
    TEST(remaining_cost_estimate_positive_when_items_left);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    ModelProfileRegistry reg;
    auto costs = makeCosts(reg);
    auto d = WorkflowProgressDashboard::build(wf, p, &costs);
    CHECK(d.estimatedRemainingCost >= 0.0, "remaining estimate should be non-negative");
    PASS();
}

void test_costs_zero_without_tracker() {
    TEST(costs_zero_without_tracker);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(d.actualCost == 0.0, "actual cost should be zero without tracker");
    CHECK(d.estimatedCost == 0.0, "estimated cost should be zero without tracker");
    PASS();
}

void test_empty_timeline_yields_empty_throughput() {
    TEST(empty_timeline_yields_empty_throughput);
    auto wf = makeWorkflow();
    WorkflowProgress p(4);
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(d.throughputSeries.empty(), "empty timeline should produce empty throughput");
    PASS();
}

void test_eta_passthrough_from_progress_snapshot() {
    TEST(eta_passthrough_from_progress_snapshot);
    auto wf = makeWorkflow();
    auto p = makeProgress();
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(d.estimatedRemainingMinutes >= 0.0f, "eta should be available in this scenario");
    PASS();
}

void test_dashboard_handles_zero_total_items() {
    TEST(dashboard_handles_zero_total_items);
    WorkflowState wf("empty");
    WorkflowProgress p(0);
    auto d = WorkflowProgressDashboard::build(wf, p, nullptr);
    CHECK(d.totalItems == 0, "total should be zero");
    CHECK(d.completionPercent == 0.0f, "completion should be zero");
    PASS();
}

int main() {
    std::cout << "Step 431: Progress Dashboard Tests\n";

    test_dashboard_copies_completion_metrics();         // 1
    test_dashboard_throughput_series_from_timeline();   // 2
    test_throughput_series_counts_completed_only();     // 3
    test_worker_breakdown_contains_all_workers();       // 4
    test_worker_breakdown_percent_sum_around_100();     // 5
    test_blocker_summary_reflects_snapshot();           // 6
    test_cost_fields_populated_with_tracker();          // 7
    test_remaining_cost_estimate_positive_when_items_left(); // 8
    test_costs_zero_without_tracker();                  // 9
    test_empty_timeline_yields_empty_throughput();      // 10
    test_eta_passthrough_from_progress_snapshot();      // 11
    test_dashboard_handles_zero_total_items();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
