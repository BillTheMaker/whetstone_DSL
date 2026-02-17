// Step 522: Cross-Panel Consistency Sweep (12 tests)

#include "CrossPanelConsistencySweep.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static PanelVisualContract baseline() {
    return {"baseline", 12.0f, 6.0f, 1.0f, "v2", "v2"};
}

static std::vector<PanelVisualContract> alignedPanels() {
    return {
        {"explorer", 12.0f, 6.0f, 1.0f, "v2", "v2"},
        {"editor", 12.0f, 6.0f, 1.0f, "v2", "v2"},
        {"status", 12.0f, 6.0f, 1.0f, "v2", "v2"}
    };
}

void test_passes_when_all_panels_aligned() {
    TEST(passes_when_all_panels_aligned);
    auto r = CrossPanelConsistencySweep::run(alignedPanels(), baseline());
    CHECK(r.pass, "aligned panels should pass");
    PASS();
}

void test_detects_spacing_drift() {
    TEST(detects_spacing_drift);
    auto p = alignedPanels();
    p[0].spacing = 20.0f;
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    CHECK(!r.pass, "spacing drift should fail");
    PASS();
}

void test_detects_elevation_drift() {
    TEST(detects_elevation_drift);
    auto p = alignedPanels();
    p[1].elevation = 10.0f;
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    CHECK(!r.pass, "elevation drift should fail");
    PASS();
}

void test_detects_border_drift() {
    TEST(detects_border_drift);
    auto p = alignedPanels();
    p[2].border = 3.0f;
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    CHECK(!r.pass, "border drift should fail");
    PASS();
}

void test_detects_state_model_version_drift() {
    TEST(detects_state_model_version_drift);
    auto p = alignedPanels();
    p[1].stateModelVersion = "v1";
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    CHECK(!r.pass, "state model drift should fail");
    PASS();
}

void test_detects_token_version_drift() {
    TEST(detects_token_version_drift);
    auto p = alignedPanels();
    p[1].tokenVersion = "v1";
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    CHECK(!r.pass, "token version drift should fail");
    PASS();
}

void test_normalize_resets_spacing_to_baseline() {
    TEST(normalize_resets_spacing_to_baseline);
    auto p = alignedPanels();
    p[0].spacing = 99.0f;
    auto n = CrossPanelConsistencySweep::normalize(p, baseline());
    CHECK(n[0].spacing == 12.0f, "spacing should normalize");
    PASS();
}

void test_normalize_resets_versions_to_baseline() {
    TEST(normalize_resets_versions_to_baseline);
    auto p = alignedPanels();
    p[0].stateModelVersion = "old";
    p[0].tokenVersion = "old";
    auto n = CrossPanelConsistencySweep::normalize(p, baseline());
    CHECK(n[0].stateModelVersion == "v2" && n[0].tokenVersion == "v2",
          "versions should normalize");
    PASS();
}

void test_histogram_groups_drift_categories() {
    TEST(histogram_groups_drift_categories);
    auto p = alignedPanels();
    p[0].spacing = 30.0f;
    p[1].spacing = 25.0f;
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    auto h = CrossPanelConsistencySweep::driftHistogram(r);
    CHECK(h["spacing"] == 2, "histogram should count spacing drifts");
    PASS();
}

void test_report_checked_panel_count_matches_input() {
    TEST(report_checked_panel_count_matches_input);
    auto p = alignedPanels();
    auto r = CrossPanelConsistencySweep::run(p, baseline());
    CHECK(r.checkedPanels == 3, "checked panel count mismatch");
    PASS();
}

void test_tolerance_allows_small_spacing_variance() {
    TEST(tolerance_allows_small_spacing_variance);
    auto p = alignedPanels();
    p[0].spacing = 12.6f;
    auto r = CrossPanelConsistencySweep::run(p, baseline(), 1.0f);
    CHECK(r.pass, "small spacing variance should pass");
    PASS();
}

void test_tolerance_rejects_large_spacing_variance() {
    TEST(tolerance_rejects_large_spacing_variance);
    auto p = alignedPanels();
    p[0].spacing = 14.5f;
    auto r = CrossPanelConsistencySweep::run(p, baseline(), 1.0f);
    CHECK(!r.pass, "large spacing variance should fail");
    PASS();
}

int main() {
    std::cout << "Step 522: Cross-Panel Consistency Sweep\n";

    test_passes_when_all_panels_aligned();              // 1
    test_detects_spacing_drift();                       // 2
    test_detects_elevation_drift();                     // 3
    test_detects_border_drift();                        // 4
    test_detects_state_model_version_drift();           // 5
    test_detects_token_version_drift();                 // 6
    test_normalize_resets_spacing_to_baseline();        // 7
    test_normalize_resets_versions_to_baseline();       // 8
    test_histogram_groups_drift_categories();           // 9
    test_report_checked_panel_count_matches_input();    // 10
    test_tolerance_allows_small_spacing_variance();     // 11
    test_tolerance_rejects_large_spacing_variance();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
