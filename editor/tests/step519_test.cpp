// Step 519: Modifier-Edge Shortcut Notation (12 tests)

#include "ModifierEdgeNotation.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_ctrl_maps_to_left_edge() {
    TEST(ctrl_maps_to_left_edge);
    auto e = ModifierEdgeNotation::fromModifiers(true, false, false, false);
    CHECK(e.left && !e.right && !e.top && !e.bottom, "ctrl edge mapping invalid");
    PASS();
}

void test_shift_maps_to_right_edge() {
    TEST(shift_maps_to_right_edge);
    auto e = ModifierEdgeNotation::fromModifiers(false, true, false, false);
    CHECK(e.right, "shift should map to right edge");
    PASS();
}

void test_super_maps_to_top_edge() {
    TEST(super_maps_to_top_edge);
    auto e = ModifierEdgeNotation::fromModifiers(false, false, true, false);
    CHECK(e.top, "super should map to top edge");
    PASS();
}

void test_alt_maps_to_bottom_edge() {
    TEST(alt_maps_to_bottom_edge);
    auto e = ModifierEdgeNotation::fromModifiers(false, false, false, true);
    CHECK(e.bottom, "alt should map to bottom edge");
    PASS();
}

void test_combined_modifiers_set_multiple_edges() {
    TEST(combined_modifiers_set_multiple_edges);
    auto e = ModifierEdgeNotation::fromModifiers(true, true, false, true);
    CHECK(ModifierEdgeNotation::activeEdgeCount(e) == 3, "expected 3 active edges");
    PASS();
}

void test_key_is_normalized_to_uppercase() {
    TEST(key_is_normalized_to_uppercase);
    auto g = ModifierEdgeNotation::buildGlyph('k', true, false, false, false, true);
    CHECK(g.key == 'K', "key should normalize to uppercase");
    PASS();
}

void test_invalid_key_becomes_placeholder() {
    TEST(invalid_key_becomes_placeholder);
    auto g = ModifierEdgeNotation::buildGlyph('%', true, false, false, false, true);
    CHECK(g.key == '?', "invalid key should become placeholder");
    PASS();
}

void test_fallback_label_contains_modifiers_and_key() {
    TEST(fallback_label_contains_modifiers_and_key);
    auto g = ModifierEdgeNotation::buildGlyph('P', true, true, false, false, true);
    CHECK(g.fallbackText == "Ctrl+Shift+P", "fallback label mismatch");
    PASS();
}

void test_glyph_disabled_uses_fallback_no_edges() {
    TEST(glyph_disabled_uses_fallback_no_edges);
    auto g = ModifierEdgeNotation::buildGlyph('P', true, true, false, false, false);
    CHECK(ModifierEdgeNotation::activeEdgeCount(g.edges) == 0, "edges should be cleared");
    CHECK(g.fallbackText == "Ctrl+Shift+P", "fallback should remain available");
    PASS();
}

void test_compact_check_passes_for_normal_shortcut() {
    TEST(compact_check_passes_for_normal_shortcut);
    auto g = ModifierEdgeNotation::buildGlyph('A', true, false, false, false, true);
    CHECK(ModifierEdgeNotation::compactEnough(g), "shortcut should be compact");
    PASS();
}

void test_active_edge_count_zero_without_modifiers() {
    TEST(active_edge_count_zero_without_modifiers);
    auto e = ModifierEdgeNotation::fromModifiers(false, false, false, false);
    CHECK(ModifierEdgeNotation::activeEdgeCount(e) == 0, "edge count should be zero");
    PASS();
}

void test_super_alt_combo_fallback_order_is_stable() {
    TEST(super_alt_combo_fallback_order_is_stable);
    auto g = ModifierEdgeNotation::buildGlyph('X', false, false, true, true, true);
    CHECK(g.fallbackText == "Super+Alt+X", "modifier ordering should be stable");
    PASS();
}

int main() {
    std::cout << "Step 519: Modifier-Edge Shortcut Notation\n";

    test_ctrl_maps_to_left_edge();                        // 1
    test_shift_maps_to_right_edge();                      // 2
    test_super_maps_to_top_edge();                        // 3
    test_alt_maps_to_bottom_edge();                       // 4
    test_combined_modifiers_set_multiple_edges();         // 5
    test_key_is_normalized_to_uppercase();                // 6
    test_invalid_key_becomes_placeholder();               // 7
    test_fallback_label_contains_modifiers_and_key();     // 8
    test_glyph_disabled_uses_fallback_no_edges();         // 9
    test_compact_check_passes_for_normal_shortcut();      // 10
    test_active_edge_count_zero_without_modifiers();      // 11
    test_super_alt_combo_fallback_order_is_stable();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
