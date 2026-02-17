// Step 606: Rollback Drill Tracker (12 tests)

#include "RollbackDrillTracker.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static RollbackDrillRecord drill(const std::string& id,
                                 const std::string& env) {
    return {id, env, 0, DrillOutcome::Scheduled, ""};
}

void test_schedule_success() {
    TEST(schedule_success);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule should succeed");
    CHECK(tracker.byEnvironment("prod").size() == 1, "env count mismatch");
    PASS();
}

void test_schedule_rejects_missing_drill_id() {
    TEST(schedule_rejects_missing_drill_id);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(!tracker.schedule(drill("", "prod"), &error), "schedule should fail");
    CHECK(error == "drill_id_missing", "wrong error");
    PASS();
}

void test_schedule_rejects_missing_environment_id() {
    TEST(schedule_rejects_missing_environment_id);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(!tracker.schedule(drill("d1", ""), &error), "schedule should fail");
    CHECK(error == "environment_id_missing", "wrong error");
    PASS();
}

void test_schedule_rejects_duplicate() {
    TEST(schedule_rejects_duplicate);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "first schedule failed");
    CHECK(!tracker.schedule(drill("d1", "staging"), &error), "duplicate should fail");
    CHECK(error == "drill_duplicate", "wrong error");
    PASS();
}

void test_complete_pass_updates_counts() {
    TEST(complete_pass_updates_counts);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule failed");
    CHECK(tracker.complete("d1", DrillOutcome::Passed, 7, "ok", &error), "complete failed");
    CHECK(tracker.passCount() == 1, "pass count mismatch");
    CHECK(tracker.failCount() == 0, "fail count mismatch");
    PASS();
}

void test_complete_fail_updates_counts() {
    TEST(complete_fail_updates_counts);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule failed");
    CHECK(tracker.complete("d1", DrillOutcome::Failed, 15, "issue", &error), "complete failed");
    CHECK(tracker.failCount() == 1, "fail count mismatch");
    PASS();
}

void test_complete_rejects_missing_drill() {
    TEST(complete_rejects_missing_drill);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(!tracker.complete("missing", DrillOutcome::Passed, 5, "", &error), "complete should fail");
    CHECK(error == "drill_missing", "wrong error");
    PASS();
}

void test_complete_rejects_scheduled_outcome() {
    TEST(complete_rejects_scheduled_outcome);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule failed");
    CHECK(!tracker.complete("d1", DrillOutcome::Scheduled, 5, "", &error), "complete should fail");
    CHECK(error == "outcome_invalid", "wrong error");
    PASS();
}

void test_complete_rejects_negative_recovery_minutes() {
    TEST(complete_rejects_negative_recovery_minutes);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule failed");
    CHECK(!tracker.complete("d1", DrillOutcome::Passed, -1, "", &error), "complete should fail");
    CHECK(error == "recovery_minutes_invalid", "wrong error");
    PASS();
}

void test_average_recovery_minutes_zero_when_no_completed_drills() {
    TEST(average_recovery_minutes_zero_when_no_completed_drills);
    RollbackDrillTracker tracker;
    CHECK(tracker.averageRecoveryMinutes() == 0, "average should be zero");
    PASS();
}

void test_average_recovery_minutes_computes_completed_only() {
    TEST(average_recovery_minutes_computes_completed_only);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule d1 failed");
    CHECK(tracker.schedule(drill("d2", "prod"), &error), "schedule d2 failed");
    CHECK(tracker.schedule(drill("d3", "staging"), &error), "schedule d3 failed");
    CHECK(tracker.complete("d1", DrillOutcome::Passed, 10, "", &error), "complete d1 failed");
    CHECK(tracker.complete("d2", DrillOutcome::Failed, 20, "", &error), "complete d2 failed");
    CHECK(tracker.averageRecoveryMinutes() == 15, "average mismatch");
    PASS();
}

void test_by_environment_empty_returns_all() {
    TEST(by_environment_empty_returns_all);
    RollbackDrillTracker tracker;
    std::string error;
    CHECK(tracker.schedule(drill("d1", "prod"), &error), "schedule d1 failed");
    CHECK(tracker.schedule(drill("d2", "staging"), &error), "schedule d2 failed");
    CHECK(tracker.byEnvironment("").size() == 2, "empty env should return all");
    PASS();
}

int main() {
    std::cout << "Step 606: Rollback Drill Tracker\n";

    test_schedule_success();                                    // 1
    test_schedule_rejects_missing_drill_id();                  // 2
    test_schedule_rejects_missing_environment_id();            // 3
    test_schedule_rejects_duplicate();                         // 4
    test_complete_pass_updates_counts();                       // 5
    test_complete_fail_updates_counts();                       // 6
    test_complete_rejects_missing_drill();                     // 7
    test_complete_rejects_scheduled_outcome();                 // 8
    test_complete_rejects_negative_recovery_minutes();         // 9
    test_average_recovery_minutes_zero_when_no_completed_drills(); // 10
    test_average_recovery_minutes_computes_completed_only();   // 11
    test_by_environment_empty_returns_all();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
