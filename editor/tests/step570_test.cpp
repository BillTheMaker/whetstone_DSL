// Step 570: Watch Expression Evaluator Hardening (12 tests)

#include "WatchExpressionEvaluatorHardening.h"

#include <iostream>
#include <map>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_evaluate_literal_expression() {
    TEST(evaluate_literal_expression);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("2 + 3 * 4", {});
    CHECK(result.ok, "evaluation should succeed");
    CHECK(result.value == 14, "value should honor precedence");
    PASS();
}

void test_evaluate_scope_identifier_expression() {
    TEST(evaluate_scope_identifier_expression);
    std::map<std::string, std::int64_t> scope{{"x", 10}, {"y", 2}};
    const auto result = WatchExpressionEvaluatorHardening::evaluate("x - y * 3", scope);
    CHECK(result.ok, "evaluation should succeed");
    CHECK(result.value == 4, "value mismatch");
    PASS();
}

void test_evaluate_rejects_empty_expression() {
    TEST(evaluate_rejects_empty_expression);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("", {});
    CHECK(!result.ok, "empty expression should fail");
    CHECK(result.error == "expression_empty", "wrong error");
    PASS();
}

void test_evaluate_rejects_unsafe_pattern() {
    TEST(evaluate_rejects_unsafe_pattern);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("system + 1", {});
    CHECK(!result.ok, "unsafe expression should fail");
    CHECK(result.rejectedUnsafe, "unsafe flag should be set");
    CHECK(result.error == "expression_unsafe", "wrong error");
    PASS();
}

void test_evaluate_rejects_unknown_identifier() {
    TEST(evaluate_rejects_unknown_identifier);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("missing + 1", {});
    CHECK(!result.ok, "unknown identifier should fail");
    CHECK(result.error == "identifier_unknown", "wrong error");
    PASS();
}

void test_evaluate_rejects_invalid_character() {
    TEST(evaluate_rejects_invalid_character);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("2 + (3)", {});
    CHECK(!result.ok, "invalid character should fail");
    CHECK(result.error == "token_invalid_character", "wrong error");
    PASS();
}

void test_evaluate_rejects_token_limit_exceeded() {
    TEST(evaluate_rejects_token_limit_exceeded);
    WatchEvalLimits limits;
    limits.maxTokens = 3;
    const auto result = WatchExpressionEvaluatorHardening::evaluate("1 + 2 + 3", {}, limits);
    CHECK(!result.ok, "token limit should fail");
    CHECK(result.error == "token_limit_exceeded", "wrong error");
    PASS();
}

void test_evaluate_rejects_operation_limit_exceeded() {
    TEST(evaluate_rejects_operation_limit_exceeded);
    WatchEvalLimits limits;
    limits.maxOperations = 1;
    limits.timeoutBudgetMs = 1000;
    const auto result = WatchExpressionEvaluatorHardening::evaluate("1 + 2 + 3", {}, limits);
    CHECK(!result.ok, "operation limit should fail");
    CHECK(result.error == "operation_limit_exceeded", "wrong error");
    PASS();
}

void test_evaluate_rejects_timeout_budget_exceeded() {
    TEST(evaluate_rejects_timeout_budget_exceeded);
    WatchEvalLimits limits;
    limits.maxOperations = 100;
    limits.timeoutBudgetMs = 1;
    const auto result = WatchExpressionEvaluatorHardening::evaluate("1 + 2 + 3", {}, limits);
    CHECK(!result.ok, "timeout should fail");
    CHECK(result.timedOut, "timedOut flag should be set");
    CHECK(result.error == "evaluation_timeout", "wrong error");
    PASS();
}

void test_evaluate_rejects_division_by_zero() {
    TEST(evaluate_rejects_division_by_zero);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("10 / 0", {});
    CHECK(!result.ok, "division by zero should fail");
    CHECK(result.error == "division_by_zero", "wrong error");
    PASS();
}

void test_evaluate_detects_operator_operand_mismatch() {
    TEST(evaluate_detects_operator_operand_mismatch);
    const auto result = WatchExpressionEvaluatorHardening::evaluate("1 +", {});
    CHECK(!result.ok, "mismatch should fail");
    CHECK(result.error == "operator_operand_mismatch", "wrong error");
    PASS();
}

void test_evaluate_is_deterministic_for_same_input() {
    TEST(evaluate_is_deterministic_for_same_input);
    std::map<std::string, std::int64_t> scope{{"a", 5}, {"b", 6}};
    const auto a = WatchExpressionEvaluatorHardening::evaluate("a * b + 2", scope);
    const auto b = WatchExpressionEvaluatorHardening::evaluate("a * b + 2", scope);
    CHECK(a.ok && b.ok, "both evaluations should succeed");
    CHECK(a.value == b.value, "values should match");
    CHECK(a.operations == b.operations, "operation counts should match");
    PASS();
}

int main() {
    std::cout << "Step 570: Watch Expression Evaluator Hardening\n";

    test_evaluate_literal_expression();                    // 1
    test_evaluate_scope_identifier_expression();           // 2
    test_evaluate_rejects_empty_expression();             // 3
    test_evaluate_rejects_unsafe_pattern();               // 4
    test_evaluate_rejects_unknown_identifier();           // 5
    test_evaluate_rejects_invalid_character();            // 6
    test_evaluate_rejects_token_limit_exceeded();         // 7
    test_evaluate_rejects_operation_limit_exceeded();     // 8
    test_evaluate_rejects_timeout_budget_exceeded();      // 9
    test_evaluate_rejects_division_by_zero();             // 10
    test_evaluate_detects_operator_operand_mismatch();    // 11
    test_evaluate_is_deterministic_for_same_input();      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
