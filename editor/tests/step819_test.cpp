// Step 819: Porting review board model (12 tests)
#include "governance/PortingReviewBoard.h"
#include <iostream>
#include <string>

static int passed = 0;
static int failed = 0;
#define T(name) { std::cout << "  " << #name << "... "; }
#define P() { std::cout << "PASS\n"; ++passed; }
#define F(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define C(cond, msg) if (!(cond)) { F(msg); return; }

static PortingReviewIssue makeIssue(const std::string& id,
                                    PortingReviewSeverity severity = PortingReviewSeverity::Medium) {
    PortingReviewIssue issue;
    issue.issueId = id;
    issue.description = "Verify porting review policy for " + id;
    issue.severity = severity;
    issue.suggestedFix = "Document rationale for catching " + id;
    return issue;
}

void t1() {
    T(add_issue_success);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-01"), &error), "add");
    C(board.summarize().pending == 1, "pending");
    P();
}

void t2() {
    T(prevent_duplicate_issue);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-02"), &error), "first_add");
    C(!board.addIssue(makeIssue("ISS-02"), &error) && error == "issue_duplicate", "duplicate");
    P();
}

void t3() {
    T(assign_reviewer);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-03"), &error), "add");
    C(board.assignReviewer("ISS-03", "alice", &error), "assign");
    const auto items = board.issues();
    C(!items.empty() && items[0].reviewer == "alice", "reviewer");
    P();
}

void t4() {
    T(assign_reviewer_fails_without_name);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-04"), &error), "add");
    C(!board.assignReviewer("ISS-04", "", &error) && error == "reviewer_missing", "missing");
    P();
}

void t5() {
    T(record_decision);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-05"), &error), "add");
    C(board.assignReviewer("ISS-05", "bob", &error), "assign");
    C(board.recordDecision("ISS-05", PortingReviewVerdict::Approved, "looks good", &error), "decision");
    C(board.summarize().approved == 1, "approved");
    P();
}

void t6() {
    T(record_decision_requires_reviewer);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-06"), &error), "add");
    C(!board.recordDecision("ISS-06", PortingReviewVerdict::Rejected, "skip", &error) && error == "reviewer_not_assigned", "no_review");
    P();
}

void t7() {
    T(summary_counts);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-07"), &error), "add");
    C(board.addIssue(makeIssue("ISS-08", PortingReviewSeverity::High), &error), "add2");
    C(board.assignReviewer("ISS-07", "carol", &error), "assign1");
    C(board.assignReviewer("ISS-08", "dennis", &error), "assign2");
    C(board.recordDecision("ISS-07", PortingReviewVerdict::Modified, "tweak", &error), "decision1");
    auto summary = board.summarize();
    C(summary.modified == 1 && summary.pending == 1 && summary.highSeverityPending, "counts");
    P();
}

void t8() {
    T(all_decided_flag);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-09"), &error), "add");
    C(board.assignReviewer("ISS-09", "ellen", &error), "assign");
    C(board.recordDecision("ISS-09", PortingReviewVerdict::Rejected, "not acceptable", &error), "decision");
    C(board.summarize().allDecided, "decided");
    P();
}

void t9() {
    T(issue_order_preserved);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-10"), &error), "add1");
    C(board.addIssue(makeIssue("ISS-11"), &error), "add2");
    const auto issues = board.issues();
    C(issues.size() == 2 && issues[0].issueId == "ISS-10" && issues[1].issueId == "ISS-11", "order");
    P();
}

void t10() {
    T(cannot_add_missing_description);
    PortingReviewBoard board;
    PortingReviewIssue issue;
    issue.issueId = "ISS-12";
    std::string error;
    C(!board.addIssue(issue, &error) && error == "description_missing", "desc");
    P();
}

void t11() {
    T(update_decision);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-13"), &error), "add");
    C(board.assignReviewer("ISS-13", "frank", &error), "assign");
    C(board.recordDecision("ISS-13", PortingReviewVerdict::Approved, "ok", &error), "first");
    C(board.recordDecision("ISS-13", PortingReviewVerdict::Modified, "tweak", &error), "second");
    const auto issues = board.issues();
    C(issues[0].verdict == PortingReviewVerdict::Modified, "updated");
    P();
}

void t12() {
    T(high_severity_waits);
    PortingReviewBoard board;
    std::string error;
    C(board.addIssue(makeIssue("ISS-14", PortingReviewSeverity::High), &error), "add");
    auto summary = board.summarize();
    C(summary.highSeverityPending, "pending_high");
    P();
}

int main() {
    std::cout << "Step 819: Porting review board model\n";
    t1();
    t2();
    t3();
    t4();
    t5();
    t6();
    t7();
    t8();
    t9();
    t10();
    t11();
    t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
