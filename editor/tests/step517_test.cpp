// Step 517: Control Library Restyle (12 tests)

#include "ControlLibraryRestyle.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_button_style_has_positive_depth() {
    TEST(button_style_has_positive_depth);
    auto s = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, false);
    CHECK(s.depthIdle > 0.0f, "idle depth should be positive");
    PASS();
}

void test_pressed_state_increases_depth_signal() {
    TEST(pressed_state_increases_depth_signal);
    auto idle = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, false);
    auto pressed = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, true);
    CHECK(ControlLibraryRestyle::hasClearPressedSignal(idle, pressed),
          "pressed depth signal not clear");
    PASS();
}

void test_toggle_has_larger_radius_than_tab() {
    TEST(toggle_has_larger_radius_than_tab);
    auto t = ControlLibraryRestyle::styleFor(ControlKind::Toggle, 1.0f, false);
    auto tab = ControlLibraryRestyle::styleFor(ControlKind::Tab, 1.0f, false);
    CHECK(t.cornerRadius > tab.cornerRadius, "toggle radius should be larger");
    PASS();
}

void test_checkbox_has_contrast_compliance() {
    TEST(checkbox_has_contrast_compliance);
    auto s = ControlLibraryRestyle::styleFor(ControlKind::Checkbox, 1.0f, false);
    CHECK(ControlLibraryRestyle::validContrast(s), "checkbox contrast invalid");
    PASS();
}

void test_dropdown_has_nonzero_border_width() {
    TEST(dropdown_has_nonzero_border_width);
    auto s = ControlLibraryRestyle::styleFor(ControlKind::Dropdown, 1.0f, false);
    CHECK(s.borderWidth > 0.0f, "dropdown border width should be > 0");
    PASS();
}

void test_menu_style_uses_small_radius() {
    TEST(menu_style_uses_small_radius);
    auto s = ControlLibraryRestyle::styleFor(ControlKind::Menu, 1.0f, false);
    CHECK(s.cornerRadius <= 3.0f, "menu radius should be compact");
    PASS();
}

void test_dpi_scale_increases_geometry() {
    TEST(dpi_scale_increases_geometry);
    auto lo = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, false);
    auto hi = ControlLibraryRestyle::styleFor(ControlKind::Button, 2.0f, false);
    CHECK(ControlLibraryRestyle::dpiStable(lo, hi), "DPI scaling not stable");
    PASS();
}

void test_dpi_scale_clamps_lower_bound() {
    TEST(dpi_scale_clamps_lower_bound);
    auto a = ControlLibraryRestyle::styleFor(ControlKind::Button, 0.2f, false);
    auto b = ControlLibraryRestyle::styleFor(ControlKind::Button, 0.75f, false);
    CHECK(a.cornerRadius == b.cornerRadius, "low DPI should clamp to floor");
    PASS();
}

void test_dpi_scale_clamps_upper_bound() {
    TEST(dpi_scale_clamps_upper_bound);
    auto a = ControlLibraryRestyle::styleFor(ControlKind::Button, 4.5f, false);
    auto b = ControlLibraryRestyle::styleFor(ControlKind::Button, 3.0f, false);
    CHECK(a.cornerRadius == b.cornerRadius, "high DPI should clamp to ceiling");
    PASS();
}

void test_tab_pressed_signal_exists() {
    TEST(tab_pressed_signal_exists);
    auto idle = ControlLibraryRestyle::styleFor(ControlKind::Tab, 1.0f, false);
    auto pressed = ControlLibraryRestyle::styleFor(ControlKind::Tab, 1.0f, true);
    CHECK(ControlLibraryRestyle::hasClearPressedSignal(idle, pressed),
          "tab pressed signal should be clear");
    PASS();
}

void test_icon_contrast_meets_threshold() {
    TEST(icon_contrast_meets_threshold);
    auto s = ControlLibraryRestyle::styleFor(ControlKind::Button, 1.0f, false);
    CHECK(s.iconContrast >= 3.0f, "icon contrast below threshold");
    PASS();
}

void test_all_controls_have_valid_contrast() {
    TEST(all_controls_have_valid_contrast);
    bool ok = true;
    for (int i = 0; i < 6; ++i) {
        auto s = ControlLibraryRestyle::styleFor(static_cast<ControlKind>(i), 1.0f, false);
        if (!ControlLibraryRestyle::validContrast(s)) ok = false;
    }
    CHECK(ok, "one or more controls fail contrast");
    PASS();
}

int main() {
    std::cout << "Step 517: Control Library Restyle\n";

    test_button_style_has_positive_depth();          // 1
    test_pressed_state_increases_depth_signal();     // 2
    test_toggle_has_larger_radius_than_tab();        // 3
    test_checkbox_has_contrast_compliance();         // 4
    test_dropdown_has_nonzero_border_width();        // 5
    test_menu_style_uses_small_radius();             // 6
    test_dpi_scale_increases_geometry();             // 7
    test_dpi_scale_clamps_lower_bound();             // 8
    test_dpi_scale_clamps_upper_bound();             // 9
    test_tab_pressed_signal_exists();                // 10
    test_icon_contrast_meets_threshold();            // 11
    test_all_controls_have_valid_contrast();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
