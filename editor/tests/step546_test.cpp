// Step 546: Confidence Calibration for Constrained Paths (12 tests)

#include "ConstrainedConfidenceCalibrator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_no_history_defaults_to_base_confidence_band() {
    TEST(no_history_defaults_to_base_confidence_band);
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.70, {}, {}, {}, "");
    CHECK(r.adjustedConfidence > 0.65 && r.adjustedConfidence < 0.75,
          "default confidence should remain near base");
    PASS();
}

void test_high_success_rate_increases_score() {
    TEST(high_success_rate_increases_score);
    std::map<std::string, int> succ{{"constrained_worker", 9}};
    std::map<std::string, int> att{{"constrained_worker", 10}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.60, succ, att, {}, "constrained_worker");
    CHECK(r.routeScores["constrained_worker"] > 0.70, "high success should boost score");
    PASS();
}

void test_low_success_rate_decreases_score() {
    TEST(low_success_rate_decreases_score);
    std::map<std::string, int> succ{{"deterministic_template", 1}};
    std::map<std::string, int> att{{"deterministic_template", 10}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.70, succ, att, {}, "deterministic_template");
    CHECK(r.routeScores["deterministic_template"] < 0.60, "low success should reduce score");
    PASS();
}

void test_post_apply_failures_penalize_route() {
    TEST(post_apply_failures_penalize_route);
    std::map<std::string, int> succ{{"constrained_worker", 8}};
    std::map<std::string, int> att{{"constrained_worker", 10}};
    std::map<std::string, int> fail{{"constrained_worker", 4}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.75, succ, att, fail, "constrained_worker");
    CHECK(r.routeScores["constrained_worker"] < 0.70, "post-apply failures should penalize score");
    PASS();
}

void test_repeated_failures_trigger_extra_decay() {
    TEST(repeated_failures_trigger_extra_decay);
    std::map<std::string, int> att{{"constrained_worker", 10}};
    std::map<std::string, int> fail3{{"constrained_worker", 3}};
    std::map<std::string, int> fail5{{"constrained_worker", 5}};
    auto r3 = ConstrainedConfidenceCalibrator::calibrate(0.8, {}, att, fail3, "constrained_worker");
    auto r5 = ConstrainedConfidenceCalibrator::calibrate(0.8, {}, att, fail5, "constrained_worker");
    CHECK(r5.routeScores["constrained_worker"] < r3.routeScores["constrained_worker"],
          "more repeated failures should decay confidence further");
    PASS();
}

void test_best_route_selected_by_highest_score() {
    TEST(best_route_selected_by_highest_score);
    std::map<std::string, int> succ{{"deterministic_template", 9}, {"constrained_worker", 4}};
    std::map<std::string, int> att{{"deterministic_template", 10}, {"constrained_worker", 10}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.65, succ, att, {}, "");
    CHECK(r.recommendedRoute == "deterministic_template", "highest score route should be selected");
    PASS();
}

void test_preferred_route_bias_applies() {
    TEST(preferred_route_bias_applies);
    auto a = ConstrainedConfidenceCalibrator::calibrate(0.70, {}, {}, {}, "");
    auto b = ConstrainedConfidenceCalibrator::calibrate(0.70, {}, {}, {}, "constrained_worker");
    CHECK(b.routeScores["constrained_worker"] > a.routeScores["constrained_worker"],
          "preferred route should receive bias");
    PASS();
}

void test_scores_are_clamped_to_one() {
    TEST(scores_are_clamped_to_one);
    std::map<std::string, int> succ{{"deterministic_template", 100}};
    std::map<std::string, int> att{{"deterministic_template", 100}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.99, succ, att, {}, "deterministic_template");
    CHECK(r.routeScores["deterministic_template"] <= 1.0, "score should clamp to one");
    PASS();
}

void test_scores_are_clamped_to_zero() {
    TEST(scores_are_clamped_to_zero);
    std::map<std::string, int> att{{"constrained_worker", 10}};
    std::map<std::string, int> fail{{"constrained_worker", 10}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.05, {}, att, fail, "constrained_worker");
    CHECK(r.routeScores["constrained_worker"] >= 0.0, "score should clamp at zero");
    PASS();
}

void test_adjusted_confidence_matches_recommended_route_score() {
    TEST(adjusted_confidence_matches_recommended_route_score);
    std::map<std::string, int> succ{{"general_worker", 8}};
    std::map<std::string, int> att{{"general_worker", 10}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.60, succ, att, {}, "general_worker");
    CHECK(r.adjustedConfidence == r.routeScores[r.recommendedRoute],
          "adjusted confidence should match recommended route score");
    PASS();
}

void test_missing_attempts_uses_neutral_prior() {
    TEST(missing_attempts_uses_neutral_prior);
    std::map<std::string, int> succ{{"constrained_worker", 5}};
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.70, succ, {}, {}, "constrained_worker");
    CHECK(r.routeScores["constrained_worker"] > 0.65 && r.routeScores["constrained_worker"] < 0.80,
          "missing attempts should stay near base with mild bias only");
    PASS();
}

void test_route_scores_include_all_policy_routes() {
    TEST(route_scores_include_all_policy_routes);
    auto r = ConstrainedConfidenceCalibrator::calibrate(0.70, {}, {}, {}, "");
    CHECK(r.routeScores.count("deterministic_template") == 1, "missing deterministic route score");
    CHECK(r.routeScores.count("constrained_worker") == 1, "missing constrained route score");
    CHECK(r.routeScores.count("general_worker") == 1, "missing general route score");
    PASS();
}

int main() {
    std::cout << "Step 546: Confidence Calibration for Constrained Paths\n";

    test_no_history_defaults_to_base_confidence_band();      // 1
    test_high_success_rate_increases_score();                // 2
    test_low_success_rate_decreases_score();                 // 3
    test_post_apply_failures_penalize_route();               // 4
    test_repeated_failures_trigger_extra_decay();            // 5
    test_best_route_selected_by_highest_score();             // 6
    test_preferred_route_bias_applies();                     // 7
    test_scores_are_clamped_to_one();                        // 8
    test_scores_are_clamped_to_zero();                       // 9
    test_adjusted_confidence_matches_recommended_route_score(); // 10
    test_missing_attempts_uses_neutral_prior();              // 11
    test_route_scores_include_all_policy_routes();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
