// Step 430: Dependency Graph Visualization Tests (12 tests)

#include "WorkflowDependencyView.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkItem makeItem(const std::string& id, const std::string& status,
                         const std::vector<std::string>& deps = {}) {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = id + "_node";
    wi.nodeName = id;
    wi.nodeType = "Function";
    wi.status = status;
    wi.workerType = "llm";
    wi.priority = "medium";
    wi.dependencies = deps;
    return wi;
}

static WorkflowState makeChainWorkflow() {
    WorkflowState wf("graph");
    wf.queue.enqueue(makeItem("a", WI_COMPLETE, {}));
    wf.queue.enqueue(makeItem("b", WI_IN_PROGRESS, {"a"}));
    wf.queue.enqueue(makeItem("c", WI_READY, {"b"}));
    return wf;
}

void test_build_graph_view_node_and_edge_counts() {
    TEST(build_graph_view_node_and_edge_counts);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    CHECK(view.nodes.size() == 3, "expected 3 nodes");
    CHECK(view.edges.size() == 2, "expected 2 edges");
    PASS();
}

void test_status_color_complete_green() {
    TEST(status_color_complete_green);
    CHECK(WorkflowDependencyView::colorForStatus(WI_COMPLETE) == "#2ECC71",
          "complete should be green");
    PASS();
}

void test_status_color_in_progress_blue() {
    TEST(status_color_in_progress_blue);
    CHECK(WorkflowDependencyView::colorForStatus(WI_IN_PROGRESS) == "#3498DB",
          "in-progress should be blue");
    PASS();
}

void test_status_color_pending_ready_gray() {
    TEST(status_color_pending_ready_gray);
    CHECK(WorkflowDependencyView::colorForStatus(WI_PENDING) == "#95A5A6", "pending gray");
    CHECK(WorkflowDependencyView::colorForStatus(WI_READY) == "#95A5A6", "ready gray");
    PASS();
}

void test_status_color_review_red() {
    TEST(status_color_review_red);
    CHECK(WorkflowDependencyView::colorForStatus(WI_REVIEW) == "#E74C3C", "review red");
    PASS();
}

void test_critical_path_detected_for_chain() {
    TEST(critical_path_detected_for_chain);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    CHECK(view.criticalPath.size() == 3, "chain critical path should be 3");
    PASS();
}

void test_critical_path_nodes_marked() {
    TEST(critical_path_nodes_marked);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    int crit = 0;
    for (const auto& n : view.nodes) if (n.criticalPath) ++crit;
    CHECK(crit >= 3, "all chain nodes should be critical");
    PASS();
}

void test_critical_path_edges_marked() {
    TEST(critical_path_edges_marked);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    int crit = 0;
    for (const auto& e : view.edges) if (e.criticalPath) ++crit;
    CHECK(crit == 2, "both chain edges should be critical");
    PASS();
}

void test_hover_text_contains_details() {
    TEST(hover_text_contains_details);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    auto node = view.getNode("b");
    CHECK(node.has_value(), "node b missing");
    CHECK(node->hoverText.find("Status:") != std::string::npos, "hover missing status");
    CHECK(node->hoverText.find("Worker:") != std::string::npos, "hover missing worker");
    PASS();
}

void test_navigate_to_task_id_from_node_click() {
    TEST(navigate_to_task_id_from_node_click);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    std::string taskId = WorkflowDependencyView::navigateToTaskId(view, "c");
    CHECK(taskId == "c", "navigation should return task id");
    PASS();
}

void test_navigate_missing_node_returns_empty() {
    TEST(navigate_missing_node_returns_empty);
    auto view = WorkflowDependencyView::build(makeChainWorkflow());
    CHECK(WorkflowDependencyView::navigateToTaskId(view, "missing").empty(),
          "missing node should return empty task id");
    PASS();
}

void test_graph_updates_after_status_change() {
    TEST(graph_updates_after_status_change);
    auto wf = makeChainWorkflow();
    auto v1 = WorkflowDependencyView::build(wf);
    auto b1 = v1.getNode("b");
    CHECK(b1.has_value() && b1->colorHex == "#3498DB", "b should start blue");
    auto item = wf.queue.getItem("b");
    item->status = WI_COMPLETE;
    wf.queue.updateItem("b", *item);
    auto v2 = WorkflowDependencyView::build(wf);
    auto b2 = v2.getNode("b");
    CHECK(b2.has_value() && b2->colorHex == "#2ECC71", "b should turn green");
    PASS();
}

int main() {
    std::cout << "Step 430: Dependency Graph Visualization Tests\n";

    test_build_graph_view_node_and_edge_counts(); // 1
    test_status_color_complete_green();           // 2
    test_status_color_in_progress_blue();         // 3
    test_status_color_pending_ready_gray();       // 4
    test_status_color_review_red();               // 5
    test_critical_path_detected_for_chain();      // 6
    test_critical_path_nodes_marked();            // 7
    test_critical_path_edges_marked();            // 8
    test_hover_text_contains_details();           // 9
    test_navigate_to_task_id_from_node_click();   // 10
    test_navigate_missing_node_returns_empty();    // 11
    test_graph_updates_after_status_change();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
