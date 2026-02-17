// Step 437: Phase 19b Integration + Sprint Summary Tests (8 tests)

#include "CodeAnnotationBadges.h"
#include "CodeAnnotationHeatmap.h"
#include "WorkflowStatusOverlay.h"
#include "ReviewComparisonView.h"
#include "WorkflowTaskBoard.h"
#include "WorkflowProgressDashboard.h"

#include <iostream>
#include <map>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& id,
                         const std::string& nodeId,
                         const std::string& name,
                         const std::string& status,
                         const std::string& file,
                         const std::string& worker) {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = nodeId;
    wi.nodeName = name;
    wi.status = status;
    wi.bufferId = file;
    wi.workerType = worker;
    wi.priority = "high";
    wi.contextWidth = "file";
    wi.reviewRequired = (status == WI_REVIEW);
    wi.result.generatedCode = "void " + name + "() {\n  int x = 1;\n}\n";
    return wi;
}

static WorkflowState makeWorkflow() {
    WorkflowState wf("phase19b");
    wf.queue.enqueue(makeItem("t1", "n1", "buildSkeleton", WI_READY, "src/a.cpp", "template"));
    wf.queue.enqueue(makeItem("t2", "n2", "emitLogic", WI_IN_PROGRESS, "src/a.cpp", "llm"));
    wf.queue.enqueue(makeItem("t3", "n3", "reviewLogic", WI_REVIEW, "src/a.cpp", "human"));
    wf.queue.enqueue(makeItem("t4", "n4", "finalize", WI_COMPLETE, "src/b.cpp", "deterministic"));
    return wf;
}

static std::vector<AnnotationBadgeInput> makeAnnotations() {
    return {
        {10, "IntentAnnotation", "intent", "n1"},
        {20, "ComplexityAnnotation", "complex", "n2"},
        {20, "RiskAnnotation", "risk", "n2"},
        {30, "ReviewAnnotation", "review", "n3"}
    };
}

static std::map<std::string, int> makeNodeLines() {
    return {{"n1", 10}, {"n2", 20}, {"n3", 30}, {"n4", 40}};
}

static std::string extractItemId(const std::string& route) {
    const std::string prefix = "task://detail/";
    if (route.find(prefix) != 0) return "";
    return route.substr(prefix.size());
}

void test_annotation_badges_visible_on_annotated_code() {
    TEST(annotation_badges_visible_on_annotated_code);
    auto badges = CodeAnnotationBadges::buildBadges(makeAnnotations());
    CHECK(badges.size() == 4, "expected one badge per annotation");
    PASS();
}

void test_heatmap_highlights_complex_regions() {
    TEST(heatmap_highlights_complex_regions);
    auto cells = CodeAnnotationHeatmap::build(makeAnnotations(), 35, true);
    int score10 = 0, score20 = 0;
    for (const auto& c : cells) {
        if (c.line == 10) score10 = c.score;
        if (c.line == 20) score20 = c.score;
    }
    CHECK(score20 > score10, "complex region should score higher than intent-only region");
    PASS();
}

void test_overlay_reflects_live_orchestration_state() {
    TEST(overlay_reflects_live_orchestration_state);
    auto wf = makeWorkflow();
    auto markers = WorkflowStatusOverlay::build(wf, makeNodeLines(), "src/a.cpp");
    CHECK(markers.size() == 3, "expected three markers for src/a.cpp");
    bool hasReview = false;
    for (const auto& m : markers) {
        if (m.itemId == "t3" && m.statusIcon == "eye") hasReview = true;
    }
    CHECK(hasReview, "review item should have eye icon");
    PASS();
}

void test_overlay_click_opens_review_comparison() {
    TEST(overlay_click_opens_review_comparison);
    auto wf = makeWorkflow();
    auto markers = WorkflowStatusOverlay::build(wf, makeNodeLines(), "src/a.cpp");
    std::string route;
    for (const auto& m : markers) if (m.itemId == "t3") route = m.detailRoute;
    std::string itemId = extractItemId(route);
    auto review = ReviewComparisonView::build(wf, itemId);
    CHECK(!itemId.empty(), "expected detail route item id");
    CHECK(review.has_value(), "review comparison should open from overlay click");
    PASS();
}

void test_task_board_review_item_opens_comparison() {
    TEST(task_board_review_item_opens_comparison);
    auto wf = makeWorkflow();
    auto board = WorkflowTaskBoard::build(wf);
    std::string reviewId;
    for (const auto& col : board) {
        if (col.key == "review" && !col.cards.empty()) reviewId = col.cards[0].itemId;
    }
    auto review = ReviewComparisonView::build(wf, reviewId);
    CHECK(!reviewId.empty(), "review card id missing");
    CHECK(review.has_value(), "review comparison should open from board selection");
    PASS();
}

void test_review_approve_updates_board_and_overlay() {
    TEST(review_approve_updates_board_and_overlay);
    auto wf = makeWorkflow();
    CHECK(ReviewComparisonView::approve(wf, "t3"), "approve should succeed");

    auto board = WorkflowTaskBoard::build(wf);
    int reviewCount = 0;
    int completeCount = 0;
    for (const auto& col : board) {
        if (col.key == "review") reviewCount = (int)col.cards.size();
        if (col.key == "complete") completeCount = (int)col.cards.size();
    }
    auto markers = WorkflowStatusOverlay::build(wf, makeNodeLines(), "src/a.cpp");
    bool t3Complete = false;
    for (const auto& m : markers) {
        if (m.itemId == "t3" && m.statusIcon == "check") t3Complete = true;
    }

    CHECK(reviewCount == 0, "review column should be empty after approval");
    CHECK(completeCount >= 2, "complete column should increase");
    CHECK(t3Complete, "overlay should show approved item as check");
    PASS();
}

void test_review_reject_requeues_and_overlay_pending_style() {
    TEST(review_reject_requeues_and_overlay_pending_style);
    auto wf = makeWorkflow();
    CHECK(ReviewComparisonView::reject(wf, "t3", "needs stronger guard"), "reject should succeed");

    auto board = WorkflowTaskBoard::build(wf);
    int readyCount = 0;
    for (const auto& col : board) if (col.key == "ready") readyCount = (int)col.cards.size();

    auto markers = WorkflowStatusOverlay::build(wf, makeNodeLines(), "src/a.cpp");
    bool t3Outline = false;
    for (const auto& m : markers) {
        if (m.itemId == "t3" && m.statusIcon == "outline") t3Outline = true;
    }

    CHECK(readyCount >= 1, "rejected item should requeue into ready board column");
    CHECK(t3Outline, "overlay should show requeued item as outline");
    PASS();
}

void test_end_to_end_visualization_stack_operational() {
    TEST(end_to_end_visualization_stack_operational);
    auto wf = makeWorkflow();
    auto annotations = makeAnnotations();

    auto badges = CodeAnnotationBadges::buildBadges(annotations);
    auto heatmap = CodeAnnotationHeatmap::build(annotations, 40, true);
    auto overlay = WorkflowStatusOverlay::build(wf, makeNodeLines());
    auto review = ReviewComparisonView::build(wf, "t3");

    WorkflowProgress p(4);
    p.recordEvent({"routed", "t1", {{"workerType", "template"}}, "2026-01-01T00:00:00Z"});
    p.recordEvent({"completed", "t4", json::object(), "2026-01-01T00:01:00Z"});
    auto dashboard = WorkflowProgressDashboard::build(wf, p, nullptr);

    CHECK(!badges.empty(), "badges should be available");
    CHECK(!heatmap.empty(), "heatmap should be available");
    CHECK(!overlay.empty(), "overlay should be available");
    CHECK(review.has_value(), "review comparison should be available");
    CHECK(dashboard.totalItems == 4, "dashboard total mismatch");
    PASS();
}

int main() {
    std::cout << "Step 437: Phase 19b Integration + Sprint Summary Tests\n";

    test_annotation_badges_visible_on_annotated_code();         // 1
    test_heatmap_highlights_complex_regions();                  // 2
    test_overlay_reflects_live_orchestration_state();           // 3
    test_overlay_click_opens_review_comparison();               // 4
    test_task_board_review_item_opens_comparison();             // 5
    test_review_approve_updates_board_and_overlay();            // 6
    test_review_reject_requeues_and_overlay_pending_style();    // 7
    test_end_to_end_visualization_stack_operational();          // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
