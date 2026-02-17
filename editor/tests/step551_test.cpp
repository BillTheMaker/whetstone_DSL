// Step 551: Review-Gate Policy Refinement (12 tests)

#include "ReviewGatePolicyRefinement.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasReason(const ReviewGateResult& r, const std::string& reason) {
    for (const auto& x : r.reasons) if (x == reason) return true;
    return false;
}

void test_policy_violation_blocks_immediately() {
    TEST(policy_violation_blocks_immediately);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, true, 95, 0, true, false});
    CHECK(r.requiresHumanReview, "block should still require human review state");
    CHECK(r.decision == "block", "policy violation should block");
    CHECK(hasReason(r, "policy_violation_detected"), "reason expected");
    PASS();
}

void test_safety_critical_requires_review() {
    TEST(safety_critical_requires_review);
    auto r = ReviewGatePolicyRefinement::evaluate({true, false, false, 95, 0, true, false});
    CHECK(r.decision == "escalate_review", "safety-critical should escalate");
    CHECK(hasReason(r, "safety_critical_change"), "safety reason expected");
    PASS();
}

void test_security_sensitive_requires_review() {
    TEST(security_sensitive_requires_review);
    auto r = ReviewGatePolicyRefinement::evaluate({false, true, false, 95, 0, true, false});
    CHECK(r.decision == "escalate_review", "security-sensitive should escalate");
    CHECK(hasReason(r, "security_sensitive_change"), "security reason expected");
    PASS();
}

void test_repeated_failures_require_review() {
    TEST(repeated_failures_require_review);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 95, 3, true, false});
    CHECK(r.decision == "escalate_review", "failure streak should escalate");
    CHECK(hasReason(r, "repeated_post_apply_failures"), "failure streak reason expected");
    PASS();
}

void test_high_confidence_deterministic_auto_approves() {
    TEST(high_confidence_deterministic_auto_approves);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 90, 0, true, false});
    CHECK(!r.requiresHumanReview, "should auto approve");
    CHECK(r.decision == "auto_approve", "decision mismatch");
    PASS();
}

void test_moderate_confidence_auto_approve_with_audit() {
    TEST(moderate_confidence_auto_approve_with_audit);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 75, 0, false, false});
    CHECK(!r.requiresHumanReview, "should not require human review");
    CHECK(r.decision == "auto_approve_with_audit", "decision mismatch");
    PASS();
}

void test_low_confidence_escalates_review() {
    TEST(low_confidence_escalates_review);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 60, 0, false, false});
    CHECK(r.requiresHumanReview, "low confidence should require review");
    CHECK(r.decision == "escalate_review", "decision mismatch");
    CHECK(hasReason(r, "low_confidence"), "low confidence reason expected");
    PASS();
}

void test_high_cost_overage_escalates_review() {
    TEST(high_cost_overage_escalates_review);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 90, 0, true, true});
    CHECK(r.requiresHumanReview, "high overage should escalate");
    CHECK(r.decision == "escalate_review", "decision mismatch");
    CHECK(hasReason(r, "high_cost_overage"), "overage reason expected");
    PASS();
}

void test_safety_overrides_high_confidence_auto_approve() {
    TEST(safety_overrides_high_confidence_auto_approve);
    auto r = ReviewGatePolicyRefinement::evaluate({true, false, false, 95, 0, true, false});
    CHECK(r.decision == "escalate_review", "safety should override auto approval");
    PASS();
}

void test_policy_violation_overrides_all_other_conditions() {
    TEST(policy_violation_overrides_all_other_conditions);
    auto r = ReviewGatePolicyRefinement::evaluate({true, true, true, 99, 10, true, true});
    CHECK(r.decision == "block", "policy violation should dominate");
    CHECK(hasReason(r, "policy_violation_detected"), "policy reason expected");
    PASS();
}

void test_failure_streak_threshold_is_inclusive() {
    TEST(failure_streak_threshold_is_inclusive);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 90, 3, true, false});
    CHECK(r.decision == "escalate_review", "streak==3 should escalate");
    PASS();
}

void test_deterministic_path_without_high_confidence_uses_audit_path() {
    TEST(deterministic_path_without_high_confidence_uses_audit_path);
    auto r = ReviewGatePolicyRefinement::evaluate({false, false, false, 80, 0, true, false});
    CHECK(r.decision == "auto_approve_with_audit", "80 confidence should use audit path");
    PASS();
}

int main() {
    std::cout << "Step 551: Review-Gate Policy Refinement\n";

    test_policy_violation_blocks_immediately();                 // 1
    test_safety_critical_requires_review();                     // 2
    test_security_sensitive_requires_review();                  // 3
    test_repeated_failures_require_review();                    // 4
    test_high_confidence_deterministic_auto_approves();        // 5
    test_moderate_confidence_auto_approve_with_audit();        // 6
    test_low_confidence_escalates_review();                     // 7
    test_high_cost_overage_escalates_review();                  // 8
    test_safety_overrides_high_confidence_auto_approve();       // 9
    test_policy_violation_overrides_all_other_conditions();     // 10
    test_failure_streak_threshold_is_inclusive();               // 11
    test_deterministic_path_without_high_confidence_uses_audit_path(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
