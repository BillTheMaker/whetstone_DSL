// Step 594: Policy Guardrail Catalog (12 tests)

#include "PolicyGuardrailCatalog.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static GuardrailRule rule(const std::string& id,
                          const std::string& prefix,
                          GuardrailDecision d,
                          const std::string& reason) {
    return {id, prefix, d, reason};
}

void test_add_rule_success() {
    TEST(add_rule_success);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "git push", GuardrailDecision::Allow, "safe push"), &error), "add should succeed");
    CHECK(c.rules().size() == 1, "rule count mismatch");
    PASS();
}

void test_add_rule_rejects_missing_id() {
    TEST(add_rule_rejects_missing_id);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(!c.addRule(rule("", "git push", GuardrailDecision::Allow, "safe"), &error), "add should fail");
    CHECK(error == "rule_id_missing", "wrong error");
    PASS();
}

void test_add_rule_rejects_missing_prefix() {
    TEST(add_rule_rejects_missing_prefix);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(!c.addRule(rule("r1", "", GuardrailDecision::Allow, "safe"), &error), "add should fail");
    CHECK(error == "operation_prefix_missing", "wrong error");
    PASS();
}

void test_add_rule_rejects_missing_reason() {
    TEST(add_rule_rejects_missing_reason);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(!c.addRule(rule("r1", "git push", GuardrailDecision::Allow, ""), &error), "add should fail");
    CHECK(error == "rule_reason_missing", "wrong error");
    PASS();
}

void test_add_rule_rejects_duplicate_id() {
    TEST(add_rule_rejects_duplicate_id);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "git push", GuardrailDecision::Allow, "safe"), &error), "first add failed");
    CHECK(!c.addRule(rule("r1", "git pull", GuardrailDecision::Allow, "safe"), &error), "duplicate should fail");
    CHECK(error == "rule_duplicate", "wrong error");
    PASS();
}

void test_evaluate_matches_allow_rule() {
    TEST(evaluate_matches_allow_rule);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "git push", GuardrailDecision::Allow, "safe push"), &error), "add failed");
    auto d = c.evaluate("git push origin main");
    CHECK(d.decision == GuardrailDecision::Allow, "decision mismatch");
    CHECK(d.matchedRuleId == "r1", "rule id mismatch");
    PASS();
}

void test_evaluate_matches_require_review_rule() {
    TEST(evaluate_matches_require_review_rule);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "npm publish", GuardrailDecision::RequireReview, "release review"), &error), "add failed");
    auto d = c.evaluate("npm publish");
    CHECK(d.decision == GuardrailDecision::RequireReview, "decision mismatch");
    CHECK(d.reason == "release review", "reason mismatch");
    PASS();
}

void test_evaluate_matches_deny_rule() {
    TEST(evaluate_matches_deny_rule);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "rm -rf", GuardrailDecision::Deny, "destructive"), &error), "add failed");
    auto d = c.evaluate("rm -rf /tmp/x");
    CHECK(d.decision == GuardrailDecision::Deny, "decision mismatch");
    PASS();
}

void test_evaluate_defaults_to_review_when_no_match() {
    TEST(evaluate_defaults_to_review_when_no_match);
    PolicyGuardrailCatalog c;
    auto d = c.evaluate("unknown command");
    CHECK(d.decision == GuardrailDecision::RequireReview, "default decision mismatch");
    CHECK(d.reason == "default_review_required", "default reason mismatch");
    PASS();
}

void test_evaluate_uses_first_matching_rule_order() {
    TEST(evaluate_uses_first_matching_rule_order);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "git", GuardrailDecision::RequireReview, "generic"), &error), "add r1 failed");
    CHECK(c.addRule(rule("r2", "git push", GuardrailDecision::Allow, "specific"), &error), "add r2 failed");
    auto d = c.evaluate("git push origin");
    CHECK(d.matchedRuleId == "r1", "first-match order should apply");
    PASS();
}

void test_rules_return_in_insertion_order() {
    TEST(rules_return_in_insertion_order);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "a", GuardrailDecision::Allow, "a"), &error), "add r1 failed");
    CHECK(c.addRule(rule("r2", "b", GuardrailDecision::Allow, "b"), &error), "add r2 failed");
    auto rules = c.rules();
    CHECK(rules[0].ruleId == "r1", "first order mismatch");
    CHECK(rules[1].ruleId == "r2", "second order mismatch");
    PASS();
}

void test_reason_propagates_from_matching_rule() {
    TEST(reason_propagates_from_matching_rule);
    PolicyGuardrailCatalog c;
    std::string error;
    CHECK(c.addRule(rule("r1", "deploy", GuardrailDecision::RequireReview, "production safety check"), &error), "add failed");
    auto d = c.evaluate("deploy prod");
    CHECK(d.reason == "production safety check", "reason should propagate");
    PASS();
}

int main() {
    std::cout << "Step 594: Policy Guardrail Catalog\n";

    test_add_rule_success();                         // 1
    test_add_rule_rejects_missing_id();              // 2
    test_add_rule_rejects_missing_prefix();          // 3
    test_add_rule_rejects_missing_reason();          // 4
    test_add_rule_rejects_duplicate_id();            // 5
    test_evaluate_matches_allow_rule();              // 6
    test_evaluate_matches_require_review_rule();     // 7
    test_evaluate_matches_deny_rule();               // 8
    test_evaluate_defaults_to_review_when_no_match();// 9
    test_evaluate_uses_first_matching_rule_order();  // 10
    test_rules_return_in_insertion_order();          // 11
    test_reason_propagates_from_matching_rule();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
