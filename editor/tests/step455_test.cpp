// Step 455: Behavioral Equivalence Checking Tests (12 tests)

#include "BehavioralEquivalence.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_sort_infers_three_standard_inputs() {
    TEST(sort_infers_three_standard_inputs);
    auto r = BehavioralChecker::check(
        "sort_values",
        "bubble sort with swap",
        "items.sort()",
        "python",
        "rust");
    CHECK(r.assertionCount() == 3, "sort should infer 3 assertions");
    CHECK(r.hasAssertionFor("[3,1,2]"), "expected primary sort input");
    PASS();
}

void test_search_infers_found_and_not_found_cases() {
    TEST(search_infers_found_and_not_found_cases);
    auto r = BehavioralChecker::check(
        "find_user",
        "find item by key",
        "items.iter().find(...)",
        "python",
        "rust");
    CHECK(r.hasAssertionFor("[1,2,3], key=2"), "expected found case");
    CHECK(r.hasAssertionFor("[1,2,3], key=99"), "expected not-found case");
    PASS();
}

void test_sum_infers_empty_and_non_empty_cases() {
    TEST(sum_infers_empty_and_non_empty_cases);
    auto r = BehavioralChecker::check(
        "sum_values",
        "sum over list with total += x",
        "items.iter().fold(0, |acc, x| acc + x)",
        "python",
        "rust");
    CHECK(r.assertionCount() == 2, "sum should infer 2 assertions");
    CHECK(r.hasAssertionFor("[1,2,3]"), "expected non-empty case");
    CHECK(r.hasAssertionFor("[]"), "expected empty case");
    PASS();
}

void test_custom_inputs_override_inference() {
    TEST(custom_inputs_override_inference);
    auto r = BehavioralChecker::check(
        "custom_fn",
        "sort values",
        "items.sort()",
        "python",
        "rust",
        {"A", "B", "C"});
    CHECK(r.assertionCount() == 3, "expected custom input count");
    CHECK(r.hasAssertionFor("A"), "expected custom input A");
    CHECK(!r.hasAssertionFor("[3,1,2]"), "inferred input should not be present");
    PASS();
}

void test_stdlib_mapping_gets_verified_contract() {
    TEST(stdlib_mapping_gets_verified_contract);
    auto r = BehavioralChecker::check(
        "sort_values",
        "sort values",
        "items.sort()",
        "python",
        "rust");
    CHECK(r.minConfidence() >= 0.90f, "expected high confidence");
    CHECK(r.annotation == "@Contract(verified)", "expected verified contract");
    PASS();
}

void test_iterator_mapping_gets_verified_contract() {
    TEST(iterator_mapping_gets_verified_contract);
    auto r = BehavioralChecker::check(
        "sum_values",
        "sum values",
        "items.iter().fold(0, |a, x| a + x)",
        "python",
        "rust");
    CHECK(r.minConfidence() >= 0.90f, "iterator mapping should be high confidence");
    CHECK(r.annotation == "@Contract(verified)", "expected verified contract");
    PASS();
}

void test_structural_todo_gets_unverified_review() {
    TEST(structural_todo_gets_unverified_review);
    auto r = BehavioralChecker::check(
        "mystery",
        "unknown transform",
        "TODO: translate this manually",
        "python",
        "rust");
    CHECK(r.minConfidence() <= 0.50f, "TODO should be low confidence");
    CHECK(r.annotation.find("@Contract(unverified)") != std::string::npos,
          "expected unverified contract");
    CHECK(r.annotation.find("@Review(required,human)") != std::string::npos,
          "expected human review annotation");
    PASS();
}

void test_cross_language_non_todo_gets_likely_contract() {
    TEST(cross_language_non_todo_gets_likely_contract);
    auto r = BehavioralChecker::check(
        "transform",
        "do transform",
        "transform(data)",
        "python",
        "java");
    CHECK(r.minConfidence() >= 0.70f && r.minConfidence() < 0.90f,
          "expected mid confidence range");
    CHECK(r.annotation == "@Contract(likely)", "expected likely contract");
    PASS();
}

void test_same_language_defaults_high_confidence() {
    TEST(same_language_defaults_high_confidence);
    auto r = BehavioralChecker::check(
        "identity",
        "generic function",
        "identity(x)",
        "python",
        "python");
    CHECK(r.minConfidence() >= 0.95f, "same language should be high confidence");
    CHECK(r.annotation == "@Contract(verified)", "expected verified contract");
    PASS();
}

void test_assertions_require_same_error_behavior() {
    TEST(assertions_require_same_error_behavior);
    auto r = BehavioralChecker::check(
        "sort_values",
        "sort values",
        "sorted(values)",
        "python",
        "python");
    CHECK(!r.assertions.empty(), "expected assertions");
    CHECK(r.assertions[0].sameErrors, "sameErrors should default true");
    PASS();
}

void test_default_generic_inputs_used_when_no_pattern() {
    TEST(default_generic_inputs_used_when_no_pattern);
    auto r = BehavioralChecker::check(
        "generic_fn",
        "compute something",
        "compute(data)",
        "python",
        "java");
    CHECK(r.hasAssertionFor("input_0"), "expected generic input_0");
    CHECK(r.hasAssertionFor("input_1"), "expected generic input_1");
    PASS();
}

void test_all_passing_defaults_true() {
    TEST(all_passing_defaults_true);
    auto r = BehavioralChecker::check(
        "sort_values",
        "sort values",
        "items.sort()",
        "python",
        "rust");
    CHECK(r.allPassing, "allPassing should default true");
    PASS();
}

int main() {
    std::cout << "Step 455: Behavioral Equivalence Checking Tests\n";

    test_sort_infers_three_standard_inputs();          // 1
    test_search_infers_found_and_not_found_cases();   // 2
    test_sum_infers_empty_and_non_empty_cases();      // 3
    test_custom_inputs_override_inference();          // 4
    test_stdlib_mapping_gets_verified_contract();     // 5
    test_iterator_mapping_gets_verified_contract();   // 6
    test_structural_todo_gets_unverified_review();    // 7
    test_cross_language_non_todo_gets_likely_contract(); // 8
    test_same_language_defaults_high_confidence();    // 9
    test_assertions_require_same_error_behavior();    // 10
    test_default_generic_inputs_used_when_no_pattern(); // 11
    test_all_passing_defaults_true();                 // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
