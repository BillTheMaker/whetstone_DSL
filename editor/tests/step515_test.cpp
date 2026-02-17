// Step 515: Theme Token System v2 (12 tests)

#include "ThemeTokenSystemV2.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_base_theme_uses_black_stone_layers() {
    TEST(base_theme_uses_black_stone_layers);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    CHECK(t.bgBase.r < 0.1f && t.bgLayer2.r > t.bgBase.r, "expected layered dark theme");
    PASS();
}

void test_spacing_scale_has_expected_keys() {
    TEST(spacing_scale_has_expected_keys);
    auto s = ThemeTokenSystemV2::spacingScale();
    CHECK(s.count("xs") && s.count("md") && s.count("xl"), "missing spacing keys");
    PASS();
}

void test_spacing_scale_is_monotonic() {
    TEST(spacing_scale_is_monotonic);
    auto s = ThemeTokenSystemV2::spacingScale();
    CHECK(s["xs"] < s["sm"] && s["sm"] < s["md"] && s["md"] < s["lg"] && s["lg"] < s["xl"],
          "spacing scale should be monotonic");
    PASS();
}

void test_derived_variant_increases_text_brightness() {
    TEST(derived_variant_increases_text_brightness);
    auto base = ThemeTokenSystemV2::baseBlackStone();
    auto v = ThemeTokenSystemV2::deriveContrastVariant(base, 0.05f, 0.02f);
    CHECK(v.textPrimary.r >= base.textPrimary.r, "primary text should brighten");
    PASS();
}

void test_derived_variant_lifts_layers() {
    TEST(derived_variant_lifts_layers);
    auto base = ThemeTokenSystemV2::baseBlackStone();
    auto v = ThemeTokenSystemV2::deriveContrastVariant(base, 0.05f, 0.02f);
    CHECK(v.bgLayer1.r >= base.bgLayer1.r && v.bgLayer2.r >= base.bgLayer2.r,
          "layers should lift");
    PASS();
}

void test_base_theme_passes_aa_validation() {
    TEST(base_theme_passes_aa_validation);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    CHECK(ThemeTokenSystemV2::validateAA(t), "base theme should pass AA");
    PASS();
}

void test_contrast_ratio_primary_text_high_enough() {
    TEST(contrast_ratio_primary_text_high_enough);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    auto ratio = ThemeTokenSystemV2::contrastTextVsBg(t.textPrimary, t.bgBase);
    CHECK(ratio >= 4.5f, "primary contrast below AA");
    PASS();
}

void test_no_default_purple_bias() {
    TEST(no_default_purple_bias);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    CHECK(!ThemeTokenSystemV2::hasHardcodedPurple(t), "unexpected purple bias");
    PASS();
}

void test_detects_purple_bias_when_present() {
    TEST(detects_purple_bias_when_present);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    t.accentPrimary = {0.55f, 0.25f, 0.85f, 1.0f};
    CHECK(ThemeTokenSystemV2::hasHardcodedPurple(t), "should detect purple bias");
    PASS();
}

void test_radius_scale_is_ordered() {
    TEST(radius_scale_is_ordered);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    CHECK(t.radiusSmall < t.radiusMedium, "radius scale invalid");
    PASS();
}

void test_border_scale_is_ordered() {
    TEST(border_scale_is_ordered);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    CHECK(t.borderThin < t.borderThick, "border scale invalid");
    PASS();
}

void test_elevation_scale_is_ordered() {
    TEST(elevation_scale_is_ordered);
    auto t = ThemeTokenSystemV2::baseBlackStone();
    CHECK(t.elevationLow < t.elevationHigh, "elevation scale invalid");
    PASS();
}

int main() {
    std::cout << "Step 515: Theme Token System v2\n";

    test_base_theme_uses_black_stone_layers();           // 1
    test_spacing_scale_has_expected_keys();              // 2
    test_spacing_scale_is_monotonic();                   // 3
    test_derived_variant_increases_text_brightness();    // 4
    test_derived_variant_lifts_layers();                 // 5
    test_base_theme_passes_aa_validation();              // 6
    test_contrast_ratio_primary_text_high_enough();      // 7
    test_no_default_purple_bias();                       // 8
    test_detects_purple_bias_when_present();             // 9
    test_radius_scale_is_ordered();                      // 10
    test_border_scale_is_ordered();                      // 11
    test_elevation_scale_is_ordered();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
