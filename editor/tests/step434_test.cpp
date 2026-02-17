// Step 434: Annotation Heatmap Tests (12 tests)

#include "CodeAnnotationHeatmap.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<AnnotationBadgeInput> sample() {
    return {
        {2, "IntentAnnotation", "intent", "n1"},
        {3, "ComplexityAnnotation", "complex", "n2"},
        {3, "RiskAnnotation", "risk", "n2"},
        {5, "ReviewAnnotation", "review", "n3"},
        {6, "ContractAnnotation", "contract", "n4"}
    };
}

void test_build_heatmap_returns_cells_for_each_line() {
    TEST(build_heatmap_returns_cells_for_each_line);
    auto cells = CodeAnnotationHeatmap::build(sample(), 8, true);
    CHECK(cells.size() == 8, "expected 8 cells");
    PASS();
}

void test_disabled_heatmap_returns_empty() {
    TEST(disabled_heatmap_returns_empty);
    auto cells = CodeAnnotationHeatmap::build(sample(), 8, false);
    CHECK(cells.empty(), "disabled heatmap should be empty");
    PASS();
}

void test_maxline_nonpositive_returns_empty() {
    TEST(maxline_nonpositive_returns_empty);
    auto cells = CodeAnnotationHeatmap::build(sample(), 0, true);
    CHECK(cells.empty(), "maxLine <=0 should return empty");
    PASS();
}

void test_weight_mapping_complexity() {
    TEST(weight_mapping_complexity);
    CHECK(CodeAnnotationHeatmap::weightForType("ComplexityAnnotation") == 3, "complexity weight");
    PASS();
}

void test_weight_mapping_risk() {
    TEST(weight_mapping_risk);
    CHECK(CodeAnnotationHeatmap::weightForType("RiskAnnotation") == 4, "risk weight");
    PASS();
}

void test_weight_mapping_review() {
    TEST(weight_mapping_review);
    CHECK(CodeAnnotationHeatmap::weightForType("ReviewAnnotation") == 2, "review weight");
    PASS();
}

void test_color_for_score_cool_blue() {
    TEST(color_for_score_cool_blue);
    CHECK(CodeAnnotationHeatmap::colorForScore(0) == "#1F6F8B", "score 0 should be blue");
    PASS();
}

void test_color_for_score_cool_green() {
    TEST(color_for_score_cool_green);
    CHECK(CodeAnnotationHeatmap::colorForScore(2) == "#2ECC71", "score 2 should be green");
    PASS();
}

void test_color_for_score_orange() {
    TEST(color_for_score_orange);
    CHECK(CodeAnnotationHeatmap::colorForScore(5) == "#F39C12", "score 5 should be orange");
    PASS();
}

void test_color_for_score_red() {
    TEST(color_for_score_red);
    CHECK(CodeAnnotationHeatmap::colorForScore(6) == "#E74C3C", "score 6 should be red");
    PASS();
}

void test_line_score_accumulates_multiple_annotations() {
    TEST(line_score_accumulates_multiple_annotations);
    auto cells = CodeAnnotationHeatmap::build(sample(), 8, true);
    int line3 = -1;
    for (const auto& c : cells) if (c.line == 3) line3 = c.score;
    CHECK(line3 == 7, "line 3 should accumulate complexity(3)+risk(4)=7");
    PASS();
}

void test_annotations_outside_range_ignored() {
    TEST(annotations_outside_range_ignored);
    auto in = sample();
    in.push_back({100, "RiskAnnotation", "x", "n5"});
    auto cells = CodeAnnotationHeatmap::build(in, 8, true);
    CHECK(cells.size() == 8, "still bounded to maxLine");
    int total = 0;
    for (const auto& c : cells) total += c.score;
    CHECK(total < 20, "out-of-range annotation should not inflate score");
    PASS();
}

int main() {
    std::cout << "Step 434: Annotation Heatmap Tests\n";

    test_build_heatmap_returns_cells_for_each_line(); // 1
    test_disabled_heatmap_returns_empty();            // 2
    test_maxline_nonpositive_returns_empty();         // 3
    test_weight_mapping_complexity();                 // 4
    test_weight_mapping_risk();                       // 5
    test_weight_mapping_review();                     // 6
    test_color_for_score_cool_blue();                // 7
    test_color_for_score_cool_green();               // 8
    test_color_for_score_orange();                   // 9
    test_color_for_score_red();                      // 10
    test_line_score_accumulates_multiple_annotations();// 11
    test_annotations_outside_range_ignored();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
