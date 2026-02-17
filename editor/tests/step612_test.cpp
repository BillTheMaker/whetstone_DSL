// Step 612: Canary Promotion Judge (12 tests)

#include "CanaryPromotionJudge.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static CanaryAssessment assess(const std::string& id,
                               int success,
                               int p95,
                               int burn,
                               bool overrideFlag = false) {
    return {id, success, p95, burn, overrideFlag};
}

void test_record_success() {
    TEST(record_success);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 99, 250, 80), &error), "record should succeed");
    CHECK(judge.promotableCount() == 1, "promotable count mismatch");
    PASS();
}

void test_record_rejects_missing_canary_id() {
    TEST(record_rejects_missing_canary_id);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(!judge.record(assess("", 99, 250, 80), &error), "record should fail");
    CHECK(error == "canary_id_missing", "wrong error");
    PASS();
}

void test_record_rejects_invalid_success_percent() {
    TEST(record_rejects_invalid_success_percent);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(!judge.record(assess("c1", 101, 250, 80), &error), "record should fail");
    CHECK(error == "success_percent_invalid", "wrong error");
    PASS();
}

void test_record_rejects_negative_latency() {
    TEST(record_rejects_negative_latency);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(!judge.record(assess("c1", 99, -1, 80), &error), "record should fail");
    CHECK(error == "latency_invalid", "wrong error");
    PASS();
}

void test_record_rejects_negative_error_budget_burn() {
    TEST(record_rejects_negative_error_budget_burn);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(!judge.record(assess("c1", 99, 200, -1), &error), "record should fail");
    CHECK(error == "error_budget_burn_invalid", "wrong error");
    PASS();
}

void test_promotable_true_when_thresholds_met() {
    TEST(promotable_true_when_thresholds_met);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 99, 300, 100), &error), "record failed");
    CHECK(judge.promotable("c1", &error), "canary should be promotable");
    PASS();
}

void test_promotable_false_when_below_thresholds() {
    TEST(promotable_false_when_below_thresholds);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 97, 450, 120), &error), "record failed");
    CHECK(!judge.promotable("c1", &error), "canary should be blocked");
    PASS();
}

void test_promotable_true_with_manual_override() {
    TEST(promotable_true_with_manual_override);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 90, 500, 200, true), &error), "record failed");
    CHECK(judge.promotable("c1", &error), "override should allow promotion");
    PASS();
}

void test_promotable_rejects_missing_canary() {
    TEST(promotable_rejects_missing_canary);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(!judge.promotable("missing", &error), "query should fail");
    CHECK(error == "canary_missing", "wrong error");
    PASS();
}

void test_risk_score_zero_when_healthy() {
    TEST(risk_score_zero_when_healthy);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 99, 250, 80), &error), "record failed");
    CHECK(judge.riskScore("c1", &error) == 0, "healthy score should be zero");
    PASS();
}

void test_risk_score_increases_for_degradation() {
    TEST(risk_score_increases_for_degradation);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 98, 320, 130), &error), "record failed");
    CHECK(judge.riskScore("c1", &error) > 0, "degraded score should be non-zero");
    PASS();
}

void test_promotable_count_counts_only_allowed_canaries() {
    TEST(promotable_count_counts_only_allowed_canaries);
    CanaryPromotionJudge judge;
    std::string error;
    CHECK(judge.record(assess("c1", 99, 250, 80), &error), "record c1 failed");
    CHECK(judge.record(assess("c2", 95, 500, 180), &error), "record c2 failed");
    CHECK(judge.record(assess("c3", 90, 500, 180, true), &error), "record c3 failed");
    CHECK(judge.promotableCount() == 2, "promotable count mismatch");
    PASS();
}

int main() {
    std::cout << "Step 612: Canary Promotion Judge\n";

    test_record_success();                              // 1
    test_record_rejects_missing_canary_id();           // 2
    test_record_rejects_invalid_success_percent();     // 3
    test_record_rejects_negative_latency();            // 4
    test_record_rejects_negative_error_budget_burn();  // 5
    test_promotable_true_when_thresholds_met();        // 6
    test_promotable_false_when_below_thresholds();     // 7
    test_promotable_true_with_manual_override();       // 8
    test_promotable_rejects_missing_canary();          // 9
    test_risk_score_zero_when_healthy();               // 10
    test_risk_score_increases_for_degradation();       // 11
    test_promotable_count_counts_only_allowed_canaries();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
