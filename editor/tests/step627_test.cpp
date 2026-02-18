// Step 627: Inline accept/reject controls (12 tests)

#include "AgentMutationApproval.h"

#include <iostream>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_ensure_record_creates_pending_entry() {
    TEST(ensure_record_creates_pending_entry);
    std::vector<MutationApprovalRecord> records;
    AgentMutationApproval::ensureRecord(&records, "p1");
    CHECK(records.size() == 1, "record should be created");
    CHECK(records[0].decision == MutationDecision::Pending, "initial decision should be pending");
    PASS();
}

void test_ensure_record_is_idempotent() {
    TEST(ensure_record_is_idempotent);
    std::vector<MutationApprovalRecord> records;
    AgentMutationApproval::ensureRecord(&records, "p1");
    AgentMutationApproval::ensureRecord(&records, "p1");
    CHECK(records.size() == 1, "record should not duplicate");
    PASS();
}

void test_accept_sets_decision_accepted() {
    TEST(accept_sets_decision_accepted);
    std::vector<MutationApprovalRecord> records;
    CHECK(AgentMutationApproval::accept(&records, "p1"), "accept should succeed");
    auto* record = AgentMutationApproval::find(records, "p1");
    CHECK(record && record->decision == MutationDecision::Accepted, "decision mismatch");
    PASS();
}

void test_reject_requires_reason() {
    TEST(reject_requires_reason);
    std::vector<MutationApprovalRecord> records;
    CHECK(!AgentMutationApproval::reject(&records, "p1", ""), "reject should fail without reason");
    PASS();
}

void test_reject_sets_decision_and_reason() {
    TEST(reject_sets_decision_and_reason);
    std::vector<MutationApprovalRecord> records;
    CHECK(AgentMutationApproval::reject(&records, "p1", "unsafe mutation"), "reject should succeed");
    auto* record = AgentMutationApproval::find(records, "p1");
    CHECK(record && record->decision == MutationDecision::Rejected, "decision mismatch");
    CHECK(record && record->reason == "unsafe mutation", "reason mismatch");
    PASS();
}

void test_modify_requires_payload() {
    TEST(modify_requires_payload);
    std::vector<MutationApprovalRecord> records;
    CHECK(!AgentMutationApproval::modify(&records, "p1", ""), "modify should fail without payload");
    PASS();
}

void test_modify_sets_decision_and_payload() {
    TEST(modify_sets_decision_and_payload);
    std::vector<MutationApprovalRecord> records;
    CHECK(AgentMutationApproval::modify(&records, "p1", "{\"type\":\"setProperty\"}"),
          "modify should succeed");
    auto* record = AgentMutationApproval::find(records, "p1");
    CHECK(record && record->decision == MutationDecision::Modified, "decision mismatch");
    CHECK(record && record->modifiedMutationJson.find("setProperty") != std::string::npos,
          "modified payload mismatch");
    PASS();
}

void test_accept_clears_rejection_reason() {
    TEST(accept_clears_rejection_reason);
    std::vector<MutationApprovalRecord> records;
    (void)AgentMutationApproval::reject(&records, "p1", "need review");
    (void)AgentMutationApproval::accept(&records, "p1");
    auto* record = AgentMutationApproval::find(records, "p1");
    CHECK(record && record->reason.empty(), "reason should be cleared on accept");
    PASS();
}

void test_decision_text_mapping_pending() {
    TEST(decision_text_mapping_pending);
    CHECK(AgentMutationApproval::decisionText(MutationDecision::Pending) == "pending",
          "pending label mismatch");
    PASS();
}

void test_decision_text_mapping_modified() {
    TEST(decision_text_mapping_modified);
    CHECK(AgentMutationApproval::decisionText(MutationDecision::Modified) == "modified",
          "modified label mismatch");
    PASS();
}

void test_rejection_message_format() {
    TEST(rejection_message_format);
    MutationApprovalRecord record;
    record.previewId = "p1";
    record.decision = MutationDecision::Rejected;
    record.reason = "invalid args";
    CHECK(AgentMutationApproval::rejectionMessage(record) == "rejected: invalid args",
          "rejection message mismatch");
    PASS();
}

void test_find_returns_null_for_missing_preview() {
    TEST(find_returns_null_for_missing_preview);
    std::vector<MutationApprovalRecord> records;
    CHECK(AgentMutationApproval::find(records, "missing") == nullptr, "find should return null");
    PASS();
}

int main() {
    std::cout << "Step 627: Inline accept/reject controls\n";

    test_ensure_record_creates_pending_entry();  // 1
    test_ensure_record_is_idempotent();          // 2
    test_accept_sets_decision_accepted();        // 3
    test_reject_requires_reason();               // 4
    test_reject_sets_decision_and_reason();      // 5
    test_modify_requires_payload();              // 6
    test_modify_sets_decision_and_payload();     // 7
    test_accept_clears_rejection_reason();       // 8
    test_decision_text_mapping_pending();        // 9
    test_decision_text_mapping_modified();       // 10
    test_rejection_message_format();             // 11
    test_find_returns_null_for_missing_preview();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
