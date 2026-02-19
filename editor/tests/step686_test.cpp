// Step 686: ABTestComparison (12 tests)

#include "ABTestComparison.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

static SessionRecord makeSession(int tokens, int reads, long long dur) {
    SessionRecord s;
    s.totalEstimatedTokens = tokens;
    s.fileReadCount = reads;
    s.totalDurationMs = dur;
    return s;
}

void t1() {
    TEST(token_reduction_pct_computed);
    auto r = ABTestComparison::compare("T", makeSession(100, 0, 0), makeSession(60, 0, 0), 50, 50);
    CHECK(r.tokenReductionPct == 40.0, "expected 40%");
    PASS();
}

void t2() {
    TEST(file_read_reduction_pct_computed);
    auto r = ABTestComparison::compare("T", makeSession(0, 10, 0), makeSession(0, 5, 0), 50, 50);
    CHECK(r.fileReadReductionPct == 50.0, "expected 50%");
    PASS();
}

void t3() {
    TEST(duration_delta_pct_positive_when_faster);
    auto r = ABTestComparison::compare("T", makeSession(0, 0, 200), makeSession(0, 0, 100), 50, 50);
    CHECK(r.durationDeltaPct == 50.0, "expected 50%");
    PASS();
}

void t4() {
    TEST(quality_delta_computed);
    auto r = ABTestComparison::compare("T", makeSession(0, 0, 0), makeSession(0, 0, 0), 70, 80);
    CHECK(r.qualityDelta == 10, "expected 10");
    PASS();
}

void t5() {
    TEST(whetstone_won_true_when_tokens_down_quality_not_down);
    auto r = ABTestComparison::compare("T", makeSession(100, 0, 0), makeSession(50, 0, 0), 80, 80);
    CHECK(r.whetstoneWon, "expected won");
    PASS();
}

void t6() {
    TEST(whetstone_won_false_when_tokens_higher);
    auto r = ABTestComparison::compare("T", makeSession(50, 0, 0), makeSession(60, 0, 0), 80, 80);
    CHECK(!r.whetstoneWon, "should not win");
    PASS();
}

void t7() {
    TEST(token_reduction_zero_when_equal_tokens);
    auto r = ABTestComparison::compare("T", makeSession(50, 0, 0), makeSession(50, 0, 0), 80, 80);
    CHECK(r.tokenReductionPct == 0.0, "expected 0");
    PASS();
}

void t8() {
    TEST(token_reduction_negative_allowed);
    auto r = ABTestComparison::compare("T", makeSession(50, 0, 0), makeSession(100, 0, 0), 80, 80);
    CHECK(r.tokenReductionPct < 0.0, "expected negative");
    PASS();
}

void t9() {
    TEST(baseline_tokens_preserved);
    auto r = ABTestComparison::compare("T", makeSession(123, 0, 0), makeSession(0, 0, 0), 0, 0);
    CHECK(r.baselineTokens == 123, "baseline tokens mismatch");
    PASS();
}

void t10() {
    TEST(whetstone_tokens_preserved);
    auto r = ABTestComparison::compare("T", makeSession(0, 0, 0), makeSession(321, 0, 0), 0, 0);
    CHECK(r.whetstoneTokens == 321, "whetstone tokens mismatch");
    PASS();
}

void t11() {
    TEST(task_id_preserved);
    auto r = ABTestComparison::compare("TaskX", makeSession(0, 0, 0), makeSession(0, 0, 0), 0, 0);
    CHECK(r.taskId == "TaskX", "task id mismatch");
    PASS();
}

void t12() {
    TEST(zero_baseline_tokens_no_divide_by_zero);
    auto r = ABTestComparison::compare("T", makeSession(0, 0, 0), makeSession(100, 0, 0), 0, 0);
    CHECK(r.tokenReductionPct == 0.0, "expected 0");
    PASS();
}

int main() {
    std::cout << "Step 686: ABTestComparison\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

