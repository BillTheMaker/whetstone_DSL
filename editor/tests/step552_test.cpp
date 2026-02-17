// Step 552: Cost vs Quality Regression Suite (12 tests)

#include "CostQualityRegressionSuite.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasFailure(const CostQualityRegressionResult& r, const std::string& f) {
    for (const auto& x : r.failures) if (x == f) return true;
    return false;
}

static CostQualityPolicy policy() {
    CostQualityPolicy p;
    p.minQualityScore = 0.80;
    p.minPassRate = 0.85;
    p.maxSafetyIncidents = 0;
    p.minTokenReduction = 20;
    return p;
}

void test_pass_when_reduction_and_quality_targets_met() {
    TEST(pass_when_reduction_and_quality_targets_met);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.92, 0},
        {"optimized", 160, 0.86, 0.90, 0},
        policy());
    CHECK(r.pass, "expected pass");
    PASS();
}

void test_fails_when_token_reduction_insufficient() {
    TEST(fails_when_token_reduction_insufficient);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.92, 0},
        {"optimized", 190, 0.86, 0.90, 0},
        policy());
    CHECK(hasFailure(r, "insufficient_token_reduction"), "token reduction failure expected");
    PASS();
}

void test_fails_when_quality_below_floor() {
    TEST(fails_when_quality_below_floor);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.92, 0},
        {"optimized", 160, 0.75, 0.90, 0},
        policy());
    CHECK(hasFailure(r, "quality_below_floor"), "quality floor failure expected");
    PASS();
}

void test_fails_when_pass_rate_below_floor() {
    TEST(fails_when_pass_rate_below_floor);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.92, 0},
        {"optimized", 160, 0.86, 0.80, 0},
        policy());
    CHECK(hasFailure(r, "pass_rate_below_floor"), "pass rate floor failure expected");
    PASS();
}

void test_fails_when_safety_incidents_exceed_limit() {
    TEST(fails_when_safety_incidents_exceed_limit);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.92, 0},
        {"optimized", 160, 0.86, 0.90, 1},
        policy());
    CHECK(hasFailure(r, "safety_incidents_exceeded"), "safety failure expected");
    PASS();
}

void test_relative_quality_regression_detected() {
    TEST(relative_quality_regression_detected);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.95, 0.92, 0},
        {"optimized", 160, 0.84, 0.90, 0},
        policy());
    CHECK(hasFailure(r, "relative_quality_regression"), "relative quality regression expected");
    PASS();
}

void test_relative_pass_rate_regression_detected() {
    TEST(relative_pass_rate_regression_detected);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.99, 0},
        {"optimized", 160, 0.86, 0.90, 0},
        policy());
    CHECK(hasFailure(r, "relative_pass_rate_regression"), "relative pass rate regression expected");
    PASS();
}

void test_multiple_failures_can_be_reported_together() {
    TEST(multiple_failures_can_be_reported_together);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.95, 0.98, 0},
        {"optimized", 195, 0.70, 0.70, 2},
        policy());
    CHECK(r.failures.size() >= 4, "multiple failures expected");
    PASS();
}

void test_exact_threshold_values_pass() {
    TEST(exact_threshold_values_pass);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.88, 0.90, 0},
        {"optimized", 180, 0.80, 0.85, 0},
        policy());
    CHECK(r.pass, "exact floor and reduction thresholds should pass");
    PASS();
}

void test_zero_safety_incident_limit_enforced() {
    TEST(zero_safety_incident_limit_enforced);
    auto p = policy();
    p.maxSafetyIncidents = 0;
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 200, 0.90, 0.92, 0},
        {"optimized", 160, 0.86, 0.90, 0},
        p);
    CHECK(r.pass, "zero incidents should satisfy limit");
    PASS();
}

void test_token_reduction_is_reported_in_result() {
    TEST(token_reduction_is_reported_in_result);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 240, 0.90, 0.92, 0},
        {"optimized", 180, 0.86, 0.90, 0},
        policy());
    CHECK(r.tokenReduction == 60, "token reduction should be reported");
    PASS();
}

void test_negative_token_reduction_fails_policy() {
    TEST(negative_token_reduction_fails_policy);
    auto r = CostQualityRegressionSuite::evaluate(
        {"baseline", 180, 0.90, 0.92, 0},
        {"optimized", 220, 0.86, 0.90, 0},
        policy());
    CHECK(hasFailure(r, "insufficient_token_reduction"), "negative reduction should fail policy");
    PASS();
}

int main() {
    std::cout << "Step 552: Cost vs Quality Regression Suite\n";

    test_pass_when_reduction_and_quality_targets_met();     // 1
    test_fails_when_token_reduction_insufficient();         // 2
    test_fails_when_quality_below_floor();                  // 3
    test_fails_when_pass_rate_below_floor();                // 4
    test_fails_when_safety_incidents_exceed_limit();        // 5
    test_relative_quality_regression_detected();            // 6
    test_relative_pass_rate_regression_detected();          // 7
    test_multiple_failures_can_be_reported_together();      // 8
    test_exact_threshold_values_pass();                     // 9
    test_zero_safety_incident_limit_enforced();             // 10
    test_token_reduction_is_reported_in_result();           // 11
    test_negative_token_reduction_fails_policy();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
