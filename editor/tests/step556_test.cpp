// Step 556: Step Engine (12 tests)

#include "StepEngine.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasDiag(const StepResult& r, const std::string& d) {
    for (const auto& x : r.diagnostics) if (x == d) return true;
    return false;
}

void test_step_over_advances_instruction_pointer() {
    TEST(step_over_advances_instruction_pointer);
    StepEngine e;
    auto before = e.state().instructionPointer;
    auto r = e.execute("step_over");
    CHECK(r.ok, "step_over should succeed");
    CHECK(r.state.instructionPointer == before + 1, "ip should advance by one");
    PASS();
}

void test_step_into_advances_ip_and_stack_depth() {
    TEST(step_into_advances_ip_and_stack_depth);
    StepEngine e;
    auto r = e.execute("step_into");
    CHECK(r.ok, "step_into should succeed");
    CHECK(r.state.instructionPointer == 1, "ip should advance");
    CHECK(r.state.stackDepth == 2, "stack depth should increase");
    PASS();
}

void test_step_out_reduces_stack_depth_when_nested() {
    TEST(step_out_reduces_stack_depth_when_nested);
    StepEngine e;
    CHECK(e.execute("step_into").ok, "step_into should nest");
    auto r = e.execute("step_out");
    CHECK(r.ok, "step_out should succeed");
    CHECK(r.state.stackDepth == 1, "stack should reduce");
    PASS();
}

void test_step_out_from_root_completes_session() {
    TEST(step_out_from_root_completes_session);
    StepEngine e;
    auto r = e.execute("step_out");
    CHECK(r.ok, "step_out root should succeed");
    CHECK(r.state.runState == DebugRunState::Completed, "session should complete");
    CHECK(hasDiag(r, "session_completed"), "completion diag expected");
    PASS();
}

void test_continue_runs_and_pauses_deterministically() {
    TEST(continue_runs_and_pauses_deterministically);
    StepEngine e;
    auto before = e.state().instructionPointer;
    auto r = e.execute("continue");
    CHECK(r.ok, "continue should succeed");
    CHECK(r.state.runState == DebugRunState::Paused, "engine should return to paused state");
    CHECK(r.state.instructionPointer == before + 5, "continue should advance by deterministic burst");
    PASS();
}

void test_unknown_command_fails() {
    TEST(unknown_command_fails);
    StepEngine e;
    auto r = e.execute("teleport");
    CHECK(!r.ok, "unknown command should fail");
    CHECK(hasDiag(r, "unknown_command"), "unknown command diag expected");
    PASS();
}

void test_pause_keeps_engine_paused() {
    TEST(pause_keeps_engine_paused);
    StepEngine e;
    CHECK(e.pause(), "pause should succeed");
    CHECK(e.state().runState == DebugRunState::Paused, "engine should remain paused");
    PASS();
}

void test_commands_fail_after_completion() {
    TEST(commands_fail_after_completion);
    StepEngine e;
    CHECK(e.execute("step_out").ok, "root step_out completes session");
    auto r = e.execute("step_over");
    CHECK(!r.ok, "commands should fail after completion");
    CHECK(hasDiag(r, "session_completed"), "completion diag expected");
    PASS();
}

void test_last_command_tracks_step_actions() {
    TEST(last_command_tracks_step_actions);
    StepEngine e;
    CHECK(e.execute("step_into").ok, "step_into should succeed");
    CHECK(e.state().lastCommand == "step_into", "last command mismatch");
    CHECK(e.execute("step_over").ok, "step_over should succeed");
    CHECK(e.state().lastCommand == "step_over", "last command mismatch");
    PASS();
}

void test_continue_updates_last_command() {
    TEST(continue_updates_last_command);
    StepEngine e;
    CHECK(e.execute("continue").ok, "continue should succeed");
    CHECK(e.state().lastCommand == "continue", "last command should be continue");
    PASS();
}

void test_multiple_step_into_accumulates_stack_depth() {
    TEST(multiple_step_into_accumulates_stack_depth);
    StepEngine e;
    CHECK(e.execute("step_into").ok, "step_into #1");
    CHECK(e.execute("step_into").ok, "step_into #2");
    CHECK(e.state().stackDepth == 3, "stack depth should accumulate");
    PASS();
}

void test_instruction_pointer_progression_is_monotonic() {
    TEST(instruction_pointer_progression_is_monotonic);
    StepEngine e;
    CHECK(e.execute("step_over").ok, "step_over");
    int a = e.state().instructionPointer;
    CHECK(e.execute("step_into").ok, "step_into");
    int b = e.state().instructionPointer;
    CHECK(e.execute("continue").ok, "continue");
    int c = e.state().instructionPointer;
    CHECK(a < b && b < c, "ip progression should be monotonic");
    PASS();
}

int main() {
    std::cout << "Step 556: Step Engine\n";

    test_step_over_advances_instruction_pointer();          // 1
    test_step_into_advances_ip_and_stack_depth();           // 2
    test_step_out_reduces_stack_depth_when_nested();        // 3
    test_step_out_from_root_completes_session();            // 4
    test_continue_runs_and_pauses_deterministically();      // 5
    test_unknown_command_fails();                           // 6
    test_pause_keeps_engine_paused();                       // 7
    test_commands_fail_after_completion();                  // 8
    test_last_command_tracks_step_actions();                // 9
    test_continue_updates_last_command();                   // 10
    test_multiple_step_into_accumulates_stack_depth();      // 11
    test_instruction_pointer_progression_is_monotonic();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
