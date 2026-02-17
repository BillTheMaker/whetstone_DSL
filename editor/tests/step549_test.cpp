// Step 549: Suggestion Engine for Cost Reductions (12 tests)

#include "CostReductionSuggestionEngine.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasType(const std::vector<CostSuggestion>& s, const std::string& type) {
    for (const auto& x : s) if (x.type == type) return true;
    return false;
}

static int priorityOf(const std::vector<CostSuggestion>& s, const std::string& type) {
    for (const auto& x : s) if (x.type == type) return x.priority;
    return -1;
}

void test_template_suggestion_emitted_when_available() {
    TEST(template_suggestion_emitted_when_available);
    auto s = CostReductionSuggestionEngine::suggest({5, 6, true, false, false, 100, 200, 500, 1000});
    CHECK(hasType(s, "template_substitution"), "template suggestion expected");
    PASS();
}

void test_batching_suggestion_emitted_when_possible() {
    TEST(batching_suggestion_emitted_when_possible);
    auto s = CostReductionSuggestionEngine::suggest({4, 2, false, true, false, 100, 200, 500, 1000});
    CHECK(hasType(s, "batching"), "batching suggestion expected");
    PASS();
}

void test_context_narrowing_suggestion_emitted_when_possible() {
    TEST(context_narrowing_suggestion_emitted_when_possible);
    auto s = CostReductionSuggestionEngine::suggest({1, 10, false, false, true, 100, 200, 500, 1000});
    CHECK(hasType(s, "context_narrowing"), "context narrowing suggestion expected");
    PASS();
}

void test_no_suggestions_when_no_optimization_paths_available() {
    TEST(no_suggestions_when_no_optimization_paths_available);
    auto s = CostReductionSuggestionEngine::suggest({1, 2, false, false, false, 100, 200, 500, 1000});
    CHECK(s.empty(), "no suggestions expected");
    PASS();
}

void test_over_budget_increases_priorities() {
    TEST(over_budget_increases_priorities);
    auto low = CostReductionSuggestionEngine::suggest({4, 8, true, true, true, 100, 200, 500, 1000});
    auto high = CostReductionSuggestionEngine::suggest({4, 8, true, true, true, 400, 200, 1500, 1000});
    CHECK(priorityOf(high, "template_substitution") > priorityOf(low, "template_substitution"),
          "over-budget should raise template priority");
    CHECK(priorityOf(high, "batching") > priorityOf(low, "batching"),
          "over-budget should raise batching priority");
    CHECK(priorityOf(high, "context_narrowing") > priorityOf(low, "context_narrowing"),
          "over-budget should raise context priority");
    PASS();
}

void test_template_savings_scale_with_operation_count() {
    TEST(template_savings_scale_with_operation_count);
    auto a = CostReductionSuggestionEngine::suggest({2, 4, true, false, false, 100, 200, 500, 1000});
    auto b = CostReductionSuggestionEngine::suggest({8, 4, true, false, false, 100, 200, 500, 1000});
    int sa = 0, sb = 0;
    for (const auto& x : a) if (x.type == "template_substitution") sa = x.estimatedTokenSavings;
    for (const auto& x : b) if (x.type == "template_substitution") sb = x.estimatedTokenSavings;
    CHECK(sb > sa, "template savings should scale with operations");
    PASS();
}

void test_context_savings_scale_with_symbol_count() {
    TEST(context_savings_scale_with_symbol_count);
    auto a = CostReductionSuggestionEngine::suggest({1, 5, false, false, true, 100, 200, 500, 1000});
    auto b = CostReductionSuggestionEngine::suggest({1, 12, false, false, true, 100, 200, 500, 1000});
    int sa = 0, sb = 0;
    for (const auto& x : a) if (x.type == "context_narrowing") sa = x.estimatedTokenSavings;
    for (const auto& x : b) if (x.type == "context_narrowing") sb = x.estimatedTokenSavings;
    CHECK(sb > sa, "context savings should scale with symbol count");
    PASS();
}

void test_batching_requires_multiple_operations() {
    TEST(batching_requires_multiple_operations);
    auto s = CostReductionSuggestionEngine::suggest({1, 5, false, true, false, 100, 200, 500, 1000});
    CHECK(!hasType(s, "batching"), "batching should require >=2 operations");
    PASS();
}

void test_context_narrowing_requires_more_than_three_symbols() {
    TEST(context_narrowing_requires_more_than_three_symbols);
    auto s = CostReductionSuggestionEngine::suggest({2, 3, false, false, true, 100, 200, 500, 1000});
    CHECK(!hasType(s, "context_narrowing"), "context narrowing should require >3 symbols");
    PASS();
}

void test_suggestions_sorted_by_priority_descending() {
    TEST(suggestions_sorted_by_priority_descending);
    auto s = CostReductionSuggestionEngine::suggest({5, 10, true, true, true, 400, 200, 1500, 1000});
    CHECK(s.size() >= 2, "need at least two suggestions");
    CHECK(s[0].priority >= s[1].priority, "suggestions should be priority sorted");
    PASS();
}

void test_tiebreaker_by_estimated_savings() {
    TEST(tiebreaker_by_estimated_savings);
    auto s = CostReductionSuggestionEngine::suggest({4, 10, true, true, false, 100, 200, 500, 1000});
    CHECK(s.size() >= 2, "need at least two suggestions");
    if (s[0].priority == s[1].priority) {
        CHECK(s[0].estimatedTokenSavings >= s[1].estimatedTokenSavings,
              "tie should break by estimated savings");
    }
    PASS();
}

void test_rationales_are_present() {
    TEST(rationales_are_present);
    auto s = CostReductionSuggestionEngine::suggest({5, 10, true, true, true, 100, 200, 500, 1000});
    for (const auto& x : s) {
        CHECK(!x.rationale.empty(), "rationale should be present");
    }
    PASS();
}

int main() {
    std::cout << "Step 549: Suggestion Engine for Cost Reductions\n";

    test_template_suggestion_emitted_when_available();         // 1
    test_batching_suggestion_emitted_when_possible();          // 2
    test_context_narrowing_suggestion_emitted_when_possible(); // 3
    test_no_suggestions_when_no_optimization_paths_available();// 4
    test_over_budget_increases_priorities();                   // 5
    test_template_savings_scale_with_operation_count();        // 6
    test_context_savings_scale_with_symbol_count();            // 7
    test_batching_requires_multiple_operations();              // 8
    test_context_narrowing_requires_more_than_three_symbols(); // 9
    test_suggestions_sorted_by_priority_descending();          // 10
    test_tiebreaker_by_estimated_savings();                    // 11
    test_rationales_are_present();                             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
