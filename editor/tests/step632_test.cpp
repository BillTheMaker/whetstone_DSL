// Step 632: Agent task status overlay (12 tests)

#include "AgentTaskStatusOverlay.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_summary_counts_running_and_pending() {
    TEST(summary_counts_running_and_pending);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::setMaxSlots(&state, 2);
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::enqueueTask(&state, "b");
    (void)AgentTaskSlots::enqueueTask(&state, "c");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    auto summary = AgentTaskStatusOverlay::buildSummary(state);
    CHECK(summary.running == 2, "running count mismatch");
    CHECK(summary.pending == 1, "pending count mismatch");
    PASS();
}

void test_summary_label_format_matches_spec() {
    TEST(summary_label_format_matches_spec);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    auto summary = AgentTaskStatusOverlay::buildSummary(state);
    CHECK(summary.label == "[Agent: 1 running, 0 pending]", "summary label mismatch");
    PASS();
}

void test_summary_has_work_false_when_no_tasks() {
    TEST(summary_has_work_false_when_no_tasks);
    AgentTaskSlotsState state;
    auto summary = AgentTaskStatusOverlay::buildSummary(state);
    CHECK(!summary.hasWork, "hasWork should be false");
    PASS();
}

void test_build_rows_returns_all_tasks() {
    TEST(build_rows_returns_all_tasks);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::enqueueTask(&state, "b");
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows.size() == 2, "rows size mismatch");
    PASS();
}

void test_running_row_elapsed_uses_now_minus_started() {
    TEST(running_row_elapsed_uses_now_minus_started);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlotsAt(&state, 100);
    CHECK(AgentTaskSlots::tickTaskElapsed(&state, id, 109), "tick should succeed");
    auto rows = AgentTaskStatusOverlay::buildRows(state, 112);
    CHECK(rows[0].elapsedSeconds == 12, "elapsed should use running now");
    PASS();
}

void test_non_running_row_uses_last_elapsed() {
    TEST(non_running_row_uses_last_elapsed);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlotsAt(&state, 100);
    (void)AgentTaskSlots::tickTaskElapsed(&state, id, 115);
    (void)AgentTaskSlots::completeTask(&state, id);
    auto rows = AgentTaskStatusOverlay::buildRows(state, 220);
    CHECK(rows[0].elapsedSeconds == 15, "completed row should keep last elapsed");
    PASS();
}

void test_row_can_cancel_for_pending_task() {
    TEST(row_can_cancel_for_pending_task);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::setMaxSlots(&state, 1);
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::enqueueTask(&state, "b");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows[1].status == "pending", "second row should be pending");
    CHECK(rows[1].canCancel, "pending task should be cancellable");
    PASS();
}

void test_row_can_cancel_for_running_task() {
    TEST(row_can_cancel_for_running_task);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows[0].status == "running", "row should be running");
    CHECK(rows[0].canCancel, "running task should be cancellable");
    PASS();
}

void test_row_cannot_cancel_for_completed_task() {
    TEST(row_cannot_cancel_for_completed_task);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    (void)AgentTaskSlots::completeTask(&state, id);
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows[0].status == "completed", "row should be completed");
    CHECK(!rows[0].canCancel, "completed task should not be cancellable");
    PASS();
}

void test_build_rows_maps_tool_calls_and_current_step() {
    TEST(build_rows_maps_tool_calls_and_current_step);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    CHECK(AgentTaskSlots::updateTaskProgress(&state, id, 3, "mutating AST"), "progress update failed");
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows[0].toolCallsMade == 3, "tool call count mismatch");
    CHECK(rows[0].currentStep == "mutating AST", "current step mismatch");
    PASS();
}

void test_build_rows_defaults_current_step_to_dash() {
    TEST(build_rows_defaults_current_step_to_dash);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows[0].currentStep == "-", "default current step should be dash");
    PASS();
}

void test_cancel_task_marks_status_cancelled() {
    TEST(cancel_task_marks_status_cancelled);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    CHECK(AgentTaskStatusOverlay::cancelTask(&state, id), "cancel should succeed");
    auto rows = AgentTaskStatusOverlay::buildRows(state, 0);
    CHECK(rows[0].status == "cancelled", "status should be cancelled");
    PASS();
}

int main() {
    std::cout << "Step 632: Agent task status overlay\n";

    test_summary_counts_running_and_pending();          // 1
    test_summary_label_format_matches_spec();           // 2
    test_summary_has_work_false_when_no_tasks();        // 3
    test_build_rows_returns_all_tasks();                // 4
    test_running_row_elapsed_uses_now_minus_started();  // 5
    test_non_running_row_uses_last_elapsed();           // 6
    test_row_can_cancel_for_pending_task();             // 7
    test_row_can_cancel_for_running_task();             // 8
    test_row_cannot_cancel_for_completed_task();        // 9
    test_build_rows_maps_tool_calls_and_current_step(); // 10
    test_build_rows_defaults_current_step_to_dash();    // 11
    test_cancel_task_marks_status_cancelled();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
