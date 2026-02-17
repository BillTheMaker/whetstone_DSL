// Step 429: Task Detail View Tests (12 tests)

#include "WorkflowTaskDetail.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeReviewItem(const std::string& id = "w1") {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = "n1";
    wi.nodeName = "processOrder";
    wi.bufferId = "orders.py";
    wi.workerType = "llm";
    wi.contextWidth = "file";
    wi.reviewRequired = true;
    wi.priority = "high";
    wi.status = WI_REVIEW;
    wi.result.generatedCode = "def process_order(x):\n    return x\n";
    wi.result.reasoning = "generated";
    return wi;
}

static WorkflowState makeWorkflow() {
    WorkflowState wf("detail");
    wf.queue.enqueue(makeReviewItem("w1"));
    WorkItem done = makeReviewItem("w2");
    done.status = WI_COMPLETE;
    done.rejectionFeedback = "old feedback";
    RejectionAttempt attempt;
    attempt.workerType = "slm";
    attempt.feedback = "fix naming";
    attempt.rejectedBy = "human";
    done.rejectionHistory.push_back(attempt);
    wf.queue.enqueue(done);
    return wf;
}

void test_build_detail_for_existing_item() {
    TEST(build_detail_for_existing_item);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "w1");
    CHECK(view.has_value(), "detail should exist");
    CHECK(view->itemId == "w1", "item id mismatch");
    PASS();
}

void test_build_detail_missing_item_returns_nullopt() {
    TEST(build_detail_missing_item_returns_nullopt);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "missing");
    CHECK(!view.has_value(), "missing item should return nullopt");
    PASS();
}

void test_detail_contains_skeleton_and_generated_code() {
    TEST(detail_contains_skeleton_and_generated_code);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "w1");
    CHECK(view->skeletonCode.find("def processOrder") != std::string::npos ||
          view->skeletonCode.find("def process_order") != std::string::npos,
          "skeleton stub missing");
    CHECK(view->generatedCode.find("return x") != std::string::npos,
          "generated code missing");
    PASS();
}

void test_detail_contains_diff_summary() {
    TEST(detail_contains_diff_summary);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "w1");
    CHECK(view->diffSummary.find("line_delta=") != std::string::npos,
          "diff summary missing");
    PASS();
}

void test_detail_contains_routing_explanation() {
    TEST(detail_contains_routing_explanation);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "w1");
    CHECK(view->routingExplanation.find("worker=llm") != std::string::npos,
          "routing explanation missing worker");
    CHECK(view->routingExplanation.find("review=required") != std::string::npos,
          "routing explanation missing review");
    PASS();
}

void test_detail_contains_annotation_tags() {
    TEST(detail_contains_annotation_tags);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "w1");
    CHECK(!view->annotationTags.empty(), "annotation tags should exist");
    PASS();
}

void test_detail_includes_rejection_history() {
    TEST(detail_includes_rejection_history);
    auto wf = makeWorkflow();
    auto view = WorkflowTaskDetail::build(wf, "w2");
    CHECK(!view->rejectionHistory.empty(), "rejection history missing");
    PASS();
}

void test_approve_review_item_sets_complete() {
    TEST(approve_review_item_sets_complete);
    auto wf = makeWorkflow();
    CHECK(WorkflowTaskDetail::approve(wf, "w1"), "approve should succeed");
    auto item = wf.queue.getItem("w1");
    CHECK(item.has_value() && item->status == WI_COMPLETE, "status should be complete");
    PASS();
}

void test_approve_non_review_item_fails() {
    TEST(approve_non_review_item_fails);
    auto wf = makeWorkflow();
    CHECK(!WorkflowTaskDetail::approve(wf, "w2"), "approve on complete item should fail");
    PASS();
}

void test_reject_review_item_sets_ready_and_feedback() {
    TEST(reject_review_item_sets_ready_and_feedback);
    auto wf = makeWorkflow();
    CHECK(WorkflowTaskDetail::reject(wf, "w1", "needs tests"), "reject should succeed");
    auto item = wf.queue.getItem("w1");
    CHECK(item.has_value() && item->status == WI_READY, "status should be ready");
    CHECK(item->result.reasoning.find("needs tests") != std::string::npos,
          "feedback should be stored");
    PASS();
}

void test_reject_without_feedback_fails() {
    TEST(reject_without_feedback_fails);
    auto wf = makeWorkflow();
    CHECK(!WorkflowTaskDetail::reject(wf, "w1", ""), "empty feedback should fail");
    PASS();
}

void test_reject_non_review_item_fails() {
    TEST(reject_non_review_item_fails);
    auto wf = makeWorkflow();
    CHECK(!WorkflowTaskDetail::reject(wf, "w2", "no"), "reject on non-review should fail");
    PASS();
}

int main() {
    std::cout << "Step 429: Task Detail View Tests\n";

    test_build_detail_for_existing_item();              // 1
    test_build_detail_missing_item_returns_nullopt();   // 2
    test_detail_contains_skeleton_and_generated_code(); // 3
    test_detail_contains_diff_summary();                // 4
    test_detail_contains_routing_explanation();         // 5
    test_detail_contains_annotation_tags();             // 6
    test_detail_includes_rejection_history();           // 7
    test_approve_review_item_sets_complete();           // 8
    test_approve_non_review_item_fails();               // 9
    test_reject_review_item_sets_ready_and_feedback();  // 10
    test_reject_without_feedback_fails();               // 11
    test_reject_non_review_item_fails();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
