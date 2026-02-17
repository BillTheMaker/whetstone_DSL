// Step 512: Window Chrome + Hierarchy Pass (12 tests)

#include "WindowHierarchyModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<SurfaceNode> baseHierarchy() {
    return {
        {"editor", SurfaceRole::Work, false, true, 0, {}},
        {"left-tool", SurfaceRole::Tool, false, true, 0, {}},
        {"completion", SurfaceRole::Overlay, false, true, 0, {}},
        {"dialog", SurfaceRole::Modal, true, true, 0, {}},
        {"toast", SurfaceRole::Notification, false, true, 0, {}}
    };
}

void test_style_for_work_role_has_lowest_base_z() {
    TEST(style_for_work_role_has_lowest_base_z);
    auto s = WindowHierarchyModel::styleForRole(SurfaceRole::Work, 0);
    CHECK(s.zIndex >= 100 && s.zIndex < 200, "work z-index range invalid");
    PASS();
}

void test_style_for_tool_role_above_work() {
    TEST(style_for_tool_role_above_work);
    auto work = WindowHierarchyModel::styleForRole(SurfaceRole::Work, 0);
    auto tool = WindowHierarchyModel::styleForRole(SurfaceRole::Tool, 0);
    CHECK(tool.zIndex > work.zIndex, "tool should be above work");
    PASS();
}

void test_overlay_role_above_tool() {
    TEST(overlay_role_above_tool);
    auto tool = WindowHierarchyModel::styleForRole(SurfaceRole::Tool, 0);
    auto overlay = WindowHierarchyModel::styleForRole(SurfaceRole::Overlay, 0);
    CHECK(overlay.zIndex > tool.zIndex, "overlay should be above tool");
    PASS();
}

void test_modal_role_above_overlay() {
    TEST(modal_role_above_overlay);
    auto overlay = WindowHierarchyModel::styleForRole(SurfaceRole::Overlay, 0);
    auto modal = WindowHierarchyModel::styleForRole(SurfaceRole::Modal, 0);
    CHECK(modal.zIndex > overlay.zIndex, "modal should be above overlay");
    PASS();
}

void test_validate_marks_modal_flag_missing() {
    TEST(validate_marks_modal_flag_missing);
    auto nodes = baseHierarchy();
    nodes[3].modal = false;
    auto report = WindowHierarchyModel::validateAndOrder(nodes);
    CHECK(!report.valid, "report should be invalid");
    CHECK(!report.violations.empty(), "expected violations");
    PASS();
}

void test_normalize_sets_modal_flag() {
    TEST(normalize_sets_modal_flag);
    auto nodes = baseHierarchy();
    nodes[3].modal = false;
    auto normalized = WindowHierarchyModel::normalize(nodes);
    bool fixed = false;
    for (const auto& n : normalized) {
        if (n.id == "dialog" && n.modal) fixed = true;
    }
    CHECK(fixed, "modal flag should be restored");
    PASS();
}

void test_order_is_sorted_by_zindex() {
    TEST(order_is_sorted_by_zindex);
    auto report = WindowHierarchyModel::validateAndOrder(baseHierarchy());
    bool sorted = true;
    for (size_t i = 1; i < report.ordered.size(); ++i) {
        if (report.ordered[i - 1].style.zIndex > report.ordered[i].style.zIndex) sorted = false;
    }
    CHECK(sorted, "order should be sorted by z-index");
    PASS();
}

void test_blocks_interaction_for_modal_over_work() {
    TEST(blocks_interaction_for_modal_over_work);
    auto modal = SurfaceNode{"dialog", SurfaceRole::Modal, true, true, 0,
        WindowHierarchyModel::styleForRole(SurfaceRole::Modal, 0)};
    auto work = SurfaceNode{"editor", SurfaceRole::Work, false, true, 0,
        WindowHierarchyModel::styleForRole(SurfaceRole::Work, 0)};
    CHECK(WindowHierarchyModel::blocksInteraction(modal, work),
          "modal should block underlying work surface");
    PASS();
}

void test_non_interactive_overlay_does_not_block() {
    TEST(non_interactive_overlay_does_not_block);
    auto overlay = SurfaceNode{"overlay", SurfaceRole::Overlay, false, false, 0,
        WindowHierarchyModel::styleForRole(SurfaceRole::Overlay, 0)};
    auto work = SurfaceNode{"editor", SurfaceRole::Work, false, true, 0,
        WindowHierarchyModel::styleForRole(SurfaceRole::Work, 0)};
    CHECK(!WindowHierarchyModel::blocksInteraction(overlay, work),
          "non-interactive overlay should not block");
    PASS();
}

void test_depth_increases_zindex_within_role() {
    TEST(depth_increases_zindex_within_role);
    auto a = WindowHierarchyModel::styleForRole(SurfaceRole::Tool, 0);
    auto b = WindowHierarchyModel::styleForRole(SurfaceRole::Tool, 3);
    CHECK(b.zIndex > a.zIndex, "deeper node should get higher z-index");
    PASS();
}

void test_notification_between_overlay_and_modal() {
    TEST(notification_between_overlay_and_modal);
    auto notif = WindowHierarchyModel::styleForRole(SurfaceRole::Notification, 0);
    auto overlay = WindowHierarchyModel::styleForRole(SurfaceRole::Overlay, 0);
    auto modal = WindowHierarchyModel::styleForRole(SurfaceRole::Modal, 0);
    CHECK(notif.zIndex > overlay.zIndex, "notification should be above overlay");
    CHECK(notif.zIndex < modal.zIndex, "notification should be below modal");
    PASS();
}

void test_validate_passes_for_base_hierarchy() {
    TEST(validate_passes_for_base_hierarchy);
    auto report = WindowHierarchyModel::validateAndOrder(baseHierarchy());
    CHECK(report.valid, "base hierarchy should be valid");
    PASS();
}

int main() {
    std::cout << "Step 512: Window Chrome + Hierarchy Pass\n";

    test_style_for_work_role_has_lowest_base_z();     // 1
    test_style_for_tool_role_above_work();            // 2
    test_overlay_role_above_tool();                   // 3
    test_modal_role_above_overlay();                  // 4
    test_validate_marks_modal_flag_missing();         // 5
    test_normalize_sets_modal_flag();                 // 6
    test_order_is_sorted_by_zindex();                 // 7
    test_blocks_interaction_for_modal_over_work();    // 8
    test_non_interactive_overlay_does_not_block();    // 9
    test_depth_increases_zindex_within_role();        // 10
    test_notification_between_overlay_and_modal();    // 11
    test_validate_passes_for_base_hierarchy();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
