// Step 511: Interaction State Model (12 tests)

#include "InteractionStateModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_state_priority_disabled_wins() {
    TEST(state_priority_disabled_wins);
    InteractionSnapshot s{true, true, true, true, true};
    CHECK(InteractionStateModel::resolveState(s) == InteractionState::Disabled,
          "disabled should dominate");
    PASS();
}

void test_state_priority_pressed_over_active_focus_hover() {
    TEST(state_priority_pressed_over_active_focus_hover);
    InteractionSnapshot s{true, true, true, true, false};
    CHECK(InteractionStateModel::resolveState(s) == InteractionState::Pressed,
          "pressed should dominate active/focus/hover");
    PASS();
}

void test_state_priority_active_over_focus_hover() {
    TEST(state_priority_active_over_focus_hover);
    InteractionSnapshot s{true, true, true, false, false};
    CHECK(InteractionStateModel::resolveState(s) == InteractionState::Active,
          "active should dominate focus/hover");
    PASS();
}

void test_focus_state_sets_focus_ring() {
    TEST(focus_state_sets_focus_ring);
    auto style = InteractionStateModel::resolveStyle({false, true, false, false, false},
                                                     InteractionStateModel::defaultDarkTokens());
    CHECK(style.state == InteractionState::Focus, "expected focus state");
    CHECK(style.showFocusRing, "focus ring should be enabled");
    PASS();
}

void test_disabled_state_uses_disabled_text() {
    TEST(disabled_state_uses_disabled_text);
    auto t = InteractionStateModel::defaultDarkTokens();
    auto style = InteractionStateModel::resolveStyle({false, false, false, false, true}, t);
    CHECK(style.state == InteractionState::Disabled, "expected disabled state");
    CHECK(style.text.r == t.textDisabled.r && style.text.g == t.textDisabled.g,
          "disabled text token not applied");
    PASS();
}

void test_idle_state_uses_base_surface() {
    TEST(idle_state_uses_base_surface);
    auto t = InteractionStateModel::defaultDarkTokens();
    auto style = InteractionStateModel::resolveStyle({}, t);
    CHECK(style.state == InteractionState::Idle, "expected idle state");
    CHECK(style.surface.r == t.surface.r && style.surface.g == t.surface.g,
          "idle surface token not applied");
    PASS();
}

void test_resolve_for_all_states_returns_six_entries() {
    TEST(resolve_for_all_states_returns_six_entries);
    auto all = InteractionStateModel::resolveForAllStates(
        InteractionStateModel::defaultDarkTokens());
    CHECK(all.size() == 6, "expected 6 state entries");
    PASS();
}

void test_default_tokens_pass_contrast_for_core_states() {
    TEST(default_tokens_pass_contrast_for_core_states);
    auto all = InteractionStateModel::resolveForAllStates(
        InteractionStateModel::defaultDarkTokens());
    CHECK(all["idle"].contrastPassAA, "idle contrast should pass");
    CHECK(all["hover"].contrastPassAA, "hover contrast should pass");
    CHECK(all["focus"].contrastPassAA, "focus contrast should pass");
    PASS();
}

void test_contrast_failures_detect_low_contrast_tokens() {
    TEST(contrast_failures_detect_low_contrast_tokens);
    auto t = InteractionStateModel::defaultDarkTokens();
    t.text = {0.2f, 0.2f, 0.2f, 1.0f};
    t.surface = {0.18f, 0.18f, 0.18f, 1.0f};
    auto fails = InteractionStateModel::contrastFailures(t);
    CHECK(!fails.empty(), "expected contrast failures");
    PASS();
}

void test_focus_ring_not_shown_for_non_focus_states() {
    TEST(focus_ring_not_shown_for_non_focus_states);
    auto t = InteractionStateModel::defaultDarkTokens();
    auto hover = InteractionStateModel::resolveStyle({true, false, false, false, false}, t);
    auto active = InteractionStateModel::resolveStyle({true, true, true, false, false}, t);
    CHECK(!hover.showFocusRing, "hover should not show focus ring");
    CHECK(!active.showFocusRing, "active should not show focus ring");
    PASS();
}

void test_pressed_and_active_surfaces_differ() {
    TEST(pressed_and_active_surfaces_differ);
    auto t = InteractionStateModel::defaultDarkTokens();
    auto pressed = InteractionStateModel::resolveStyle({true, true, true, true, false}, t);
    auto active = InteractionStateModel::resolveStyle({true, true, true, false, false}, t);
    CHECK(pressed.surface.r != active.surface.r ||
          pressed.surface.g != active.surface.g ||
          pressed.surface.b != active.surface.b,
          "pressed and active surfaces should differ");
    PASS();
}

void test_contrast_ratio_is_positive() {
    TEST(contrast_ratio_is_positive);
    auto style = InteractionStateModel::resolveStyle({},
        InteractionStateModel::defaultDarkTokens());
    CHECK(style.contrastRatio > 1.0f, "contrast ratio should be > 1");
    PASS();
}

int main() {
    std::cout << "Step 511: Interaction State Model\n";

    test_state_priority_disabled_wins();                      // 1
    test_state_priority_pressed_over_active_focus_hover();    // 2
    test_state_priority_active_over_focus_hover();            // 3
    test_focus_state_sets_focus_ring();                       // 4
    test_disabled_state_uses_disabled_text();                 // 5
    test_idle_state_uses_base_surface();                      // 6
    test_resolve_for_all_states_returns_six_entries();        // 7
    test_default_tokens_pass_contrast_for_core_states();      // 8
    test_contrast_failures_detect_low_contrast_tokens();      // 9
    test_focus_ring_not_shown_for_non_focus_states();         // 10
    test_pressed_and_active_surfaces_differ();                // 11
    test_contrast_ratio_is_positive();                        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
