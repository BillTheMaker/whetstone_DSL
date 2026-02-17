// Step 456: Transpilation Confidence Scoring Tests (12 tests)

#include "TranspilationConfidence.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_direct_stdlib_scores_point_95() {
    TEST(direct_stdlib_scores_point_95);
    auto s = TranspilationScorer::score("sort_values", "sort loop", "items.sort()", "python", "rust", true);
    CHECK(s.category == TranslationCategory::DirectStdLib, "expected direct stdlib category");
    CHECK(s.score == 0.95f, "expected 0.95 score");
    CHECK(!s.reviewRequired, "high confidence should not require review");
    PASS();
}

void test_algorithm_pattern_scores_point_85() {
    TEST(algorithm_pattern_scores_point_85);
    auto s = TranspilationScorer::score("sum_values", "sum loop", "items.iter().fold(0, |a, x| a + x)", "python", "rust", true);
    CHECK(s.category == TranslationCategory::AlgorithmPattern, "expected algorithm pattern");
    CHECK(s.score == 0.85f, "expected 0.85 score");
    PASS();
}

void test_structural_with_intent_scores_point_70() {
    TEST(structural_with_intent_scores_point_70);
    auto s = TranspilationScorer::score("transform", "custom transform", "transform(data)", "python", "java", true);
    CHECK(s.category == TranslationCategory::StructuralWithIntent, "expected structural with intent");
    CHECK(s.score == 0.70f, "expected 0.70 score");
    CHECK(!s.reviewRequired, "0.70 should not require review");
    PASS();
}

void test_structural_no_intent_scores_point_50_and_reviews() {
    TEST(structural_no_intent_scores_point_50_and_reviews);
    auto s = TranspilationScorer::score("transform", "custom transform", "transform(data)", "python", "java", false);
    CHECK(s.category == TranslationCategory::StructuralNoIntent, "expected structural no intent");
    CHECK(s.score == 0.50f, "expected 0.50 score");
    CHECK(s.reviewRequired, "0.50 should require review");
    PASS();
}

void test_unknown_todo_scores_point_30_and_reviews() {
    TEST(unknown_todo_scores_point_30_and_reviews);
    auto s = TranspilationScorer::score("mystery", "unknown", "TODO: implement", "python", "rust", false);
    CHECK(s.category == TranslationCategory::Unknown, "expected unknown category");
    CHECK(s.score == 0.30f, "expected 0.30 score");
    CHECK(s.reviewRequired, "0.30 should require review");
    PASS();
}

void test_low_confidence_adds_review_annotation() {
    TEST(low_confidence_adds_review_annotation);
    auto s = TranspilationScorer::score("mystery", "unknown", "TODO", "python", "rust", false);
    CHECK(s.annotation.find("@Review(required,human)") != std::string::npos,
          "expected human review annotation");
    CHECK(!s.reviewReason.empty(), "expected non-empty review reason");
    PASS();
}

void test_high_confidence_annotation_has_no_review_tag() {
    TEST(high_confidence_annotation_has_no_review_tag);
    auto s = TranspilationScorer::score("sort_values", "sort loop", "std::sort(v.begin(), v.end())", "cpp", "cpp", true);
    CHECK(s.annotation.find("@Confidence(") != std::string::npos, "expected confidence annotation");
    CHECK(s.annotation.find("@Review(required,human)") == std::string::npos,
          "high confidence should not include review annotation");
    PASS();
}

void test_score_all_aggregates_counts() {
    TEST(score_all_aggregates_counts);
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> fns = {
        {"sort_values", {"sort loop", "items.sort()"}},
        {"sum_values", {"sum loop", "items.iter().fold(0, |a, x| a + x)"}},
        {"unknown", {"unknown", "TODO"}}
    };
    auto report = TranspilationScorer::scoreAll(fns, "python", "rust", {true, true, false});
    CHECK(report.scores.size() == 3, "expected 3 scores");
    CHECK(report.countByCategory(TranslationCategory::DirectStdLib) == 1, "expected one stdlib score");
    CHECK(report.countReviewRequired() == 1, "expected one review-required score");
    PASS();
}

void test_average_score_computation() {
    TEST(average_score_computation);
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> fns = {
        {"a", {"sort", "items.sort()"}},    // 0.95
        {"b", {"sum", "items.iter().fold(0, |a, x| a + x)"}}, // 0.85
        {"c", {"u", "TODO"}}                // 0.30
    };
    auto report = TranspilationScorer::scoreAll(fns, "python", "rust", {true, true, false});
    float avg = report.averageScore();
    CHECK(avg > 0.69f && avg < 0.71f, "expected average around 0.70");
    PASS();
}

void test_get_score_by_function_name() {
    TEST(get_score_by_function_name);
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> fns = {
        {"sort_values", {"sort", "items.sort()"}},
        {"mystery", {"u", "TODO"}}
    };
    auto report = TranspilationScorer::scoreAll(fns, "python", "rust", {true, false});
    auto s = report.getScore("mystery");
    CHECK(s.functionName == "mystery", "expected returned function score");
    CHECK(s.reviewRequired, "mystery should require review");
    PASS();
}

void test_score_all_defaults_has_intent_false() {
    TEST(score_all_defaults_has_intent_false);
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> fns = {
        {"x", {"custom", "transform(data)"}}
    };
    auto report = TranspilationScorer::scoreAll(fns, "python", "java");
    CHECK(report.scores.size() == 1, "expected one score");
    CHECK(report.scores[0].category == TranslationCategory::StructuralNoIntent,
          "default intent should be false");
    PASS();
}

void test_category_score_lookup_values() {
    TEST(category_score_lookup_values);
    CHECK(TranspilationScorer::categoryScore(TranslationCategory::DirectStdLib) == 0.95f,
          "expected stdlib category score");
    CHECK(TranspilationScorer::categoryScore(TranslationCategory::Unknown) == 0.30f,
          "expected unknown category score");
    PASS();
}

int main() {
    std::cout << "Step 456: Transpilation Confidence Scoring Tests\n";

    test_direct_stdlib_scores_point_95();                // 1
    test_algorithm_pattern_scores_point_85();            // 2
    test_structural_with_intent_scores_point_70();       // 3
    test_structural_no_intent_scores_point_50_and_reviews(); // 4
    test_unknown_todo_scores_point_30_and_reviews();     // 5
    test_low_confidence_adds_review_annotation();        // 6
    test_high_confidence_annotation_has_no_review_tag(); // 7
    test_score_all_aggregates_counts();                  // 8
    test_average_score_computation();                    // 9
    test_get_score_by_function_name();                   // 10
    test_score_all_defaults_has_intent_false();          // 11
    test_category_score_lookup_values();                 // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
