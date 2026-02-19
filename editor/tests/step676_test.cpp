// Step 676: TokenBudgetEnforcer (12 tests)

#include "TokenBudgetEnforcer.h"

#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static ContextSlice makeSlice(const std::string& file, const std::string& content) {
    ContextSlice s;
    s.filePath = file;
    s.content = content;
    s.found = true;
    s.lineStart = 1;
    s.lineEnd = 1;
    return s;
}

void t1() {
    TEST(empty_slices_returns_zero_report);
    auto [included, report] = TokenBudgetEnforcer::enforce({}, 100);
    CHECK(included.empty(), "included should be empty");
    CHECK(report.totalSlices == 0, "totalSlices should be 0");
    CHECK(report.estimatedTokensUsed == 0, "tokens should be 0");
    PASS();
}

void t2() {
    TEST(single_slice_under_budget_kept);
    auto [included, report] = TokenBudgetEnforcer::enforce({makeSlice("a.cpp", "abcd")}, 2);
    CHECK(included.size() == 1, "slice should be kept");
    CHECK(report.includedSlices == 1, "included count mismatch");
    PASS();
}

void t3() {
    TEST(single_slice_over_budget_dropped);
    auto [included, report] = TokenBudgetEnforcer::enforce({makeSlice("a.cpp", "abcdefgh")}, 1);
    CHECK(included.empty(), "slice should be dropped");
    CHECK(report.droppedSlices == 1, "dropped count mismatch");
    PASS();
}

void t4() {
    TEST(priority_order_keeps_earlier_slices);
    std::vector<ContextSlice> in = {
        makeSlice("a.cpp", "abcdefgh"), // 2 tokens
        makeSlice("b.cpp", "abcdefgh"), // 2 tokens
        makeSlice("c.cpp", "abcdefgh")  // 2 tokens
    };
    auto [included, report] = TokenBudgetEnforcer::enforce(in, 4);
    CHECK(included.size() == 2, "expected 2 included");
    CHECK(included[0].filePath == "a.cpp", "priority order broken");
    CHECK(included[1].filePath == "b.cpp", "priority order broken");
    CHECK(report.droppedSlices == 1, "expected one drop");
    PASS();
}

void t5() {
    TEST(dropped_files_list_matches_drops);
    std::vector<ContextSlice> in = {
        makeSlice("a.cpp", "abcdefgh"),
        makeSlice("b.cpp", "abcdefgh")
    };
    auto [included, report] = TokenBudgetEnforcer::enforce(in, 2);
    CHECK(report.droppedFiles.size() == 1, "expected one dropped file");
    CHECK(report.droppedFiles[0] == "b.cpp", "wrong dropped file");
    PASS();
}

void t6() {
    TEST(estimated_tokens_used_within_budget_when_fits);
    auto [included, report] = TokenBudgetEnforcer::enforce({makeSlice("a.cpp", "abcd")}, 2);
    CHECK(report.estimatedTokensUsed <= report.budgetTokens, "used should not exceed budget");
    PASS();
}

void t7() {
    TEST(budget_exceeded_false_when_all_fit);
    auto [included, report] = TokenBudgetEnforcer::enforce({makeSlice("a.cpp", "abcd")}, 10);
    CHECK(!report.budgetExceeded, "budgetExceeded should be false");
    PASS();
}

void t8() {
    TEST(budget_exceeded_true_when_any_dropped);
    auto [included, report] = TokenBudgetEnforcer::enforce({makeSlice("a.cpp", "abcdefghijkl")}, 1);
    CHECK(report.budgetExceeded, "budgetExceeded should be true");
    PASS();
}

void t9() {
    TEST(estimate_tokens_empty_is_zero);
    CHECK(TokenBudgetEnforcer::estimateTokens("") == 0, "expected 0");
    PASS();
}

void t10() {
    TEST(estimate_tokens_abcd_is_one);
    CHECK(TokenBudgetEnforcer::estimateTokens("abcd") == 1, "expected 1");
    PASS();
}

void t11() {
    TEST(report_total_slices_matches_input);
    std::vector<ContextSlice> in = {makeSlice("a.cpp", "x"), makeSlice("b.cpp", "y")};
    auto [included, report] = TokenBudgetEnforcer::enforce(in, 100);
    CHECK(report.totalSlices == 2, "expected total 2");
    PASS();
}

void t12() {
    TEST(included_plus_dropped_equals_total);
    std::vector<ContextSlice> in = {makeSlice("a.cpp", "abcdefgh"), makeSlice("b.cpp", "abcdefgh")};
    auto [included, report] = TokenBudgetEnforcer::enforce(in, 2);
    CHECK(report.includedSlices + report.droppedSlices == report.totalSlices, "count mismatch");
    PASS();
}

int main() {
    std::cout << "Step 676: TokenBudgetEnforcer\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

