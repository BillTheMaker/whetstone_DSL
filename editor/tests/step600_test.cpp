// Step 600: Security Exception Review Board (12 tests)

#include "SecurityExceptionReviewBoard.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_submit_success() {
    TEST(submit_success);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "rm -rf /tmp/x", "alice", "urgent rollback", &error), "submit should succeed");
    CHECK(b.pending().size() == 1, "pending size mismatch");
    PASS();
}

void test_submit_rejects_missing_case_id() {
    TEST(submit_rejects_missing_case_id);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(!b.submit("", "op", "alice", "why", &error), "submit should fail");
    CHECK(error == "case_id_missing", "wrong error");
    PASS();
}

void test_submit_rejects_missing_operation() {
    TEST(submit_rejects_missing_operation);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(!b.submit("c1", "", "alice", "why", &error), "submit should fail");
    CHECK(error == "operation_missing", "wrong error");
    PASS();
}

void test_submit_rejects_missing_requester() {
    TEST(submit_rejects_missing_requester);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(!b.submit("c1", "op", "", "why", &error), "submit should fail");
    CHECK(error == "requester_missing", "wrong error");
    PASS();
}

void test_submit_rejects_missing_justification() {
    TEST(submit_rejects_missing_justification);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(!b.submit("c1", "op", "alice", "", &error), "submit should fail");
    CHECK(error == "justification_missing", "wrong error");
    PASS();
}

void test_submit_rejects_duplicate_case() {
    TEST(submit_rejects_duplicate_case);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "op", "alice", "why", &error), "submit failed");
    CHECK(!b.submit("c1", "op", "alice", "why", &error), "duplicate should fail");
    CHECK(error == "case_duplicate", "wrong error");
    PASS();
}

void test_decide_approve_success() {
    TEST(decide_approve_success);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "op", "alice", "why", &error), "submit failed");
    CHECK(b.decide("c1", ExceptionDecision::Approved, "sec-admin", &error), "decide should succeed");
    CHECK(b.approvedCount() == 1, "approved count mismatch");
    PASS();
}

void test_decide_reject_success() {
    TEST(decide_reject_success);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "op", "alice", "why", &error), "submit failed");
    CHECK(b.decide("c1", ExceptionDecision::Rejected, "sec-admin", &error), "decide should succeed");
    CHECK(b.rejectedCount() == 1, "rejected count mismatch");
    PASS();
}

void test_decide_fails_for_missing_case() {
    TEST(decide_fails_for_missing_case);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(!b.decide("missing", ExceptionDecision::Approved, "sec-admin", &error), "decide should fail");
    CHECK(error == "case_missing", "wrong error");
    PASS();
}

void test_decide_fails_for_missing_reviewer() {
    TEST(decide_fails_for_missing_reviewer);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "op", "alice", "why", &error), "submit failed");
    CHECK(!b.decide("c1", ExceptionDecision::Approved, "", &error), "decide should fail");
    CHECK(error == "reviewer_missing", "wrong error");
    PASS();
}

void test_decide_fails_for_pending_decision() {
    TEST(decide_fails_for_pending_decision);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "op", "alice", "why", &error), "submit failed");
    CHECK(!b.decide("c1", ExceptionDecision::Pending, "sec-admin", &error), "decide should fail");
    CHECK(error == "decision_invalid", "wrong error");
    PASS();
}

void test_pending_filters_closed_cases() {
    TEST(pending_filters_closed_cases);
    SecurityExceptionReviewBoard b;
    std::string error;
    CHECK(b.submit("c1", "op", "alice", "why", &error), "submit c1 failed");
    CHECK(b.submit("c2", "op", "bob", "why", &error), "submit c2 failed");
    CHECK(b.decide("c1", ExceptionDecision::Approved, "sec-admin", &error), "decide failed");
    CHECK(b.pending().size() == 1, "one pending expected");
    PASS();
}

int main() {
    std::cout << "Step 600: Security Exception Review Board\n";

    test_submit_success();                        // 1
    test_submit_rejects_missing_case_id();       // 2
    test_submit_rejects_missing_operation();     // 3
    test_submit_rejects_missing_requester();     // 4
    test_submit_rejects_missing_justification(); // 5
    test_submit_rejects_duplicate_case();        // 6
    test_decide_approve_success();               // 7
    test_decide_reject_success();                // 8
    test_decide_fails_for_missing_case();        // 9
    test_decide_fails_for_missing_reviewer();    // 10
    test_decide_fails_for_pending_decision();    // 11
    test_pending_filters_closed_cases();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
