// Step 516: Signature Visual Identity Pack (12 tests)

#include "SignatureVisualIdentity.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_gradient_has_three_stops() {
    TEST(gradient_has_three_stops);
    auto g = SignatureVisualIdentity::blackSurfaceGradient(false);
    CHECK(g.size() == 3, "expected 3 stops");
    PASS();
}

void test_elevated_gradient_brighter_than_base() {
    TEST(elevated_gradient_brighter_than_base);
    auto a = SignatureVisualIdentity::blackSurfaceGradient(false);
    auto b = SignatureVisualIdentity::blackSurfaceGradient(true);
    CHECK(b.back().r > a.back().r, "elevated gradient should be brighter");
    PASS();
}

void test_blade_edge_active_increases_thickness() {
    TEST(blade_edge_active_increases_thickness);
    auto idle = SignatureVisualIdentity::bladeEdge("left", false, 0.5f);
    auto active = SignatureVisualIdentity::bladeEdge("left", true, 0.5f);
    CHECK(active.thickness > idle.thickness, "active edge should be thicker");
    PASS();
}

void test_blade_edge_active_uses_higher_intensity() {
    TEST(blade_edge_active_uses_higher_intensity);
    auto idle = SignatureVisualIdentity::bladeEdge("left", false, 0.5f);
    auto active = SignatureVisualIdentity::bladeEdge("left", true, 0.7f);
    CHECK(active.intensity > idle.intensity, "active edge should be stronger");
    PASS();
}

void test_logic_link_highlight_increases_width() {
    TEST(logic_link_highlight_increases_width);
    auto normal = SignatureVisualIdentity::electricBlueLink(false, 0.1f);
    auto hi = SignatureVisualIdentity::electricBlueLink(true, 0.1f);
    CHECK(hi.width > normal.width, "highlight should increase width");
    PASS();
}

void test_logic_link_highlight_increases_glow() {
    TEST(logic_link_highlight_increases_glow);
    auto normal = SignatureVisualIdentity::electricBlueLink(false, 0.1f);
    auto hi = SignatureVisualIdentity::electricBlueLink(true, 0.1f);
    CHECK(hi.glow > normal.glow, "highlight should increase glow");
    PASS();
}

void test_logic_link_alpha_fades_with_distance() {
    TEST(logic_link_alpha_fades_with_distance);
    auto near = SignatureVisualIdentity::electricBlueLink(false, 0.0f);
    auto far = SignatureVisualIdentity::electricBlueLink(false, 1.0f);
    CHECK(near.alpha > far.alpha, "alpha should fade with distance");
    PASS();
}

void test_logic_link_is_electric_blue() {
    TEST(logic_link_is_electric_blue);
    auto s = SignatureVisualIdentity::electricBlueLink(true, 0.3f);
    CHECK(SignatureVisualIdentity::isElectricBlue(s), "expected electric-blue profile");
    PASS();
}

void test_reveal_duration_shorter_in_reduced_motion() {
    TEST(reveal_duration_shorter_in_reduced_motion);
    auto normal = SignatureVisualIdentity::revealDurationMs(false);
    auto reduced = SignatureVisualIdentity::revealDurationMs(true);
    CHECK(reduced < normal, "reduced-motion reveal should be shorter");
    PASS();
}

void test_stagger_delay_zero_in_reduced_motion() {
    TEST(stagger_delay_zero_in_reduced_motion);
    CHECK(SignatureVisualIdentity::staggerDelayMs(3, true) == 0.0f,
          "reduced-motion stagger should be zero");
    PASS();
}

void test_stagger_delay_grows_with_index() {
    TEST(stagger_delay_grows_with_index);
    auto a = SignatureVisualIdentity::staggerDelayMs(1, false);
    auto b = SignatureVisualIdentity::staggerDelayMs(4, false);
    CHECK(b > a, "stagger should increase with index");
    PASS();
}

void test_gradient_t_values_are_ordered() {
    TEST(gradient_t_values_are_ordered);
    auto g = SignatureVisualIdentity::blackSurfaceGradient(true);
    CHECK(g[0].t < g[1].t && g[1].t < g[2].t, "gradient t-values must be ordered");
    PASS();
}

int main() {
    std::cout << "Step 516: Signature Visual Identity Pack\n";

    test_gradient_has_three_stops();                        // 1
    test_elevated_gradient_brighter_than_base();            // 2
    test_blade_edge_active_increases_thickness();           // 3
    test_blade_edge_active_uses_higher_intensity();         // 4
    test_logic_link_highlight_increases_width();            // 5
    test_logic_link_highlight_increases_glow();             // 6
    test_logic_link_alpha_fades_with_distance();            // 7
    test_logic_link_is_electric_blue();                     // 8
    test_reveal_duration_shorter_in_reduced_motion();       // 9
    test_stagger_delay_zero_in_reduced_motion();            // 10
    test_stagger_delay_grows_with_index();                  // 11
    test_gradient_t_values_are_ordered();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
