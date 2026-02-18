// Step 631: Background task slots (12 tests)

#include "AgentTaskSlots.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_default_max_slots_is_two() {
    TEST(default_max_slots_is_two);
    AgentTaskSlotsState state;
    CHECK(state.maxSlots == 2, "default max slots mismatch");
    PASS();
}

void test_set_max_slots_rejects_invalid_values() {
    TEST(set_max_slots_rejects_invalid_values);
    AgentTaskSlotsState state;
    CHECK(!AgentTaskSlots::setMaxSlots(&state, 0), "setMaxSlots should fail for 0");
    PASS();
}

void test_set_max_slots_applies_valid_value() {
    TEST(set_max_slots_applies_valid_value);
    AgentTaskSlotsState state;
    CHECK(AgentTaskSlots::setMaxSlots(&state, 4), "setMaxSlots should succeed");
    CHECK(state.maxSlots == 4, "maxSlots mismatch");
    PASS();
}

void test_enqueue_task_creates_pending_task() {
    TEST(enqueue_task_creates_pending_task);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "generate tests");
    CHECK(!id.empty(), "task id should not be empty");
    CHECK(state.tasks.size() == 1, "task count mismatch");
    CHECK(state.tasks[0].status == AgentTaskStatus::Pending, "task should be pending");
    PASS();
}

void test_enqueue_task_rejects_empty_description() {
    TEST(enqueue_task_rejects_empty_description);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "");
    CHECK(id.empty(), "task id should be empty");
    CHECK(state.tasks.empty(), "no task should be created");
    PASS();
}

void test_assign_pending_respects_max_slots() {
    TEST(assign_pending_respects_max_slots);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::setMaxSlots(&state, 2);
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::enqueueTask(&state, "b");
    (void)AgentTaskSlots::enqueueTask(&state, "c");
    int assigned = AgentTaskSlots::assignPendingToSlots(&state);
    CHECK(assigned == 2, "assigned count should be 2");
    CHECK(AgentTaskSlots::runningCount(state) == 2, "running count mismatch");
    CHECK(AgentTaskSlots::pendingCount(state) == 1, "pending count mismatch");
    PASS();
}

void test_assign_pending_uses_distinct_slot_indices() {
    TEST(assign_pending_uses_distinct_slot_indices);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::setMaxSlots(&state, 2);
    (void)AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::enqueueTask(&state, "b");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    CHECK(state.tasks[0].slotIndex != state.tasks[1].slotIndex, "slot indices should differ");
    PASS();
}

void test_complete_task_transitions_running_to_completed() {
    TEST(complete_task_transitions_running_to_completed);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    CHECK(AgentTaskSlots::completeTask(&state, id), "complete should succeed");
    CHECK(AgentTaskSlots::completedCount(state) == 1, "completed count mismatch");
    PASS();
}

void test_complete_task_rejects_non_running_task() {
    TEST(complete_task_rejects_non_running_task);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    CHECK(!AgentTaskSlots::completeTask(&state, id), "complete should fail for pending task");
    PASS();
}

void test_cancel_task_marks_task_cancelled() {
    TEST(cancel_task_marks_task_cancelled);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    CHECK(AgentTaskSlots::cancelTask(&state, id), "cancel should succeed");
    CHECK(state.tasks[0].status == AgentTaskStatus::Cancelled, "status should be cancelled");
    PASS();
}

void test_cancel_task_clears_slot_assignment() {
    TEST(cancel_task_clears_slot_assignment);
    AgentTaskSlotsState state;
    std::string id = AgentTaskSlots::enqueueTask(&state, "a");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    CHECK(AgentTaskSlots::cancelTask(&state, id), "cancel should succeed");
    CHECK(state.tasks[0].slotIndex == -1, "slot index should be reset");
    PASS();
}

void test_assign_after_completion_picks_next_pending() {
    TEST(assign_after_completion_picks_next_pending);
    AgentTaskSlotsState state;
    (void)AgentTaskSlots::setMaxSlots(&state, 1);
    std::string id1 = AgentTaskSlots::enqueueTask(&state, "a");
    std::string id2 = AgentTaskSlots::enqueueTask(&state, "b");
    (void)AgentTaskSlots::assignPendingToSlots(&state);
    (void)AgentTaskSlots::completeTask(&state, id1);
    int assigned = AgentTaskSlots::assignPendingToSlots(&state);
    CHECK(assigned == 1, "one pending task should be assigned");
    CHECK(state.tasks[1].status == AgentTaskStatus::Running, "second task should be running");
    CHECK(state.tasks[1].taskId == id2, "task id mismatch");
    PASS();
}

int main() {
    std::cout << "Step 631: Background task slots\n";

    test_default_max_slots_is_two();                    // 1
    test_set_max_slots_rejects_invalid_values();        // 2
    test_set_max_slots_applies_valid_value();           // 3
    test_enqueue_task_creates_pending_task();           // 4
    test_enqueue_task_rejects_empty_description();      // 5
    test_assign_pending_respects_max_slots();           // 6
    test_assign_pending_uses_distinct_slot_indices();   // 7
    test_complete_task_transitions_running_to_completed();// 8
    test_complete_task_rejects_non_running_task();      // 9
    test_cancel_task_marks_task_cancelled();            // 10
    test_cancel_task_clears_slot_assignment();          // 11
    test_assign_after_completion_picks_next_pending();  // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
