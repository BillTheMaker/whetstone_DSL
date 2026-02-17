// Step 510: Panel Boundaries, Resizers, and Hit-Target Corrections (12 tests)

#include "PanelBoundaryReliability.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<PanelRect> basePanels() {
    return {
        {"left", 0, 0, 220, 800},
        {"editor", 220, 0, 760, 800},
        {"right", 980, 0, 220, 800}
    };
}

static std::vector<SplitBar> baseBars() {
    return {
        {"left-editor", true, 220, 6, "left", "editor"},
        {"editor-right", true, 980, 6, "editor", "right"}
    };
}

void test_resize_band_takes_priority_over_content() {
    TEST(resize_band_takes_priority_over_content);
    auto hit = PanelBoundaryReliability::resolveClick(221, 100, basePanels(), baseBars());
    CHECK(hit.hit && hit.resizeHit, "expected resize hit");
    CHECK(hit.targetId == "left-editor", "wrong bar target");
    PASS();
}

void test_content_hit_when_away_from_split_bar() {
    TEST(content_hit_when_away_from_split_bar);
    auto hit = PanelBoundaryReliability::resolveClick(400, 100, basePanels(), baseBars());
    CHECK(hit.hit && !hit.resizeHit, "expected content hit");
    CHECK(hit.targetId == "editor", "expected editor panel hit");
    PASS();
}

void test_miss_returns_none() {
    TEST(miss_returns_none);
    auto hit = PanelBoundaryReliability::resolveClick(1300, 900, basePanels(), baseBars());
    CHECK(!hit.hit, "expected miss");
    CHECK(hit.reason == "none", "expected reason none");
    PASS();
}

void test_panel_width_is_clamped_to_min() {
    TEST(panel_width_is_clamped_to_min);
    auto panels = basePanels();
    panels[0].w = 20;
    auto fixed = PanelBoundaryReliability::normalizeGeometry(panels, baseBars(), 120);
    CHECK(fixed.panels[0].w == 120, "width should be clamped");
    PASS();
}

void test_panel_height_is_clamped_to_min() {
    TEST(panel_height_is_clamped_to_min);
    auto panels = basePanels();
    panels[1].h = 80;
    auto fixed = PanelBoundaryReliability::normalizeGeometry(panels, baseBars(), 120);
    CHECK(fixed.panels[1].h == 120, "height should be clamped");
    PASS();
}

void test_split_bar_thickness_clamped_low() {
    TEST(split_bar_thickness_clamped_low);
    auto bars = baseBars();
    bars[0].thickness = 1;
    auto fixed = PanelBoundaryReliability::normalizeGeometry(basePanels(), bars);
    CHECK(fixed.bars[0].thickness == 5, "bar thickness should clamp to min");
    PASS();
}

void test_split_bar_thickness_clamped_high() {
    TEST(split_bar_thickness_clamped_high);
    auto bars = baseBars();
    bars[1].thickness = 40;
    auto fixed = PanelBoundaryReliability::normalizeGeometry(basePanels(), bars);
    CHECK(fixed.bars[1].thickness == 14, "bar thickness should clamp to max");
    PASS();
}

void test_keyboard_resize_moves_split_bar() {
    TEST(keyboard_resize_moves_split_bar);
    auto moved = PanelBoundaryReliability::keyboardResize(baseBars()[0], 10, 100, 400, false);
    CHECK(moved.pos == 230, "split bar should move by delta");
    PASS();
}

void test_keyboard_resize_respects_bounds() {
    TEST(keyboard_resize_respects_bounds);
    auto moved = PanelBoundaryReliability::keyboardResize(baseBars()[0], -500, 150, 400, false);
    CHECK(moved.pos == 150, "split bar should clamp to min bound");
    PASS();
}

void test_keyboard_resize_fine_step_halves_delta() {
    TEST(keyboard_resize_fine_step_halves_delta);
    auto moved = PanelBoundaryReliability::keyboardResize(baseBars()[0], 8, 100, 400, true);
    CHECK(moved.pos == 224, "fine step should halve delta");
    PASS();
}

void test_navigation_order_prefers_panels_near_bar() {
    TEST(navigation_order_prefers_panels_near_bar);
    auto ids = PanelBoundaryReliability::navigationOrderByBarProximity(basePanels(), baseBars()[0]);
    CHECK(!ids.empty(), "expected non-empty order");
    CHECK(ids[0] == "left" || ids[0] == "editor", "nearest should be left or editor");
    PASS();
}

void test_normalize_emits_fix_notes() {
    TEST(normalize_emits_fix_notes);
    auto panels = basePanels();
    auto bars = baseBars();
    panels[0].w = 50;
    bars[0].thickness = 2;
    auto fixed = PanelBoundaryReliability::normalizeGeometry(panels, bars, 120);
    CHECK(!fixed.notes.empty(), "expected fix notes");
    PASS();
}

int main() {
    std::cout << "Step 510: Panel Boundaries, Resizers, and Hit-Target Corrections\n";

    test_resize_band_takes_priority_over_content();      // 1
    test_content_hit_when_away_from_split_bar();         // 2
    test_miss_returns_none();                            // 3
    test_panel_width_is_clamped_to_min();                // 4
    test_panel_height_is_clamped_to_min();               // 5
    test_split_bar_thickness_clamped_low();              // 6
    test_split_bar_thickness_clamped_high();             // 7
    test_keyboard_resize_moves_split_bar();              // 8
    test_keyboard_resize_respects_bounds();              // 9
    test_keyboard_resize_fine_step_halves_delta();       // 10
    test_navigation_order_prefers_panels_near_bar();     // 11
    test_normalize_emits_fix_notes();                    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
