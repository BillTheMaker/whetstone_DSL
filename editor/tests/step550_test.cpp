// Step 550: Worker Efficiency Dashboard Data Model (12 tests)

#include "WorkerEfficiencyDashboardModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkerRunRecord run(const std::string& worker,
                           const std::string& task,
                           bool success,
                           int tokens,
                           int latency,
                           int rejects) {
    WorkerRunRecord r;
    r.workerId = worker;
    r.taskClass = task;
    r.success = success;
    r.tokensUsed = tokens;
    r.latencyMs = latency;
    r.rejectionCount = rejects;
    return r;
}

void test_empty_snapshot_has_no_groups() {
    TEST(empty_snapshot_has_no_groups);
    WorkerEfficiencyDashboardModel m;
    auto s = m.snapshot();
    CHECK(s.byWorker.empty(), "no worker groups expected");
    CHECK(s.byTaskClass.empty(), "no task groups expected");
    PASS();
}

void test_single_run_aggregates_worker_counts() {
    TEST(single_run_aggregates_worker_counts);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 100, 0));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].runs == 1, "worker run count mismatch");
    CHECK(s.byWorker["w1"].successCount == 1, "worker success count mismatch");
    PASS();
}

void test_single_run_aggregates_task_counts() {
    TEST(single_run_aggregates_task_counts);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 100, 0));
    auto s = m.snapshot();
    CHECK(s.byTaskClass["rename"].runs == 1, "task run count mismatch");
    PASS();
}

void test_success_and_failure_counts() {
    TEST(success_and_failure_counts);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 100, 0));
    m.record(run("w1", "rename", false, 25, 120, 1));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].successCount == 1, "success count mismatch");
    CHECK(s.byWorker["w1"].failureCount == 1, "failure count mismatch");
    PASS();
}

void test_success_rate_computation() {
    TEST(success_rate_computation);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 100, 0));
    m.record(run("w1", "rename", false, 25, 120, 1));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].successRate > 0.49 && s.byWorker["w1"].successRate < 0.51,
          "success rate should be 0.5");
    PASS();
}

void test_average_tokens_computation() {
    TEST(average_tokens_computation);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 100, 0));
    m.record(run("w1", "rename", true, 50, 100, 0));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].avgTokens == 40.0, "avg tokens should be 40");
    PASS();
}

void test_average_latency_computation() {
    TEST(average_latency_computation);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 80, 0));
    m.record(run("w1", "rename", true, 30, 120, 0));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].avgLatencyMs == 100.0, "avg latency should be 100");
    PASS();
}

void test_average_rejections_computation() {
    TEST(average_rejections_computation);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 80, 0));
    m.record(run("w1", "rename", false, 30, 120, 2));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].avgRejections == 1.0, "avg rejections should be 1");
    PASS();
}

void test_multiple_workers_are_grouped_separately() {
    TEST(multiple_workers_are_grouped_separately);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 80, 0));
    m.record(run("w2", "rename", false, 30, 120, 2));
    auto s = m.snapshot();
    CHECK(s.byWorker.size() == 2, "should have two worker groups");
    PASS();
}

void test_multiple_task_classes_are_grouped_separately() {
    TEST(multiple_task_classes_are_grouped_separately);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 80, 0));
    m.record(run("w1", "extract", true, 40, 140, 1));
    auto s = m.snapshot();
    CHECK(s.byTaskClass.size() == 2, "should have two task groups");
    PASS();
}

void test_totals_accumulate_per_group() {
    TEST(totals_accumulate_per_group);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 80, 0));
    m.record(run("w1", "rename", true, 50, 120, 2));
    auto s = m.snapshot();
    CHECK(s.byWorker["w1"].totalTokens == 80, "total tokens mismatch");
    CHECK(s.byWorker["w1"].totalLatencyMs == 200, "total latency mismatch");
    CHECK(s.byWorker["w1"].totalRejections == 2, "total rejections mismatch");
    PASS();
}

void test_task_group_metrics_reflect_cross_worker_runs() {
    TEST(task_group_metrics_reflect_cross_worker_runs);
    WorkerEfficiencyDashboardModel m;
    m.record(run("w1", "rename", true, 30, 80, 0));
    m.record(run("w2", "rename", false, 50, 140, 2));
    auto s = m.snapshot();
    CHECK(s.byTaskClass["rename"].runs == 2, "task runs mismatch");
    CHECK(s.byTaskClass["rename"].failureCount == 1, "task failures mismatch");
    PASS();
}

int main() {
    std::cout << "Step 550: Worker Efficiency Dashboard Data Model\n";

    test_empty_snapshot_has_no_groups();                     // 1
    test_single_run_aggregates_worker_counts();             // 2
    test_single_run_aggregates_task_counts();               // 3
    test_success_and_failure_counts();                      // 4
    test_success_rate_computation();                        // 5
    test_average_tokens_computation();                      // 6
    test_average_latency_computation();                     // 7
    test_average_rejections_computation();                  // 8
    test_multiple_workers_are_grouped_separately();         // 9
    test_multiple_task_classes_are_grouped_separately();    // 10
    test_totals_accumulate_per_group();                     // 11
    test_task_group_metrics_reflect_cross_worker_runs();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
