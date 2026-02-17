// Step 547: Cost Policy Guard (12 tests)

#include "CostPolicyGuard.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasViolation(const CostPolicyResult& r, const std::string& v) {
    for (const auto& x : r.violations) if (x == v) return true;
    return false;
}

void test_within_limits_allows_execution() {
    TEST(within_limits_allows_execution);
    auto r = CostPolicyGuard::evaluate({100, 500, 200, 1000, false, false});
    CHECK(r.allowed, "within limits should allow");
    CHECK(r.action == "allow", "action should be allow");
    PASS();
}

void test_step_ceiling_exceeded_without_rationale_requests_rationale() {
    TEST(step_ceiling_exceeded_without_rationale_requests_rationale);
    auto r = CostPolicyGuard::evaluate({250, 500, 200, 1000, false, false});
    CHECK(!r.allowed, "should not allow without rationale");
    CHECK(r.action == "request_rationale", "action mismatch");
    CHECK(hasViolation(r, "step_token_ceiling_exceeded"), "step violation missing");
    PASS();
}

void test_workflow_ceiling_exceeded_without_rationale_requests_rationale() {
    TEST(workflow_ceiling_exceeded_without_rationale_requests_rationale);
    auto r = CostPolicyGuard::evaluate({150, 1200, 200, 1000, false, false});
    CHECK(!r.allowed, "should not allow without rationale");
    CHECK(r.action == "request_rationale", "action mismatch");
    CHECK(hasViolation(r, "workflow_token_ceiling_exceeded"), "workflow violation missing");
    PASS();
}

void test_both_ceilings_exceeded_collects_both_violations() {
    TEST(both_ceilings_exceeded_collects_both_violations);
    auto r = CostPolicyGuard::evaluate({250, 1200, 200, 1000, false, false});
    CHECK(hasViolation(r, "step_token_ceiling_exceeded"), "step violation missing");
    CHECK(hasViolation(r, "workflow_token_ceiling_exceeded"), "workflow violation missing");
    PASS();
}

void test_with_rationale_but_no_approval_requires_escalation() {
    TEST(with_rationale_but_no_approval_requires_escalation);
    auto r = CostPolicyGuard::evaluate({250, 500, 200, 1000, true, false});
    CHECK(!r.allowed, "should not allow without approval");
    CHECK(r.requiresEscalation, "escalation should be required");
    CHECK(r.action == "escalate", "action should be escalate");
    PASS();
}

void test_with_rationale_and_approval_allows_with_escalation_flag() {
    TEST(with_rationale_and_approval_allows_with_escalation_flag);
    auto r = CostPolicyGuard::evaluate({250, 500, 200, 1000, true, true});
    CHECK(r.allowed, "approved escalation should allow");
    CHECK(r.requiresEscalation, "escalation flag should remain true");
    PASS();
}

void test_workflow_overage_with_rationale_and_approval_allows() {
    TEST(workflow_overage_with_rationale_and_approval_allows);
    auto r = CostPolicyGuard::evaluate({150, 1500, 200, 1000, true, true});
    CHECK(r.allowed, "approved workflow overage should allow");
    CHECK(hasViolation(r, "workflow_token_ceiling_exceeded"), "workflow violation should be tracked");
    PASS();
}

void test_equal_to_step_ceiling_is_allowed() {
    TEST(equal_to_step_ceiling_is_allowed);
    auto r = CostPolicyGuard::evaluate({200, 500, 200, 1000, false, false});
    CHECK(r.allowed, "equal to step ceiling should be allowed");
    PASS();
}

void test_equal_to_workflow_ceiling_is_allowed() {
    TEST(equal_to_workflow_ceiling_is_allowed);
    auto r = CostPolicyGuard::evaluate({150, 1000, 200, 1000, false, false});
    CHECK(r.allowed, "equal to workflow ceiling should be allowed");
    PASS();
}

void test_zero_ceiling_disables_check() {
    TEST(zero_ceiling_disables_check);
    auto r = CostPolicyGuard::evaluate({10000, 10000, 0, 0, false, false});
    CHECK(r.allowed, "zero ceilings should disable checks");
    PASS();
}

void test_negative_tokens_do_not_trigger_violations() {
    TEST(negative_tokens_do_not_trigger_violations);
    auto r = CostPolicyGuard::evaluate({-10, -50, 200, 1000, false, false});
    CHECK(r.allowed, "negative token values should not violate ceilings");
    PASS();
}

void test_escalation_not_required_when_within_limits_even_with_rationale() {
    TEST(escalation_not_required_when_within_limits_even_with_rationale);
    auto r = CostPolicyGuard::evaluate({100, 500, 200, 1000, true, true});
    CHECK(r.allowed, "within limits should allow");
    CHECK(!r.requiresEscalation, "escalation should not be required");
    CHECK(r.violations.empty(), "no violations expected");
    PASS();
}

int main() {
    std::cout << "Step 547: Cost Policy Guard\n";

    test_within_limits_allows_execution();                              // 1
    test_step_ceiling_exceeded_without_rationale_requests_rationale();  // 2
    test_workflow_ceiling_exceeded_without_rationale_requests_rationale();// 3
    test_both_ceilings_exceeded_collects_both_violations();             // 4
    test_with_rationale_but_no_approval_requires_escalation();          // 5
    test_with_rationale_and_approval_allows_with_escalation_flag();     // 6
    test_workflow_overage_with_rationale_and_approval_allows();         // 7
    test_equal_to_step_ceiling_is_allowed();                            // 8
    test_equal_to_workflow_ceiling_is_allowed();                        // 9
    test_zero_ceiling_disables_check();                                 // 10
    test_negative_tokens_do_not_trigger_violations();                   // 11
    test_escalation_not_required_when_within_limits_even_with_rationale();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
