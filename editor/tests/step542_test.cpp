// Step 542: Cross-Language Consistency Gate (12 tests)

#include "CrossLanguageConsistencyGate.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_cpp_allowed_path_normalizes_to_no_failure() {
    TEST(cpp_allowed_path_normalizes_to_no_failure);
    auto r = CrossLanguageConsistencyGate::evaluate({"cpp", "declaration", "rename", false, false, false, "header"});
    CHECK(r.supported && r.allowed, "cpp allowed path should pass");
    CHECK(r.standardizedFailure.empty(), "no failure expected");
    CHECK(r.fallbackAction == "none", "no fallback expected");
    PASS();
}

void test_python_side_effect_block_maps_to_safety_guard_blocked() {
    TEST(python_side_effect_block_maps_to_safety_guard_blocked);
    auto r = CrossLanguageConsistencyGate::evaluate({"python", "module_stmt", "delete", true});
    CHECK(!r.allowed, "python side effect block expected");
    CHECK(r.standardizedFailure == "safety_guard_blocked", "safety mapping expected");
    CHECK(r.fallbackAction == "escalate_with_safety_context", "fallback mismatch");
    PASS();
}

void test_rust_lifetime_block_maps_to_safety_guard_blocked() {
    TEST(rust_lifetime_block_maps_to_safety_guard_blocked);
    auto r = CrossLanguageConsistencyGate::evaluate({"rust", "borrow_region", "extract", false, true, false});
    CHECK(!r.allowed, "rust lifetime block expected");
    CHECK(r.standardizedFailure == "safety_guard_blocked", "safety mapping expected");
    PASS();
}

void test_go_concurrency_block_maps_to_safety_guard_blocked() {
    TEST(go_concurrency_block_maps_to_safety_guard_blocked);
    auto r = CrossLanguageConsistencyGate::evaluate({"go", "channel", "extract", false, false, true});
    CHECK(!r.allowed, "go concurrency block expected");
    CHECK(r.standardizedFailure == "safety_guard_blocked", "safety mapping expected");
    PASS();
}

void test_illegal_operation_maps_to_suggest_legal_ops_fallback() {
    TEST(illegal_operation_maps_to_suggest_legal_ops_fallback);
    auto r = CrossLanguageConsistencyGate::evaluate({"typescript", "signature", "rename"});
    CHECK(!r.allowed, "illegal op expected");
    CHECK(r.standardizedFailure == "operation_not_legal_for_construct", "operation mapping expected");
    CHECK(r.fallbackAction == "suggest_legal_ops", "fallback mismatch");
    PASS();
}

void test_unsupported_construct_maps_to_readonly_fallback() {
    TEST(unsupported_construct_maps_to_readonly_fallback);
    auto r = CrossLanguageConsistencyGate::evaluate({"cpp", "template_magic", "rename"});
    CHECK(!r.supported, "unsupported construct should fail support");
    CHECK(r.standardizedFailure == "unsupported_construct", "construct mapping expected");
    CHECK(r.fallbackAction == "fallback_to_readonly", "fallback mismatch");
    PASS();
}

void test_unsupported_language_maps_to_readonly_fallback() {
    TEST(unsupported_language_maps_to_readonly_fallback);
    auto r = CrossLanguageConsistencyGate::evaluate({"haskell", "function", "rename"});
    CHECK(!r.supported, "unsupported language should fail support");
    CHECK(r.standardizedFailure == "unsupported_language", "language mapping expected");
    CHECK(r.fallbackAction == "fallback_to_readonly", "fallback mismatch");
    PASS();
}

void test_python_allowed_path_is_consistent() {
    TEST(python_allowed_path_is_consistent);
    auto r = CrossLanguageConsistencyGate::evaluate({"python", "function", "rename"});
    CHECK(r.supported && r.allowed, "python allowed path expected");
    CHECK(r.fallbackAction == "none", "allowed path fallback should be none");
    PASS();
}

void test_rust_allowed_path_is_consistent() {
    TEST(rust_allowed_path_is_consistent);
    auto r = CrossLanguageConsistencyGate::evaluate({"rust", "function", "rename"});
    CHECK(r.supported && r.allowed, "rust allowed path expected");
    PASS();
}

void test_cpp_boundary_block_maps_to_safety_guard_blocked() {
    TEST(cpp_boundary_block_maps_to_safety_guard_blocked);
    auto r = CrossLanguageConsistencyGate::evaluate({"cpp", "definition", "inline", false, false, false, "header"});
    CHECK(!r.allowed, "cpp boundary block expected");
    CHECK(r.standardizedFailure == "safety_guard_blocked", "safety mapping expected");
    PASS();
}

void test_unknown_diag_maps_to_constraint_failure_fallback_retry() {
    TEST(unknown_diag_maps_to_constraint_failure_fallback_retry);
    auto r = CrossLanguageConsistencyGate::evaluate({"cpp", "member", "extract", false, false, false, "source"});
    CHECK(!r.allowed, "member extract in source should fail");
    CHECK(r.standardizedFailure == "operation_not_legal_for_construct", "expected canonical op failure");
    CHECK(r.fallbackAction == "suggest_legal_ops", "expected legal op fallback");
    PASS();
}

void test_diagnostics_are_preserved_for_downstream_use() {
    TEST(diagnostics_are_preserved_for_downstream_use);
    auto r = CrossLanguageConsistencyGate::evaluate({"go", "channel", "extract", false, false, true});
    CHECK(!r.diagnostics.empty(), "raw diagnostics should be preserved");
    CHECK(r.diagnostics[0] == "go_concurrency_extract_blocked", "expected raw diagnostic");
    PASS();
}

int main() {
    std::cout << "Step 542: Cross-Language Consistency Gate\n";

    test_cpp_allowed_path_normalizes_to_no_failure();                // 1
    test_python_side_effect_block_maps_to_safety_guard_blocked();    // 2
    test_rust_lifetime_block_maps_to_safety_guard_blocked();         // 3
    test_go_concurrency_block_maps_to_safety_guard_blocked();        // 4
    test_illegal_operation_maps_to_suggest_legal_ops_fallback();     // 5
    test_unsupported_construct_maps_to_readonly_fallback();          // 6
    test_unsupported_language_maps_to_readonly_fallback();           // 7
    test_python_allowed_path_is_consistent();                        // 8
    test_rust_allowed_path_is_consistent();                          // 9
    test_cpp_boundary_block_maps_to_safety_guard_blocked();          // 10
    test_unknown_diag_maps_to_constraint_failure_fallback_retry();   // 11
    test_diagnostics_are_preserved_for_downstream_use();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
