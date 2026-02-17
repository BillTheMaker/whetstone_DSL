// Step 605: Deployment Promotion Gate (12 tests)

#include "DeploymentPromotionGate.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static EnvironmentGateState state(const std::string& env,
                                  int required,
                                  int passedChecks,
                                  int approvals,
                                  int blockers) {
    return {env, required, passedChecks, approvals, blockers};
}

void test_upsert_success() {
    TEST(upsert_success);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("staging", 4, 3, 1, 0), &error), "upsert should succeed");
    CHECK(gate.blockedEnvironments().size() == 1, "blocked env count mismatch");
    PASS();
}

void test_upsert_rejects_missing_environment_id() {
    TEST(upsert_rejects_missing_environment_id);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(!gate.upsert(state("", 4, 3, 1, 0), &error), "upsert should fail");
    CHECK(error == "environment_id_missing", "wrong error");
    PASS();
}

void test_upsert_rejects_negative_required_checks() {
    TEST(upsert_rejects_negative_required_checks);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(!gate.upsert(state("staging", -1, 0, 0, 0), &error), "upsert should fail");
    CHECK(error == "required_checks_invalid", "wrong error");
    PASS();
}

void test_upsert_rejects_passed_checks_exceed_required() {
    TEST(upsert_rejects_passed_checks_exceed_required);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(!gate.upsert(state("staging", 2, 3, 1, 0), &error), "upsert should fail");
    CHECK(error == "passed_checks_exceed_required", "wrong error");
    PASS();
}

void test_promotable_true_when_all_conditions_met() {
    TEST(promotable_true_when_all_conditions_met);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("prod", 5, 5, 2, 0), &error), "upsert failed");
    CHECK(gate.promotable("prod", &error), "prod should be promotable");
    PASS();
}

void test_promotable_false_when_checks_incomplete() {
    TEST(promotable_false_when_checks_incomplete);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("prod", 5, 4, 2, 0), &error), "upsert failed");
    CHECK(!gate.promotable("prod", &error), "prod should not be promotable");
    PASS();
}

void test_promotable_false_when_no_manual_approval() {
    TEST(promotable_false_when_no_manual_approval);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("prod", 5, 5, 0, 0), &error), "upsert failed");
    CHECK(!gate.promotable("prod", &error), "prod should not be promotable");
    PASS();
}

void test_promotable_rejects_missing_environment() {
    TEST(promotable_rejects_missing_environment);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(!gate.promotable("missing", &error), "missing env should fail");
    CHECK(error == "environment_missing", "wrong error");
    PASS();
}

void test_promotion_score_base_calculation() {
    TEST(promotion_score_base_calculation);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("prod", 10, 8, 1, 0), &error), "upsert failed");
    CHECK(gate.promotionScore("prod", &error) == 85, "score mismatch");
    PASS();
}

void test_promotion_score_penalizes_blockers() {
    TEST(promotion_score_penalizes_blockers);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("prod", 10, 10, 2, 2), &error), "upsert failed");
    CHECK(gate.promotionScore("prod", &error) == 70, "score mismatch");
    PASS();
}

void test_promotion_score_clamps_to_hundred() {
    TEST(promotion_score_clamps_to_hundred);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("prod", 1, 1, 8, 0), &error), "upsert failed");
    CHECK(gate.promotionScore("prod", &error) == 100, "score should clamp to 100");
    PASS();
}

void test_blocked_environments_lists_only_blocked() {
    TEST(blocked_environments_lists_only_blocked);
    DeploymentPromotionGate gate;
    std::string error;
    CHECK(gate.upsert(state("staging", 3, 3, 1, 0), &error), "upsert staging failed");
    CHECK(gate.upsert(state("prod", 3, 2, 1, 0), &error), "upsert prod failed");
    CHECK(gate.upsert(state("canary", 2, 2, 1, 1), &error), "upsert canary failed");
    CHECK(gate.blockedEnvironments().size() == 2, "blocked list size mismatch");
    PASS();
}

int main() {
    std::cout << "Step 605: Deployment Promotion Gate\n";

    test_upsert_success();                              // 1
    test_upsert_rejects_missing_environment_id();       // 2
    test_upsert_rejects_negative_required_checks();     // 3
    test_upsert_rejects_passed_checks_exceed_required();// 4
    test_promotable_true_when_all_conditions_met();     // 5
    test_promotable_false_when_checks_incomplete();     // 6
    test_promotable_false_when_no_manual_approval();    // 7
    test_promotable_rejects_missing_environment();      // 8
    test_promotion_score_base_calculation();            // 9
    test_promotion_score_penalizes_blockers();          // 10
    test_promotion_score_clamps_to_hundred();           // 11
    test_blocked_environments_lists_only_blocked();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
