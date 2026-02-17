// Step 599: Incident Response Runbook Engine (12 tests)

#include "IncidentResponseRunbookEngine.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Runbook runbook() {
    Runbook rb;
    rb.runbookId = "rb-1";
    rb.incidentType = "crash";
    rb.steps = {
        {"s1", "collect logs", true},
        {"s2", "restart services", true},
        {"s3", "postmortem draft", false}
    };
    return rb;
}

void test_register_runbook_success() {
    TEST(register_runbook_success);
    IncidentResponseRunbookEngine e;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register should succeed");
    PASS();
}

void test_register_runbook_rejects_missing_id() {
    TEST(register_runbook_rejects_missing_id);
    IncidentResponseRunbookEngine e;
    std::string error;
    auto rb = runbook();
    rb.runbookId.clear();
    CHECK(!e.registerRunbook(rb, &error), "register should fail");
    CHECK(error == "runbook_id_missing", "wrong error");
    PASS();
}

void test_register_runbook_rejects_missing_steps() {
    TEST(register_runbook_rejects_missing_steps);
    IncidentResponseRunbookEngine e;
    std::string error;
    auto rb = runbook();
    rb.steps.clear();
    CHECK(!e.registerRunbook(rb, &error), "register should fail");
    CHECK(error == "runbook_steps_missing", "wrong error");
    PASS();
}

void test_register_runbook_rejects_duplicate() {
    TEST(register_runbook_rejects_duplicate);
    IncidentResponseRunbookEngine e;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "first register failed");
    CHECK(!e.registerRunbook(runbook(), &error), "duplicate should fail");
    CHECK(error == "runbook_duplicate", "wrong error");
    PASS();
}

void test_start_execution_success() {
    TEST(start_execution_success);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start should succeed");
    CHECK(s.runbookId == "rb-1", "runbook id mismatch");
    PASS();
}

void test_start_execution_fails_for_missing_runbook() {
    TEST(start_execution_fails_for_missing_runbook);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(!e.start("missing", &s, &error), "start should fail");
    CHECK(error == "runbook_missing", "wrong error");
    PASS();
}

void test_complete_step_success() {
    TEST(complete_step_success);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start failed");
    CHECK(e.completeStep(&s, "s1", &error), "complete should succeed");
    CHECK(s.completedStepIds.size() == 1, "completed size mismatch");
    PASS();
}

void test_complete_step_rejects_unknown_step() {
    TEST(complete_step_rejects_unknown_step);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start failed");
    CHECK(!e.completeStep(&s, "missing", &error), "complete should fail");
    CHECK(error == "step_missing", "wrong error");
    PASS();
}

void test_complete_step_rejects_duplicate_completion() {
    TEST(complete_step_rejects_duplicate_completion);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start failed");
    CHECK(e.completeStep(&s, "s1", &error), "complete failed");
    CHECK(!e.completeStep(&s, "s1", &error), "duplicate complete should fail");
    CHECK(error == "step_already_completed", "wrong error");
    PASS();
}

void test_close_if_ready_requires_required_steps() {
    TEST(close_if_ready_requires_required_steps);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start failed");
    CHECK(!e.closeIfReady(&s, &error), "close should fail");
    CHECK(error == "required_steps_incomplete", "wrong error");
    PASS();
}

void test_close_if_ready_succeeds_after_required_steps() {
    TEST(close_if_ready_succeeds_after_required_steps);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start failed");
    CHECK(e.completeStep(&s, "s1", &error), "s1 complete failed");
    CHECK(e.completeStep(&s, "s2", &error), "s2 complete failed");
    CHECK(e.closeIfReady(&s, &error), "close should succeed");
    CHECK(s.closed, "state should be closed");
    PASS();
}

void test_completion_percent_computed_from_steps() {
    TEST(completion_percent_computed_from_steps);
    IncidentResponseRunbookEngine e;
    RunbookExecutionState s;
    std::string error;
    CHECK(e.registerRunbook(runbook(), &error), "register failed");
    CHECK(e.start("rb-1", &s, &error), "start failed");
    CHECK(e.completeStep(&s, "s1", &error), "complete failed");
    CHECK(e.completionPercent(s) == 33, "completion percent mismatch");
    PASS();
}

int main() {
    std::cout << "Step 599: Incident Response Runbook Engine\n";

    test_register_runbook_success();                 // 1
    test_register_runbook_rejects_missing_id();      // 2
    test_register_runbook_rejects_missing_steps();   // 3
    test_register_runbook_rejects_duplicate();       // 4
    test_start_execution_success();                  // 5
    test_start_execution_fails_for_missing_runbook();// 6
    test_complete_step_success();                    // 7
    test_complete_step_rejects_unknown_step();       // 8
    test_complete_step_rejects_duplicate_completion();// 9
    test_close_if_ready_requires_required_steps();   // 10
    test_close_if_ready_succeeds_after_required_steps();// 11
    test_completion_percent_computed_from_steps();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
