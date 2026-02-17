// Step 608: Incident Postmortem Ledger (12 tests)

#include "IncidentPostmortemLedger.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static PostmortemRecord record(const std::string& id,
                               const std::string& owner,
                               int total,
                               int done = 0) {
    return {id, owner, total, done, PostmortemStatus::Open};
}

void test_open_success() {
    TEST(open_success);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.open(record("inc-1", "ops", 3), &error), "open should succeed");
    CHECK(ledger.openCount() == 1, "open count mismatch");
    PASS();
}

void test_open_rejects_missing_incident_id() {
    TEST(open_rejects_missing_incident_id);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(!ledger.open(record("", "ops", 3), &error), "open should fail");
    CHECK(error == "incident_id_missing", "wrong error");
    PASS();
}

void test_open_rejects_missing_owner() {
    TEST(open_rejects_missing_owner);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(!ledger.open(record("inc-1", "", 3), &error), "open should fail");
    CHECK(error == "owner_missing", "wrong error");
    PASS();
}

void test_open_rejects_invalid_corrective_actions() {
    TEST(open_rejects_invalid_corrective_actions);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(!ledger.open(record("inc-1", "ops", -1), &error), "open should fail");
    CHECK(error == "corrective_actions_invalid", "wrong error");
    PASS();
}

void test_open_rejects_completed_actions_exceed_total() {
    TEST(open_rejects_completed_actions_exceed_total);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(!ledger.open(record("inc-1", "ops", 2, 3), &error), "open should fail");
    CHECK(error == "completed_actions_exceed_total", "wrong error");
    PASS();
}

void test_open_rejects_duplicate() {
    TEST(open_rejects_duplicate);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.open(record("inc-1", "ops", 3), &error), "first open failed");
    CHECK(!ledger.open(record("inc-1", "ops", 3), &error), "duplicate should fail");
    CHECK(error == "incident_duplicate", "wrong error");
    PASS();
}

void test_update_actions_success_changes_percent() {
    TEST(update_actions_success_changes_percent);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.open(record("inc-1", "ops", 4), &error), "open failed");
    CHECK(ledger.updateActions("inc-1", 2, &error), "update failed");
    CHECK(ledger.completionPercent("inc-1", &error) == 50, "percent mismatch");
    PASS();
}

void test_update_actions_marks_completed_when_done() {
    TEST(update_actions_marks_completed_when_done);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.open(record("inc-1", "ops", 2), &error), "open failed");
    CHECK(ledger.updateActions("inc-1", 2, &error), "update failed");
    CHECK(ledger.completedCount() == 1, "completed count mismatch");
    CHECK(ledger.openCount() == 0, "open count mismatch");
    PASS();
}

void test_update_actions_rejects_missing_incident() {
    TEST(update_actions_rejects_missing_incident);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(!ledger.updateActions("missing", 1, &error), "update should fail");
    CHECK(error == "incident_missing", "wrong error");
    PASS();
}

void test_update_actions_rejects_invalid_completed_actions() {
    TEST(update_actions_rejects_invalid_completed_actions);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.open(record("inc-1", "ops", 2), &error), "open failed");
    CHECK(!ledger.updateActions("inc-1", 3, &error), "update should fail");
    CHECK(error == "completed_actions_exceed_total", "wrong error");
    PASS();
}

void test_completion_percent_returns_100_when_no_actions() {
    TEST(completion_percent_returns_100_when_no_actions);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.open(record("inc-1", "ops", 0), &error), "open failed");
    CHECK(ledger.completionPercent("inc-1", &error) == 100, "zero-action incident should be complete");
    PASS();
}

void test_completion_percent_rejects_missing_incident() {
    TEST(completion_percent_rejects_missing_incident);
    IncidentPostmortemLedger ledger;
    std::string error;
    CHECK(ledger.completionPercent("missing", &error) == 0, "missing incident should return zero");
    CHECK(error == "incident_missing", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 608: Incident Postmortem Ledger\n";

    test_open_success();                                    // 1
    test_open_rejects_missing_incident_id();               // 2
    test_open_rejects_missing_owner();                     // 3
    test_open_rejects_invalid_corrective_actions();        // 4
    test_open_rejects_completed_actions_exceed_total();    // 5
    test_open_rejects_duplicate();                         // 6
    test_update_actions_success_changes_percent();         // 7
    test_update_actions_marks_completed_when_done();       // 8
    test_update_actions_rejects_missing_incident();        // 9
    test_update_actions_rejects_invalid_completed_actions();// 10
    test_completion_percent_returns_100_when_no_actions(); // 11
    test_completion_percent_rejects_missing_incident();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
