// Step 436: Review Comparison View Tests (12 tests)

#include "ReviewComparisonView.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkflowState sampleReviewState() {
    WorkflowState wf("review");
    WorkItem wi;
    wi.id = "r1";
    wi.nodeId = "node-review";
    wi.nodeName = "computePath";
    wi.bufferId = "src/path.cpp";
    wi.status = WI_REVIEW;
    wi.workerType = "llm";
    wi.priority = "high";
    wi.contextWidth = "file";
    wi.reviewRequired = true;
    wi.result.generatedCode = "void computePath() {\n  int x = 1;\n}\n";
    wi.rejectionFeedback = "add bounds check";
    wf.queue.enqueue(wi);
    return wf;
}

void test_build_returns_model_for_existing_item() {
    TEST(build_returns_model_for_existing_item);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "model should exist");
    CHECK(model->itemId == "r1", "item id mismatch");
    PASS();
}

void test_build_returns_null_for_missing_item() {
    TEST(build_returns_null_for_missing_item);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "missing");
    CHECK(!model.has_value(), "missing item should return nullopt");
    PASS();
}

void test_diff_contains_modified_line_entries() {
    TEST(diff_contains_modified_line_entries);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "expected model");
    bool hasModified = false;
    for (const auto& d : model->diff) if (d.kind == '~') hasModified = true;
    CHECK(hasModified, "diff should contain modified line");
    PASS();
}

void test_review_status_enables_approve_and_reject() {
    TEST(review_status_enables_approve_and_reject);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "expected model");
    CHECK(model->canApprove, "review item should allow approve");
    CHECK(model->canReject, "review item should allow reject");
    PASS();
}

void test_shortcuts_match_spec() {
    TEST(shortcuts_match_spec);
    CHECK(ReviewComparisonView::approveShortcut() == "Ctrl+Shift+A", "approve shortcut mismatch");
    CHECK(ReviewComparisonView::rejectShortcut() == "Ctrl+Shift+R", "reject shortcut mismatch");
    PASS();
}

void test_model_includes_shortcuts() {
    TEST(model_includes_shortcuts);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "expected model");
    CHECK(model->approveShortcut == "Ctrl+Shift+A", "approve shortcut missing");
    CHECK(model->rejectShortcut == "Ctrl+Shift+R", "reject shortcut missing");
    PASS();
}

void test_change_notes_include_routing() {
    TEST(change_notes_include_routing);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "expected model");
    bool hasRouting = false;
    for (const auto& n : model->changeNotes) if (n.find("routing:") == 0) hasRouting = true;
    CHECK(hasRouting, "routing note expected");
    PASS();
}

void test_change_notes_include_rejection_feedback() {
    TEST(change_notes_include_rejection_feedback);
    auto wf = sampleReviewState();
    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "expected model");
    bool hasFeedback = false;
    for (const auto& n : model->changeNotes) if (n.find("latest-feedback:") == 0) hasFeedback = true;
    CHECK(hasFeedback, "feedback note expected");
    PASS();
}

void test_approve_transitions_item_to_complete() {
    TEST(approve_transitions_item_to_complete);
    auto wf = sampleReviewState();
    bool ok = ReviewComparisonView::approve(wf, "r1", "architect");
    CHECK(ok, "approve should succeed");
    auto updated = wf.queue.getItem("r1");
    CHECK(updated.has_value(), "item should exist");
    CHECK(updated->status == WI_COMPLETE, "status should be complete");
    PASS();
}

void test_reject_transitions_item_to_ready() {
    TEST(reject_transitions_item_to_ready);
    auto wf = sampleReviewState();
    bool ok = ReviewComparisonView::reject(wf, "r1", "needs tests", "architect");
    CHECK(ok, "reject should succeed");
    auto updated = wf.queue.getItem("r1");
    CHECK(updated.has_value(), "item should exist");
    CHECK(updated->status == WI_READY, "status should be ready after rejection");
    PASS();
}

void test_reject_requires_feedback() {
    TEST(reject_requires_feedback);
    auto wf = sampleReviewState();
    bool ok = ReviewComparisonView::reject(wf, "r1", "", "architect");
    CHECK(!ok, "reject should fail without feedback");
    PASS();
}

void test_non_review_item_disables_actions() {
    TEST(non_review_item_disables_actions);
    auto wf = sampleReviewState();
    auto item = wf.queue.getItem("r1");
    CHECK(item.has_value(), "item missing");
    WorkItem updated = *item;
    updated.status = WI_COMPLETE;
    CHECK(wf.queue.updateItem("r1", updated), "update failed");

    auto model = ReviewComparisonView::build(wf, "r1");
    CHECK(model.has_value(), "expected model");
    CHECK(!model->canApprove, "approve should be disabled");
    CHECK(!model->canReject, "reject should be disabled");
    PASS();
}

int main() {
    std::cout << "Step 436: Review Comparison View Tests\n";

    test_build_returns_model_for_existing_item();         // 1
    test_build_returns_null_for_missing_item();           // 2
    test_diff_contains_modified_line_entries();           // 3
    test_review_status_enables_approve_and_reject();      // 4
    test_shortcuts_match_spec();                          // 5
    test_model_includes_shortcuts();                      // 6
    test_change_notes_include_routing();                  // 7
    test_change_notes_include_rejection_feedback();       // 8
    test_approve_transitions_item_to_complete();          // 9
    test_reject_transitions_item_to_ready();              // 10
    test_reject_requires_feedback();                      // 11
    test_non_review_item_disables_actions();              // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
