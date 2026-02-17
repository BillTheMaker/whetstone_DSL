// Step 613: Sprint 35 Operational Readiness (12 tests)

#include "Sprint35OperationalReadiness.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static Sprint35ReadinessInput healthy() {
    return {0, 0, 0, 0, 0};
}

void test_validate_success() {
    TEST(validate_success);
    std::string error;
    CHECK(Sprint35OperationalReadiness::validate(healthy(), &error), "validate should pass");
    CHECK(error.empty(), "error should be empty");
    PASS();
}

void test_validate_rejects_negative_freeze_violations() {
    TEST(validate_rejects_negative_freeze_violations);
    auto in = healthy();
    in.freezeViolations = -1;
    std::string error;
    CHECK(!Sprint35OperationalReadiness::validate(in, &error), "validate should fail");
    CHECK(error == "freeze_violations_invalid", "wrong error");
    PASS();
}

void test_validate_rejects_negative_high_risk_dependencies() {
    TEST(validate_rejects_negative_high_risk_dependencies);
    auto in = healthy();
    in.highRiskDependencies = -1;
    std::string error;
    CHECK(!Sprint35OperationalReadiness::validate(in, &error), "validate should fail");
    CHECK(error == "high_risk_dependencies_invalid", "wrong error");
    PASS();
}

void test_validate_rejects_negative_oncall_gaps() {
    TEST(validate_rejects_negative_oncall_gaps);
    auto in = healthy();
    in.onCallCoverageGaps = -1;
    std::string error;
    CHECK(!Sprint35OperationalReadiness::validate(in, &error), "validate should fail");
    CHECK(error == "oncall_gaps_invalid", "wrong error");
    PASS();
}

void test_validate_rejects_negative_blocked_canaries() {
    TEST(validate_rejects_negative_blocked_canaries);
    auto in = healthy();
    in.blockedCanaries = -1;
    std::string error;
    CHECK(!Sprint35OperationalReadiness::validate(in, &error), "validate should fail");
    CHECK(error == "blocked_canaries_invalid", "wrong error");
    PASS();
}

void test_validate_rejects_negative_incomplete_postmortems() {
    TEST(validate_rejects_negative_incomplete_postmortems);
    auto in = healthy();
    in.incompletePostmortems = -1;
    std::string error;
    CHECK(!Sprint35OperationalReadiness::validate(in, &error), "validate should fail");
    CHECK(error == "incomplete_postmortems_invalid", "wrong error");
    PASS();
}

void test_readiness_score_full_when_healthy() {
    TEST(readiness_score_full_when_healthy);
    CHECK(Sprint35OperationalReadiness::readinessScore(healthy()) == 100, "healthy score should be 100");
    PASS();
}

void test_readiness_score_decreases_with_risks() {
    TEST(readiness_score_decreases_with_risks);
    auto in = Sprint35ReadinessInput{1, 1, 4, 1, 1};
    CHECK(Sprint35OperationalReadiness::readinessScore(in) < 100, "score should be reduced");
    PASS();
}

void test_readiness_score_clamps_to_zero() {
    TEST(readiness_score_clamps_to_zero);
    auto in = Sprint35ReadinessInput{10, 10, 40, 10, 10};
    CHECK(Sprint35OperationalReadiness::readinessScore(in) == 0, "score should clamp to zero");
    PASS();
}

void test_blockers_include_expected_flags() {
    TEST(blockers_include_expected_flags);
    auto in = Sprint35ReadinessInput{1, 3, 1, 1, 1};
    CHECK(Sprint35OperationalReadiness::blockers(in).size() == 5, "all blockers should be present");
    PASS();
}

void test_release_allowed_true_when_clean() {
    TEST(release_allowed_true_when_clean);
    CHECK(Sprint35OperationalReadiness::releaseAllowed(healthy()), "healthy should allow release");
    PASS();
}

void test_release_allowed_false_with_blockers() {
    TEST(release_allowed_false_with_blockers);
    auto in = healthy();
    in.blockedCanaries = 1;
    CHECK(!Sprint35OperationalReadiness::releaseAllowed(in), "blocked canaries should block release");
    PASS();
}

int main() {
    std::cout << "Step 613: Sprint 35 Operational Readiness\n";

    test_validate_success();                                 // 1
    test_validate_rejects_negative_freeze_violations();     // 2
    test_validate_rejects_negative_high_risk_dependencies();// 3
    test_validate_rejects_negative_oncall_gaps();           // 4
    test_validate_rejects_negative_blocked_canaries();      // 5
    test_validate_rejects_negative_incomplete_postmortems();// 6
    test_readiness_score_full_when_healthy();               // 7
    test_readiness_score_decreases_with_risks();            // 8
    test_readiness_score_clamps_to_zero();                  // 9
    test_blockers_include_expected_flags();                 // 10
    test_release_allowed_true_when_clean();                 // 11
    test_release_allowed_false_with_blockers();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
