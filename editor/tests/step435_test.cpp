// Step 435: Workflow Status Overlay Tests (12 tests)

#include "WorkflowStatusOverlay.h"

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
                         const std::string& file) {
    WorkItem wi;
    wi.id = id;
    wi.nodeId = nodeId;
    wi.nodeName = name;
    wi.status = status;
    wi.bufferId = file;
    wi.workerType = "llm";
    wi.priority = "medium";
    return wi;
}

static WorkflowState sampleState() {
    WorkflowState wf("overlay");
    wf.queue.enqueue(makeItem("w1", "n1", "parseTokens", WI_PENDING, "src/a.cpp"));
    wf.queue.enqueue(makeItem("w2", "n2", "routeTask", WI_READY, "src/a.cpp"));
    wf.queue.enqueue(makeItem("w3", "n3", "emitCode", WI_IN_PROGRESS, "src/a.cpp"));
    wf.queue.enqueue(makeItem("w4", "n4", "reviewOutput", WI_REVIEW, "src/b.cpp"));
    wf.queue.enqueue(makeItem("w5", "n5", "finalize", WI_COMPLETE, "src/b.cpp"));
    return wf;
}

void test_build_overlay_returns_marker_per_item() {
    TEST(build_overlay_returns_marker_per_item);
    auto wf = sampleState();
    std::map<std::string, int> lines = {{"n1", 10}, {"n2", 20}, {"n3", 30}, {"n4", 40}, {"n5", 50}};
    auto markers = WorkflowStatusOverlay::build(wf, lines);
    CHECK(markers.size() == 5, "expected one marker per work item");
    PASS();
}

void test_buffer_filter_limits_markers() {
    TEST(buffer_filter_limits_markers);
    auto wf = sampleState();
    std::map<std::string, int> lines = {{"n1", 10}, {"n2", 20}, {"n3", 30}, {"n4", 40}, {"n5", 50}};
    auto markers = WorkflowStatusOverlay::build(wf, lines, "src/a.cpp");
    CHECK(markers.size() == 3, "expected only src/a.cpp markers");
    PASS();
}

void test_missing_line_mapping_defaults_to_zero() {
    TEST(missing_line_mapping_defaults_to_zero);
    auto wf = sampleState();
    std::map<std::string, int> lines = {{"n1", 10}};
    auto markers = WorkflowStatusOverlay::build(wf, lines);
    bool hasZero = false;
    for (const auto& m : markers) if (m.nodeId == "n2" && m.line == 0) hasZero = true;
    CHECK(hasZero, "missing line map should produce line 0");
    PASS();
}

void test_icon_mapping_for_pending_is_outline() {
    TEST(icon_mapping_for_pending_is_outline);
    CHECK(WorkflowStatusOverlay::iconForStatus(WI_PENDING) == "outline", "pending icon mismatch");
    PASS();
}

void test_icon_mapping_for_in_progress_is_spinner() {
    TEST(icon_mapping_for_in_progress_is_spinner);
    CHECK(WorkflowStatusOverlay::iconForStatus(WI_IN_PROGRESS) == "spinner", "in-progress icon mismatch");
    PASS();
}

void test_icon_mapping_for_review_is_eye() {
    TEST(icon_mapping_for_review_is_eye);
    CHECK(WorkflowStatusOverlay::iconForStatus(WI_REVIEW) == "eye", "review icon mismatch");
    PASS();
}

void test_icon_mapping_for_complete_is_check() {
    TEST(icon_mapping_for_complete_is_check);
    CHECK(WorkflowStatusOverlay::iconForStatus(WI_COMPLETE) == "check", "complete icon mismatch");
    PASS();
}

void test_color_mapping_matches_board_statuses() {
    TEST(color_mapping_matches_board_statuses);
    CHECK(WorkflowStatusOverlay::colorForStatus(WI_REVIEW) == "#F39C12", "review color mismatch");
    CHECK(WorkflowStatusOverlay::colorForStatus(WI_COMPLETE) == "#27AE60", "complete color mismatch");
    PASS();
}

void test_board_column_mapping_for_assigned() {
    TEST(board_column_mapping_for_assigned);
    CHECK(WorkflowStatusOverlay::boardColumnForStatus(WI_ASSIGNED) == "in-progress", "assigned column mismatch");
    PASS();
}

void test_click_route_points_to_task_detail() {
    TEST(click_route_points_to_task_detail);
    CHECK(WorkflowStatusOverlay::detailRoute("work-9") == "task://detail/work-9", "detail route mismatch");
    PASS();
}

void test_display_name_prefers_node_name() {
    TEST(display_name_prefers_node_name);
    WorkflowState wf("overlay");
    wf.queue.enqueue(makeItem("x1", "node-x", "prettyName", WI_READY, "src/c.cpp"));
    std::map<std::string, int> lines = {{"node-x", 12}};
    auto markers = WorkflowStatusOverlay::build(wf, lines);
    CHECK(markers.size() == 1, "expected one marker");
    CHECK(markers[0].displayName == "prettyName", "display name should prefer nodeName");
    PASS();
}

void test_markers_sort_by_file_then_line() {
    TEST(markers_sort_by_file_then_line);
    WorkflowState wf("overlay");
    wf.queue.enqueue(makeItem("z2", "n2", "b", WI_READY, "src/z.cpp"));
    wf.queue.enqueue(makeItem("a1", "n1", "a", WI_READY, "src/a.cpp"));
    std::map<std::string, int> lines = {{"n1", 40}, {"n2", 5}};
    auto markers = WorkflowStatusOverlay::build(wf, lines);
    CHECK(markers.size() == 2, "expected two markers");
    CHECK(markers[0].bufferId == "src/a.cpp", "file order mismatch");
    CHECK(markers[1].bufferId == "src/z.cpp", "file order mismatch");
    PASS();
}

int main() {
    std::cout << "Step 435: Workflow Status Overlay Tests\n";

    test_build_overlay_returns_marker_per_item();      // 1
    test_buffer_filter_limits_markers();               // 2
    test_missing_line_mapping_defaults_to_zero();      // 3
    test_icon_mapping_for_pending_is_outline();        // 4
    test_icon_mapping_for_in_progress_is_spinner();    // 5
    test_icon_mapping_for_review_is_eye();             // 6
    test_icon_mapping_for_complete_is_check();         // 7
    test_color_mapping_matches_board_statuses();       // 8
    test_board_column_mapping_for_assigned();          // 9
    test_click_route_points_to_task_detail();          // 10
    test_display_name_prefers_node_name();             // 11
    test_markers_sort_by_file_then_line();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
