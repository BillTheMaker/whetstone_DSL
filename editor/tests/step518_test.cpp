// Step 518: Icon + Typography Contrast Harmonization (12 tests)

#include "IconTypographyHarmonizer.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_typography_scales_with_density() {
    TEST(typography_scales_with_density);
    auto a = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    auto b = IconTypographyHarmonizer::darkSurfaceTypography(1.3f);
    CHECK(b.bodySizePx > a.bodySizePx, "body size should scale up");
    PASS();
}

void test_density_clamps_low() {
    TEST(density_clamps_low);
    auto a = IconTypographyHarmonizer::darkSurfaceTypography(0.2f);
    auto b = IconTypographyHarmonizer::darkSurfaceTypography(0.8f);
    CHECK(a.bodySizePx == b.bodySizePx, "density should clamp low");
    PASS();
}

void test_density_clamps_high() {
    TEST(density_clamps_high);
    auto a = IconTypographyHarmonizer::darkSurfaceTypography(3.0f);
    auto b = IconTypographyHarmonizer::darkSurfaceTypography(1.4f);
    CHECK(a.bodySizePx == b.bodySizePx, "density should clamp high");
    PASS();
}

void test_heading_larger_than_body() {
    TEST(heading_larger_than_body);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    CHECK(t.headingSizePx > t.bodySizePx, "heading should be larger");
    PASS();
}

void test_text_contrast_meets_aa() {
    TEST(text_contrast_meets_aa);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    CHECK(t.textContrast >= 4.5f, "text contrast should meet AA");
    PASS();
}

void test_icon_profile_emphasized_has_higher_stroke() {
    TEST(icon_profile_emphasized_has_higher_stroke);
    auto a = IconTypographyHarmonizer::iconProfileFor(1.0f, false);
    auto b = IconTypographyHarmonizer::iconProfileFor(1.0f, true);
    CHECK(b.strokePx > a.strokePx, "emphasized icon stroke should be higher");
    PASS();
}

void test_icon_profile_has_min_contrast() {
    TEST(icon_profile_has_min_contrast);
    auto i = IconTypographyHarmonizer::iconProfileFor(1.0f, false);
    CHECK(i.contrast >= 3.0f, "icon contrast should meet threshold");
    PASS();
}

void test_harmonized_profile_passes() {
    TEST(harmonized_profile_passes);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    auto i = IconTypographyHarmonizer::iconProfileFor(1.0f, false);
    CHECK(IconTypographyHarmonizer::harmonized(t, i), "harmonization should pass");
    PASS();
}

void test_harmonized_fails_with_low_icon_contrast() {
    TEST(harmonized_fails_with_low_icon_contrast);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    IconProfile i = IconTypographyHarmonizer::iconProfileFor(1.0f, false);
    i.contrast = 2.0f;
    CHECK(!IconTypographyHarmonizer::harmonized(t, i), "harmonization should fail");
    PASS();
}

void test_rhythm_score_high_for_consistent_panels() {
    TEST(rhythm_score_high_for_consistent_panels);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    float r = IconTypographyHarmonizer::rhythmScore(t, {12.0f, 13.0f, 12.5f});
    CHECK(r > 0.7f, "rhythm should be high for consistent densities");
    PASS();
}

void test_rhythm_score_low_for_wide_spread() {
    TEST(rhythm_score_low_for_wide_spread);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    float r = IconTypographyHarmonizer::rhythmScore(t, {4.0f, 20.0f, 30.0f});
    CHECK(r < 0.5f, "rhythm should drop for wide spread");
    PASS();
}

void test_rhythm_zero_when_no_panels() {
    TEST(rhythm_zero_when_no_panels);
    auto t = IconTypographyHarmonizer::darkSurfaceTypography(1.0f);
    CHECK(IconTypographyHarmonizer::rhythmScore(t, {}) == 0.0f,
          "empty panel list should produce zero score");
    PASS();
}

int main() {
    std::cout << "Step 518: Icon + Typography Contrast Harmonization\n";

    test_typography_scales_with_density();                 // 1
    test_density_clamps_low();                             // 2
    test_density_clamps_high();                            // 3
    test_heading_larger_than_body();                       // 4
    test_text_contrast_meets_aa();                         // 5
    test_icon_profile_emphasized_has_higher_stroke();      // 6
    test_icon_profile_has_min_contrast();                  // 7
    test_harmonized_profile_passes();                      // 8
    test_harmonized_fails_with_low_icon_contrast();        // 9
    test_rhythm_score_high_for_consistent_panels();        // 10
    test_rhythm_score_low_for_wide_spread();               // 11
    test_rhythm_zero_when_no_panels();                     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
