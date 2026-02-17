// Step 520: Completion Overlay UX Hardening (12 tests)

#include "CompletionOverlayUXHardening.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_overlay_defaults_below_cursor() {
    TEST(overlay_defaults_below_cursor);
    auto r = CompletionOverlayUXHardening::placeOverlayNearCursor(100, 100, 18, 800, 600, 200, 120);
    CHECK(r.y >= 118, "overlay should appear below cursor");
    PASS();
}

void test_overlay_flips_above_when_bottom_overflow() {
    TEST(overlay_flips_above_when_bottom_overflow);
    auto r = CompletionOverlayUXHardening::placeOverlayNearCursor(100, 560, 18, 800, 600, 220, 120);
    CHECK(r.y < 560, "overlay should flip above");
    PASS();
}

void test_overlay_clamps_right_edge() {
    TEST(overlay_clamps_right_edge);
    auto r = CompletionOverlayUXHardening::placeOverlayNearCursor(760, 100, 18, 800, 600, 120, 100);
    CHECK(r.x + r.w <= 800, "overlay should clamp to right edge");
    PASS();
}

void test_overlay_never_negative_y() {
    TEST(overlay_never_negative_y);
    auto r = CompletionOverlayUXHardening::placeOverlayNearCursor(10, 5, 18, 200, 60, 190, 100);
    CHECK(r.y >= 0, "overlay y must remain non-negative");
    PASS();
}

void test_dismiss_hides_overlay_and_sets_flag() {
    TEST(dismiss_hides_overlay_and_sets_flag);
    CompletionOverlayState s{true, false, 0, "foo", {}};
    auto d = CompletionOverlayUXHardening::onDismiss(s);
    CHECK(!d.visible && d.dismissed, "dismiss state mismatch");
    PASS();
}

void test_trigger_with_same_token_keeps_dismissed() {
    TEST(trigger_with_same_token_keeps_dismissed);
    CompletionOverlayState s{false, true, 0, "foo", {}};
    auto n = CompletionOverlayUXHardening::onTrigger(s, "foo", false);
    CHECK(!n.visible, "overlay should stay hidden");
    PASS();
}

void test_trigger_with_new_token_reopens_overlay() {
    TEST(trigger_with_new_token_reopens_overlay);
    CompletionOverlayState s{false, true, 0, "foo", {}};
    auto n = CompletionOverlayUXHardening::onTrigger(s, "bar", false);
    CHECK(n.visible && !n.dismissed, "overlay should reopen on new token");
    PASS();
}

void test_trigger_with_semantic_change_reopens_overlay() {
    TEST(trigger_with_semantic_change_reopens_overlay);
    CompletionOverlayState s{false, true, 0, "foo", {}};
    auto n = CompletionOverlayUXHardening::onTrigger(s, "foo", true);
    CHECK(n.visible && !n.dismissed, "semantic change should reopen overlay");
    PASS();
}

void test_pointer_selection_clamps_index() {
    TEST(pointer_selection_clamps_index);
    CompletionOverlayState s{true, false, 0, "", {}};
    auto n = CompletionOverlayUXHardening::onPointerSelect(s, 99, 5);
    CHECK(n.selectedIndex == 4, "index should clamp to last item");
    PASS();
}

void test_pointer_selection_hides_when_no_items() {
    TEST(pointer_selection_hides_when_no_items);
    CompletionOverlayState s{true, false, 0, "", {}};
    auto n = CompletionOverlayUXHardening::onPointerSelect(s, 0, 0);
    CHECK(!n.visible, "overlay should hide with zero items");
    PASS();
}

void test_keyboard_nav_wraps_forward() {
    TEST(keyboard_nav_wraps_forward);
    CompletionOverlayState s{true, false, 4, "", {}};
    auto n = CompletionOverlayUXHardening::onKeyNav(s, 1, 5);
    CHECK(n.selectedIndex == 0, "forward nav should wrap");
    PASS();
}

void test_keyboard_nav_wraps_backward() {
    TEST(keyboard_nav_wraps_backward);
    CompletionOverlayState s{true, false, 0, "", {}};
    auto n = CompletionOverlayUXHardening::onKeyNav(s, -1, 5);
    CHECK(n.selectedIndex == 4, "backward nav should wrap");
    PASS();
}

int main() {
    std::cout << "Step 520: Completion Overlay UX Hardening\n";

    test_overlay_defaults_below_cursor();                 // 1
    test_overlay_flips_above_when_bottom_overflow();      // 2
    test_overlay_clamps_right_edge();                     // 3
    test_overlay_never_negative_y();                      // 4
    test_dismiss_hides_overlay_and_sets_flag();           // 5
    test_trigger_with_same_token_keeps_dismissed();       // 6
    test_trigger_with_new_token_reopens_overlay();        // 7
    test_trigger_with_semantic_change_reopens_overlay();  // 8
    test_pointer_selection_clamps_index();                // 9
    test_pointer_selection_hides_when_no_items();         // 10
    test_keyboard_nav_wraps_forward();                    // 11
    test_keyboard_nav_wraps_backward();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
