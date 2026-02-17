// Step 513: Accessibility Baseline Pass (12 tests)

#include "AccessibilityBaseline.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<AccessibilityNode> baseNodes() {
    return {
        {"menu", true, true, true, 0},
        {"explorer", true, true, true, 1},
        {"editor", true, true, true, 2},
        {"status", true, true, true, 3}
    };
}

void test_traversal_order_uses_tab_order() {
    TEST(traversal_order_uses_tab_order);
    auto order = AccessibilityBaseline::keyboardTraversalOrder(baseNodes());
    CHECK(order.size() == 4, "expected 4 items");
    CHECK(order[0] == "menu" && order[3] == "status", "unexpected order");
    PASS();
}

void test_traversal_skips_disabled_nodes() {
    TEST(traversal_skips_disabled_nodes);
    auto nodes = baseNodes();
    nodes[1].enabled = false;
    auto order = AccessibilityBaseline::keyboardTraversalOrder(nodes);
    CHECK(order.size() == 3, "disabled node should be skipped");
    PASS();
}

void test_traversal_skips_invisible_nodes() {
    TEST(traversal_skips_invisible_nodes);
    auto nodes = baseNodes();
    nodes[2].visible = false;
    auto order = AccessibilityBaseline::keyboardTraversalOrder(nodes);
    CHECK(order.size() == 3, "invisible node should be skipped");
    PASS();
}

void test_focus_ring_resolves_for_focused_control() {
    TEST(focus_ring_resolves_for_focused_control);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, false, 0.1f, 0.9f);
    CHECK(ring.thickness >= 2.0f, "ring thickness too low");
    CHECK(ring.alpha >= 0.85f, "ring alpha too low");
    PASS();
}

void test_focus_ring_disabled_when_not_focused() {
    TEST(focus_ring_disabled_when_not_focused);
    auto ring = AccessibilityBaseline::resolveFocusRing(false, false, 0.1f, 0.9f);
    CHECK(ring.thickness == 0.0f, "unfocused ring should be off");
    PASS();
}

void test_focus_ring_visible_with_good_contrast() {
    TEST(focus_ring_visible_with_good_contrast);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, true, 0.1f, 0.9f);
    CHECK(AccessibilityBaseline::focusRingVisible(ring), "ring should be visible");
    PASS();
}

void test_focus_ring_not_visible_with_low_contrast() {
    TEST(focus_ring_not_visible_with_low_contrast);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, false, 0.40f, 0.42f);
    CHECK(!AccessibilityBaseline::focusRingVisible(ring), "ring should fail visibility");
    PASS();
}

void test_validate_passes_for_accessible_baseline() {
    TEST(validate_passes_for_accessible_baseline);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, true, 0.1f, 0.9f);
    auto report = AccessibilityBaseline::validate(baseNodes(), ring, 7.0f, 4.0f);
    CHECK(report.pass, "baseline should pass");
    PASS();
}

void test_validate_fails_when_no_focusable_controls() {
    TEST(validate_fails_when_no_focusable_controls);
    auto nodes = baseNodes();
    for (auto& n : nodes) n.focusable = false;
    auto ring = AccessibilityBaseline::resolveFocusRing(true, true, 0.1f, 0.9f);
    auto report = AccessibilityBaseline::validate(nodes, ring, 7.0f, 4.0f);
    CHECK(!report.pass, "expected failure");
    CHECK(!report.failures.empty(), "expected failure reasons");
    PASS();
}

void test_validate_fails_text_contrast_below_aa() {
    TEST(validate_fails_text_contrast_below_aa);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, true, 0.1f, 0.9f);
    auto report = AccessibilityBaseline::validate(baseNodes(), ring, 3.0f, 4.0f);
    CHECK(!report.pass, "should fail text contrast");
    PASS();
}

void test_validate_fails_icon_contrast_below_threshold() {
    TEST(validate_fails_icon_contrast_below_threshold);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, true, 0.1f, 0.9f);
    auto report = AccessibilityBaseline::validate(baseNodes(), ring, 7.0f, 2.0f);
    CHECK(!report.pass, "should fail icon contrast");
    PASS();
}

void test_validate_fails_when_focus_ring_not_visible() {
    TEST(validate_fails_when_focus_ring_not_visible);
    auto ring = AccessibilityBaseline::resolveFocusRing(true, false, 0.4f, 0.42f);
    auto report = AccessibilityBaseline::validate(baseNodes(), ring, 7.0f, 4.0f);
    CHECK(!report.pass, "should fail hidden focus ring");
    PASS();
}

int main() {
    std::cout << "Step 513: Accessibility Baseline Pass\n";

    test_traversal_order_uses_tab_order();                 // 1
    test_traversal_skips_disabled_nodes();                 // 2
    test_traversal_skips_invisible_nodes();                // 3
    test_focus_ring_resolves_for_focused_control();        // 4
    test_focus_ring_disabled_when_not_focused();           // 5
    test_focus_ring_visible_with_good_contrast();          // 6
    test_focus_ring_not_visible_with_low_contrast();       // 7
    test_validate_passes_for_accessible_baseline();        // 8
    test_validate_fails_when_no_focusable_controls();      // 9
    test_validate_fails_text_contrast_below_aa();          // 10
    test_validate_fails_icon_contrast_below_threshold();   // 11
    test_validate_fails_when_focus_ring_not_visible();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
