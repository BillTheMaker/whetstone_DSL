// Step 577: Architect Review Surface for Intake (12 tests)

#include "ArchitectReviewSurface.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static NormalizedRequirement c(const std::string& id, const std::string& text) {
    NormalizedRequirement r;
    r.requirementId = id;
    r.kind = NormalizedRequirementKind::Constraint;
    r.normalizedText = text;
    r.anchor = "constraints";
    r.sourceLine = 1;
    return r;
}

void test_load_constraints_success() {
    TEST(load_constraints_success);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load should succeed");
    CHECK(review.items().size() == 1, "one review item expected");
    PASS();
}

void test_load_constraints_rejects_empty_constraint_set() {
    TEST(load_constraints_rejects_empty_constraint_set);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(!review.loadConstraints({}, &error), "load should fail");
    CHECK(error == "no_constraints_to_review", "wrong error");
    PASS();
}

void test_approve_sets_decision_and_reviewer() {
    TEST(approve_sets_decision_and_reviewer);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load failed");
    CHECK(review.approve("constraint-1", "architect-a", &error), "approve failed");
    const auto items = review.items();
    CHECK(items[0].decision == ReviewDecision::Approved, "decision mismatch");
    CHECK(items[0].reviewer == "architect-a", "reviewer mismatch");
    PASS();
}

void test_modify_sets_reviewed_text() {
    TEST(modify_sets_reviewed_text);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load failed");
    CHECK(review.modify("constraint-1", "must be deterministic and offline", "architect-a", &error), "modify failed");
    const auto items = review.items();
    CHECK(items[0].decision == ReviewDecision::Modified, "decision mismatch");
    CHECK(items[0].reviewedText == "must be deterministic and offline", "reviewed text mismatch");
    PASS();
}

void test_reject_sets_decision() {
    TEST(reject_sets_decision);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load failed");
    CHECK(review.reject("constraint-1", "architect-a", &error), "reject failed");
    const auto items = review.items();
    CHECK(items[0].decision == ReviewDecision::Rejected, "decision mismatch");
    PASS();
}

void test_approve_fails_for_unknown_requirement() {
    TEST(approve_fails_for_unknown_requirement);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load failed");
    CHECK(!review.approve("constraint-404", "architect-a", &error), "approve should fail");
    CHECK(error == "constraint_not_found", "wrong error");
    PASS();
}

void test_modify_fails_for_missing_reviewed_text() {
    TEST(modify_fails_for_missing_reviewed_text);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load failed");
    CHECK(!review.modify("constraint-1", "", "architect-a", &error), "modify should fail");
    CHECK(error == "reviewed_text_missing", "wrong error");
    PASS();
}

void test_decisions_require_reviewer() {
    TEST(decisions_require_reviewer);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "must be deterministic")}, &error), "load failed");
    CHECK(!review.approve("constraint-1", "", &error), "approve should fail");
    CHECK(error == "reviewer_missing", "wrong approve error");
    CHECK(!review.reject("constraint-1", "", &error), "reject should fail");
    CHECK(error == "reviewer_missing", "wrong reject error");
    PASS();
}

void test_summary_counts_pending_and_decided() {
    TEST(summary_counts_pending_and_decided);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "a"), c("constraint-2", "b")}, &error), "load failed");
    CHECK(review.approve("constraint-1", "architect-a", &error), "approve failed");
    const auto summary = review.summarize();
    CHECK(summary.approved == 1, "approved count mismatch");
    CHECK(summary.pending == 1, "pending count mismatch");
    CHECK(!summary.allDecided, "allDecided should be false");
    PASS();
}

void test_summary_all_decided_when_no_pending() {
    TEST(summary_all_decided_when_no_pending);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "a"), c("constraint-2", "b")}, &error), "load failed");
    CHECK(review.approve("constraint-1", "architect-a", &error), "approve failed");
    CHECK(review.modify("constraint-2", "b reviewed", "architect-a", &error), "modify failed");
    const auto summary = review.summarize();
    CHECK(summary.pending == 0, "pending should be zero");
    CHECK(summary.allDecided, "allDecided should be true");
    PASS();
}

void test_modify_overrides_prior_approval() {
    TEST(modify_overrides_prior_approval);
    ArchitectReviewSurface review;
    std::string error;
    CHECK(review.loadConstraints({c("constraint-1", "a")}, &error), "load failed");
    CHECK(review.approve("constraint-1", "architect-a", &error), "approve failed");
    CHECK(review.modify("constraint-1", "a updated", "architect-b", &error), "modify failed");
    const auto items = review.items();
    CHECK(items[0].decision == ReviewDecision::Modified, "decision should be modified");
    CHECK(items[0].reviewer == "architect-b", "reviewer should update");
    PASS();
}

void test_load_ignores_non_constraint_requirements() {
    TEST(load_ignores_non_constraint_requirements);
    ArchitectReviewSurface review;
    std::string error;
    NormalizedRequirement goal;
    goal.requirementId = "goal-1";
    goal.kind = NormalizedRequirementKind::Goal;
    goal.normalizedText = "goal";
    CHECK(!review.loadConstraints({goal}, &error), "load should fail because no constraints");
    CHECK(error == "no_constraints_to_review", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 577: Architect Review Surface for Intake\n";

    test_load_constraints_success();                    // 1
    test_load_constraints_rejects_empty_constraint_set();// 2
    test_approve_sets_decision_and_reviewer();         // 3
    test_modify_sets_reviewed_text();                  // 4
    test_reject_sets_decision();                       // 5
    test_approve_fails_for_unknown_requirement();      // 6
    test_modify_fails_for_missing_reviewed_text();     // 7
    test_decisions_require_reviewer();                 // 8
    test_summary_counts_pending_and_decided();         // 9
    test_summary_all_decided_when_no_pending();        // 10
    test_modify_overrides_prior_approval();            // 11
    test_load_ignores_non_constraint_requirements();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
