// Step 432: Phase 19a Integration Tests (8 tests)

#include "WorkflowTaskBoard.h"
#include "WorkflowTaskDetail.h"
#include "WorkflowDependencyView.h"
#include "WorkflowProgressDashboard.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& id, const std::string& status,
                         const std::string& worker, const std::vector<std::string>& deps = {}) {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = id + "_node";
    wi.nodeName = id;
    wi.nodeType = "Function";
    wi.bufferId = "src/" + id + ".cpp";
    wi.status = status;
    wi.workerType = worker;
    wi.priority = "high";
    wi.contextWidth = "file";
    wi.reviewRequired = (worker == "llm" || worker == "human");
    wi.dependencies = deps;
    wi.result.generatedCode = "void " + id + "() {}";
    return wi;
}

static WorkflowState makeWorkflow() {
    WorkflowState wf("phase19a");
    wf.queue.enqueue(makeItem("taskA", WI_READY, "template"));
    wf.queue.enqueue(makeItem("taskB", WI_IN_PROGRESS, "llm", {"taskA"}));
    wf.queue.enqueue(makeItem("taskC", WI_REVIEW, "llm", {"taskB"}));
    wf.queue.enqueue(makeItem("taskD", WI_COMPLETE, "deterministic"));
    return wf;
}

static WorkflowProgress makeProgress() {
    WorkflowProgress p(4);
    p.recordEvent({"routed", "taskA", {{"workerType", "template"}}, "2026-01-01T00:00:00Z"});
    p.recordEvent({"completed", "taskA", json::object(), "2026-01-01T00:01:00Z"});
    p.recordEvent({"routed", "taskB", {{"workerType", "llm"}}, "2026-01-01T00:02:00Z"});
    p.recordEvent({"blocked", "", {{"blockers", json::array({
        {{"type", "needs-review"}, {"itemIds", json::array({"taskC"})}, {"description", "review pending"}}
    })}}, "2026-01-01T00:03:00Z"});
    return p;
}

static int countColumn(const std::vector<TaskBoardColumn>& cols, const std::string& key) {
    for (const auto& c : cols) if (c.key == key) return (int)c.cards.size();
    return 0;
}

void test_task_board_reflects_workflow_state() {
    TEST(task_board_reflects_workflow_state);
    auto board = WorkflowTaskBoard::build(makeWorkflow());
    CHECK(countColumn(board, "ready") >= 1, "ready column missing cards");
    CHECK(countColumn(board, "review") >= 1, "review column missing cards");
    PASS();
}

void test_dependency_graph_updates_after_completion() {
    TEST(dependency_graph_updates_after_completion);
    auto wf = makeWorkflow();
    auto g1 = WorkflowDependencyView::build(wf);
    auto b1 = g1.getNode("taskB");
    CHECK(b1.has_value() && b1->colorHex == "#3498DB", "taskB should start blue");
    auto item = wf.queue.getItem("taskB");
    item->status = WI_COMPLETE;
    wf.queue.updateItem("taskB", *item);
    auto g2 = WorkflowDependencyView::build(wf);
    auto b2 = g2.getNode("taskB");
    CHECK(b2.has_value() && b2->colorHex == "#2ECC71", "taskB should turn green");
    PASS();
}

void test_progress_dashboard_reflects_event_stream() {
    TEST(progress_dashboard_reflects_event_stream);
    auto d = WorkflowProgressDashboard::build(makeWorkflow(), makeProgress(), nullptr);
    CHECK(!d.throughputSeries.empty(), "throughput series missing");
    CHECK(!d.blockerSummary.empty(), "blocker summary missing");
    PASS();
}

void test_review_approve_updates_board_and_detail() {
    TEST(review_approve_updates_board_and_detail);
    auto wf = makeWorkflow();
    CHECK(WorkflowTaskDetail::approve(wf, "taskC"), "approve should succeed");
    auto board = WorkflowTaskBoard::build(wf);
    CHECK(countColumn(board, "review") == 0, "review column should be cleared");
    CHECK(countColumn(board, "complete") >= 2, "complete column should increase");
    auto detail = WorkflowTaskDetail::build(wf, "taskC");
    CHECK(detail.has_value() && detail->status == WI_COMPLETE, "detail should show complete");
    PASS();
}

void test_review_reject_requeues_and_visible_on_board() {
    TEST(review_reject_requeues_and_visible_on_board);
    auto wf = makeWorkflow();
    CHECK(WorkflowTaskDetail::reject(wf, "taskC", "needs changes"), "reject should succeed");
    auto board = WorkflowTaskBoard::build(wf);
    CHECK(countColumn(board, "ready") >= 2, "rejected item should requeue to ready");
    PASS();
}

void test_dependency_click_navigates_to_task_detail() {
    TEST(dependency_click_navigates_to_task_detail);
    auto wf = makeWorkflow();
    auto graph = WorkflowDependencyView::build(wf);
    std::string id = WorkflowDependencyView::navigateToTaskId(graph, "taskB");
    auto detail = WorkflowTaskDetail::build(wf, id);
    CHECK(!id.empty(), "navigation id missing");
    CHECK(detail.has_value(), "detail should be openable from graph click");
    PASS();
}

void test_board_filter_and_detail_consistency() {
    TEST(board_filter_and_detail_consistency);
    auto wf = makeWorkflow();
    TaskBoardFilter f;
    f.workerType = "llm";
    auto board = WorkflowTaskBoard::build(wf, f);
    int cards = 0;
    std::string firstId;
    for (const auto& col : board) {
        cards += (int)col.cards.size();
        if (firstId.empty() && !col.cards.empty()) firstId = col.cards[0].itemId;
    }
    auto detail = WorkflowTaskDetail::build(wf, firstId);
    CHECK(cards >= 2, "llm filter should keep two cards");
    CHECK(detail.has_value() && detail->workerType == "llm", "detail worker mismatch");
    PASS();
}

void test_end_to_end_visual_workflow_sequence() {
    TEST(end_to_end_visual_workflow_sequence);
    auto wf = makeWorkflow();
    WorkflowProgress prog(4);
    prog.recordEvent({"routed", "taskA", {{"workerType", "template"}}, "2026-01-01T00:00:00Z"});
    prog.recordEvent({"completed", "taskA", json::object(), "2026-01-01T00:01:00Z"});

    auto a = wf.queue.getItem("taskB");
    a->status = WI_COMPLETE;
    wf.queue.updateItem("taskB", *a);
    prog.recordEvent({"completed", "taskB", json::object(), "2026-01-01T00:02:00Z"});

    CHECK(WorkflowTaskDetail::approve(wf, "taskC"), "review approval failed");
    prog.recordEvent({"completed", "taskC", json::object(), "2026-01-01T00:03:00Z"});

    auto board = WorkflowTaskBoard::build(wf);
    auto graph = WorkflowDependencyView::build(wf);
    auto dash = WorkflowProgressDashboard::build(wf, prog, nullptr);

    CHECK(countColumn(board, "review") == 0, "review should be empty");
    CHECK(graph.getNode("taskC").has_value() &&
          graph.getNode("taskC")->colorHex == "#2ECC71",
          "taskC should be green in graph");
    CHECK(dash.completedItems >= 3, "dashboard completion should advance");
    PASS();
}

int main() {
    std::cout << "Step 432: Phase 19a Integration Tests\n";

    test_task_board_reflects_workflow_state();           // 1
    test_dependency_graph_updates_after_completion();    // 2
    test_progress_dashboard_reflects_event_stream();     // 3
    test_review_approve_updates_board_and_detail();      // 4
    test_review_reject_requeues_and_visible_on_board();  // 5
    test_dependency_click_navigates_to_task_detail();    // 6
    test_board_filter_and_detail_consistency();          // 7
    test_end_to_end_visual_workflow_sequence();          // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
