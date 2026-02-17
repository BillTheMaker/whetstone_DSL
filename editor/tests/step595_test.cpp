// Step 595: Approval Escalation Planner (12 tests)

#include "ApprovalEscalationPlanner.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static PolicyGuardrailCatalog catalogWithRules() {
    PolicyGuardrailCatalog c;
    std::string error;
    c.addRule({"allow-push", "git push", GuardrailDecision::Allow, "trusted push path"}, &error);
    c.addRule({"review-deploy", "deploy", GuardrailDecision::RequireReview, "deployment review required"}, &error);
    c.addRule({"deny-rm", "rm -rf", GuardrailDecision::Deny, "destructive operation denied"}, &error);
    return c;
}

void test_allow_rule_produces_no_escalation() {
    TEST(allow_rule_produces_no_escalation);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "git push origin main");
    CHECK(plan.level == EscalationLevel::None, "level should be none");
    CHECK(!plan.requiresJustification, "justification should be false");
    CHECK(plan.approverRoles.empty(), "approvers should be empty");
    PASS();
}

void test_review_rule_produces_reviewer_escalation() {
    TEST(review_rule_produces_reviewer_escalation);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "deploy prod");
    CHECK(plan.level == EscalationLevel::Reviewer, "level should be reviewer");
    CHECK(plan.requiresJustification, "justification should be true");
    CHECK(plan.approverRoles.size() == 1 && plan.approverRoles[0] == "reviewer", "reviewer role expected");
    PASS();
}

void test_deny_rule_produces_admin_escalation() {
    TEST(deny_rule_produces_admin_escalation);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "rm -rf /tmp/x");
    CHECK(plan.level == EscalationLevel::Admin, "level should be admin");
    CHECK(plan.approverRoles.size() == 2, "two admin approvers expected");
    PASS();
}

void test_default_review_when_no_rule_matches() {
    TEST(default_review_when_no_rule_matches);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "unknown op");
    CHECK(plan.decision == GuardrailDecision::RequireReview, "default decision should be review");
    CHECK(plan.reason == "default_review_required", "default reason mismatch");
    PASS();
}

void test_reason_propagates_from_guardrail_decision() {
    TEST(reason_propagates_from_guardrail_decision);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "deploy prod");
    CHECK(plan.reason == "deployment review required", "reason should propagate");
    PASS();
}

void test_operation_field_is_preserved() {
    TEST(operation_field_is_preserved);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "git push origin main");
    CHECK(plan.operation == "git push origin main", "operation field mismatch");
    PASS();
}

void test_is_executable_true_for_allow() {
    TEST(is_executable_true_for_allow);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "git push origin main");
    CHECK(ApprovalEscalationPlanner::isExecutable(plan), "allow should be executable");
    PASS();
}

void test_is_executable_true_for_review() {
    TEST(is_executable_true_for_review);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "deploy prod");
    CHECK(ApprovalEscalationPlanner::isExecutable(plan), "review should be executable");
    PASS();
}

void test_is_executable_false_for_deny() {
    TEST(is_executable_false_for_deny);
    auto c = catalogWithRules();
    auto plan = ApprovalEscalationPlanner::planFor(c, "rm -rf /tmp/x");
    CHECK(!ApprovalEscalationPlanner::isExecutable(plan), "deny should not be executable");
    PASS();
}

void test_risk_score_increases_by_decision_severity() {
    TEST(risk_score_increases_by_decision_severity);
    auto c = catalogWithRules();
    auto allowPlan = ApprovalEscalationPlanner::planFor(c, "git push origin main");
    auto reviewPlan = ApprovalEscalationPlanner::planFor(c, "deploy prod");
    auto denyPlan = ApprovalEscalationPlanner::planFor(c, "rm -rf /tmp/x");
    CHECK(ApprovalEscalationPlanner::riskScore(allowPlan) < ApprovalEscalationPlanner::riskScore(reviewPlan),
          "review risk should exceed allow risk");
    CHECK(ApprovalEscalationPlanner::riskScore(reviewPlan) < ApprovalEscalationPlanner::riskScore(denyPlan),
          "deny risk should exceed review risk");
    PASS();
}

void test_risk_score_capped_at_100() {
    TEST(risk_score_capped_at_100);
    EscalationPlan p;
    p.decision = GuardrailDecision::Deny;
    p.level = EscalationLevel::Admin;
    p.requiresJustification = true;
    CHECK(ApprovalEscalationPlanner::riskScore(p) <= 100, "risk score should be capped");
    PASS();
}

void test_admin_plan_requires_justification() {
    TEST(admin_plan_requires_justification);
    auto c = catalogWithRules();
    auto p = ApprovalEscalationPlanner::planFor(c, "rm -rf /tmp/x");
    CHECK(p.requiresJustification, "admin plan should require justification");
    PASS();
}

int main() {
    std::cout << "Step 595: Approval Escalation Planner\n";

    test_allow_rule_produces_no_escalation();           // 1
    test_review_rule_produces_reviewer_escalation();    // 2
    test_deny_rule_produces_admin_escalation();         // 3
    test_default_review_when_no_rule_matches();         // 4
    test_reason_propagates_from_guardrail_decision();   // 5
    test_operation_field_is_preserved();                // 6
    test_is_executable_true_for_allow();                // 7
    test_is_executable_true_for_review();               // 8
    test_is_executable_false_for_deny();                // 9
    test_risk_score_increases_by_decision_severity();   // 10
    test_risk_score_capped_at_100();                    // 11
    test_admin_plan_requires_justification();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
