// Step 596: Escalation Audit Ledger (12 tests)

#include "EscalationAuditLedger.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static EscalationAuditEntry entry(const std::string& id,
                                  const std::string& actor,
                                  GuardrailDecision decision,
                                  int risk) {
    EscalationAuditEntry e;
    e.entryId = id;
    e.operation = "deploy prod";
    e.decision = decision;
    e.level = EscalationLevel::Reviewer;
    e.actor = actor;
    e.outcome = "approved";
    e.riskScore = risk;
    return e;
}

void test_record_success() {
    TEST(record_success);
    EscalationAuditLedger l;
    std::string error;
    CHECK(l.record(entry("e1", "alice", GuardrailDecision::Allow, 20), &error), "record should succeed");
    CHECK(l.entries().size() == 1, "entry count mismatch");
    PASS();
}

void test_record_rejects_missing_entry_id() {
    TEST(record_rejects_missing_entry_id);
    EscalationAuditLedger l;
    std::string error;
    auto e = entry("", "alice", GuardrailDecision::Allow, 20);
    CHECK(!l.record(e, &error), "record should fail");
    CHECK(error == "entry_id_missing", "wrong error");
    PASS();
}

void test_record_rejects_missing_operation() {
    TEST(record_rejects_missing_operation);
    EscalationAuditLedger l;
    std::string error;
    auto e = entry("e1", "alice", GuardrailDecision::Allow, 20);
    e.operation.clear();
    CHECK(!l.record(e, &error), "record should fail");
    CHECK(error == "operation_missing", "wrong error");
    PASS();
}

void test_record_rejects_missing_actor() {
    TEST(record_rejects_missing_actor);
    EscalationAuditLedger l;
    std::string error;
    auto e = entry("e1", "", GuardrailDecision::Allow, 20);
    CHECK(!l.record(e, &error), "record should fail");
    CHECK(error == "actor_missing", "wrong error");
    PASS();
}

void test_record_rejects_missing_outcome() {
    TEST(record_rejects_missing_outcome);
    EscalationAuditLedger l;
    std::string error;
    auto e = entry("e1", "alice", GuardrailDecision::Allow, 20);
    e.outcome.clear();
    CHECK(!l.record(e, &error), "record should fail");
    CHECK(error == "outcome_missing", "wrong error");
    PASS();
}

void test_record_rejects_invalid_risk_score() {
    TEST(record_rejects_invalid_risk_score);
    EscalationAuditLedger l;
    std::string error;
    auto e = entry("e1", "alice", GuardrailDecision::Allow, 120);
    CHECK(!l.record(e, &error), "record should fail");
    CHECK(error == "risk_score_invalid", "wrong error");
    PASS();
}

void test_record_rejects_duplicate_entry_id() {
    TEST(record_rejects_duplicate_entry_id);
    EscalationAuditLedger l;
    std::string error;
    CHECK(l.record(entry("e1", "alice", GuardrailDecision::Allow, 20), &error), "first record failed");
    CHECK(!l.record(entry("e1", "alice", GuardrailDecision::Allow, 20), &error), "duplicate should fail");
    CHECK(error == "entry_duplicate", "wrong error");
    PASS();
}

void test_entries_preserve_insertion_order() {
    TEST(entries_preserve_insertion_order);
    EscalationAuditLedger l;
    std::string error;
    CHECK(l.record(entry("e1", "alice", GuardrailDecision::Allow, 20), &error), "e1 failed");
    CHECK(l.record(entry("e2", "bob", GuardrailDecision::Allow, 25), &error), "e2 failed");
    auto all = l.entries();
    CHECK(all[0].entryId == "e1", "first order mismatch");
    CHECK(all[1].entryId == "e2", "second order mismatch");
    PASS();
}

void test_by_actor_filters_entries() {
    TEST(by_actor_filters_entries);
    EscalationAuditLedger l;
    std::string error;
    CHECK(l.record(entry("e1", "alice", GuardrailDecision::Allow, 20), &error), "e1 failed");
    CHECK(l.record(entry("e2", "bob", GuardrailDecision::Allow, 25), &error), "e2 failed");
    CHECK(l.byActor("alice").size() == 1, "actor filter mismatch");
    PASS();
}

void test_average_risk_score_computed() {
    TEST(average_risk_score_computed);
    EscalationAuditLedger l;
    std::string error;
    CHECK(l.record(entry("e1", "alice", GuardrailDecision::Allow, 20), &error), "e1 failed");
    CHECK(l.record(entry("e2", "bob", GuardrailDecision::Allow, 40), &error), "e2 failed");
    CHECK(l.averageRiskScore() == 30, "avg risk mismatch");
    PASS();
}

void test_average_risk_zero_when_empty() {
    TEST(average_risk_zero_when_empty);
    EscalationAuditLedger l;
    CHECK(l.averageRiskScore() == 0, "empty avg risk should be zero");
    PASS();
}

void test_denied_count_tracks_deny_decisions() {
    TEST(denied_count_tracks_deny_decisions);
    EscalationAuditLedger l;
    std::string error;
    CHECK(l.record(entry("e1", "alice", GuardrailDecision::Deny, 95), &error), "e1 failed");
    CHECK(l.record(entry("e2", "bob", GuardrailDecision::Allow, 20), &error), "e2 failed");
    CHECK(l.deniedCount() == 1, "denied count mismatch");
    PASS();
}

int main() {
    std::cout << "Step 596: Escalation Audit Ledger\n";

    test_record_success();                          // 1
    test_record_rejects_missing_entry_id();         // 2
    test_record_rejects_missing_operation();        // 3
    test_record_rejects_missing_actor();            // 4
    test_record_rejects_missing_outcome();          // 5
    test_record_rejects_invalid_risk_score();       // 6
    test_record_rejects_duplicate_entry_id();       // 7
    test_entries_preserve_insertion_order();        // 8
    test_by_actor_filters_entries();                // 9
    test_average_risk_score_computed();             // 10
    test_average_risk_zero_when_empty();            // 11
    test_denied_count_tracks_deny_decisions();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
