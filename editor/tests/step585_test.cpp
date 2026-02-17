// Step 585: Workflow Visualization 2.0 (12 tests)

#include "WorkflowVisualizationV2.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static WorkflowNodeVisual node(const std::string& id, WorkflowNodeKind kind, const std::string& label) {
    WorkflowNodeVisual n;
    n.nodeId = id;
    n.kind = kind;
    n.label = label;
    n.accentColorHex = "#2F7BFF";
    return n;
}

void test_add_node_success() {
    TEST(add_node_success);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("intake", WorkflowNodeKind::Intake, "Intake"), &error), "add node should succeed");
    CHECK(graph.nodes.size() == 1, "node count mismatch");
    PASS();
}

void test_add_node_rejects_missing_id() {
    TEST(add_node_rejects_missing_id);
    WorkflowVisualizationGraph graph;
    std::string error;
    auto n = node("", WorkflowNodeKind::Intake, "Intake");
    CHECK(!WorkflowVisualizationV2::addNode(&graph, n, &error), "add node should fail");
    CHECK(error == "node_id_missing", "wrong error");
    PASS();
}

void test_add_node_rejects_invalid_color() {
    TEST(add_node_rejects_invalid_color);
    WorkflowVisualizationGraph graph;
    std::string error;
    auto n = node("intake", WorkflowNodeKind::Intake, "Intake");
    n.accentColorHex = "blue";
    CHECK(!WorkflowVisualizationV2::addNode(&graph, n, &error), "add node should fail");
    CHECK(error == "node_color_invalid", "wrong error");
    PASS();
}

void test_add_node_rejects_duplicate() {
    TEST(add_node_rejects_duplicate);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("intake", WorkflowNodeKind::Intake, "Intake"), &error), "first add failed");
    CHECK(!WorkflowVisualizationV2::addNode(&graph, node("intake", WorkflowNodeKind::Intake, "Intake2"), &error), "duplicate should fail");
    CHECK(error == "node_duplicate", "wrong error");
    PASS();
}

void test_add_edge_success() {
    TEST(add_edge_success);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("a", WorkflowNodeKind::Intake, "A"), &error), "add A failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("b", WorkflowNodeKind::ReviewGate, "B"), &error), "add B failed");
    WorkflowEdgeVisual e{"edge-1", "a", "b", "needs review due to ambiguity"};
    CHECK(WorkflowVisualizationV2::addEdge(&graph, e, &error), "add edge should succeed");
    CHECK(graph.edges.size() == 1, "edge count mismatch");
    PASS();
}

void test_add_edge_rejects_missing_node_reference() {
    TEST(add_edge_rejects_missing_node_reference);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("a", WorkflowNodeKind::Intake, "A"), &error), "add A failed");
    WorkflowEdgeVisual e{"edge-1", "a", "missing", "reason"};
    CHECK(!WorkflowVisualizationV2::addEdge(&graph, e, &error), "add edge should fail");
    CHECK(error == "edge_node_missing", "wrong error");
    PASS();
}

void test_add_edge_rejects_duplicate_edge_id() {
    TEST(add_edge_rejects_duplicate_edge_id);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("a", WorkflowNodeKind::Intake, "A"), &error), "add A failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("b", WorkflowNodeKind::ReviewGate, "B"), &error), "add B failed");
    WorkflowEdgeVisual e{"edge-1", "a", "b", "reason"};
    CHECK(WorkflowVisualizationV2::addEdge(&graph, e, &error), "first edge add failed");
    CHECK(!WorkflowVisualizationV2::addEdge(&graph, e, &error), "duplicate edge should fail");
    CHECK(error == "edge_duplicate", "wrong error");
    PASS();
}

void test_outgoing_filters_by_from_node() {
    TEST(outgoing_filters_by_from_node);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("a", WorkflowNodeKind::Intake, "A"), &error), "add A failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("b", WorkflowNodeKind::ReviewGate, "B"), &error), "add B failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("c", WorkflowNodeKind::TaskQueue, "C"), &error), "add C failed");
    CHECK(WorkflowVisualizationV2::addEdge(&graph, {"e1", "a", "b", "r1"}, &error), "add e1 failed");
    CHECK(WorkflowVisualizationV2::addEdge(&graph, {"e2", "a", "c", "r2"}, &error), "add e2 failed");
    CHECK(WorkflowVisualizationV2::addEdge(&graph, {"e3", "b", "c", "r3"}, &error), "add e3 failed");
    const auto out = WorkflowVisualizationV2::outgoing(graph, "a");
    CHECK(out.size() == 2, "outgoing count mismatch");
    PASS();
}

void test_routing_summary_emits_plain_language_lines() {
    TEST(routing_summary_emits_plain_language_lines);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("intake", WorkflowNodeKind::Intake, "Intake"), &error), "add intake failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("review", WorkflowNodeKind::ReviewGate, "Review"), &error), "add review failed");
    CHECK(WorkflowVisualizationV2::addEdge(&graph, {"e1", "intake", "review", "ambiguity requires architect review"}, &error), "add edge failed");
    const auto lines = WorkflowVisualizationV2::plainLanguageRoutingSummary(graph);
    CHECK(lines.size() == 1, "summary size mismatch");
    CHECK(lines[0].find("ambiguity requires architect review") != std::string::npos, "summary reason missing");
    PASS();
}

void test_nodes_by_kind_filters_review_gates() {
    TEST(nodes_by_kind_filters_review_gates);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("intake", WorkflowNodeKind::Intake, "Intake"), &error), "add intake failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("review1", WorkflowNodeKind::ReviewGate, "Review1"), &error), "add review1 failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("review2", WorkflowNodeKind::ReviewGate, "Review2"), &error), "add review2 failed");
    const auto reviewNodes = WorkflowVisualizationV2::nodesByKind(graph, WorkflowNodeKind::ReviewGate);
    CHECK(reviewNodes.size() == 2, "review node count mismatch");
    PASS();
}

void test_electric_blue_default_is_preserved() {
    TEST(electric_blue_default_is_preserved);
    WorkflowVisualizationGraph graph;
    std::string error;
    auto n = node("intake", WorkflowNodeKind::Intake, "Intake");
    CHECK(WorkflowVisualizationV2::addNode(&graph, n, &error), "add node failed");
    CHECK(graph.nodes[0].accentColorHex == "#2F7BFF", "default electric-blue mismatch");
    PASS();
}

void test_add_edge_rejects_missing_reason() {
    TEST(add_edge_rejects_missing_reason);
    WorkflowVisualizationGraph graph;
    std::string error;
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("a", WorkflowNodeKind::Intake, "A"), &error), "add A failed");
    CHECK(WorkflowVisualizationV2::addNode(&graph, node("b", WorkflowNodeKind::ReviewGate, "B"), &error), "add B failed");
    CHECK(!WorkflowVisualizationV2::addEdge(&graph, {"e1", "a", "b", ""}, &error), "edge should fail");
    CHECK(error == "edge_reason_missing", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 585: Workflow Visualization 2.0\n";

    test_add_node_success();                           // 1
    test_add_node_rejects_missing_id();               // 2
    test_add_node_rejects_invalid_color();            // 3
    test_add_node_rejects_duplicate();                // 4
    test_add_edge_success();                          // 5
    test_add_edge_rejects_missing_node_reference();   // 6
    test_add_edge_rejects_duplicate_edge_id();        // 7
    test_outgoing_filters_by_from_node();             // 8
    test_routing_summary_emits_plain_language_lines();// 9
    test_nodes_by_kind_filters_review_gates();        // 10
    test_electric_blue_default_is_preserved();        // 11
    test_add_edge_rejects_missing_reason();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
