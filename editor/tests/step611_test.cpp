// Step 611: On-Call Coverage Planner (12 tests)

#include "OnCallCoveragePlanner.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static OnCallShift shift(const std::string& id,
                         const std::string& svc,
                         int start,
                         int end) {
    return {id, svc, "alice", "bob", start, end};
}

void test_add_shift_success() {
    TEST(add_shift_success);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 0, 8), &error), "add should succeed");
    CHECK(planner.shiftCount("svc-a") == 1, "shift count mismatch");
    PASS();
}

void test_add_shift_rejects_missing_shift_id() {
    TEST(add_shift_rejects_missing_shift_id);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(!planner.addShift(shift("", "svc-a", 0, 8), &error), "add should fail");
    CHECK(error == "shift_id_missing", "wrong error");
    PASS();
}

void test_add_shift_rejects_missing_service_id() {
    TEST(add_shift_rejects_missing_service_id);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(!planner.addShift(shift("s1", "", 0, 8), &error), "add should fail");
    CHECK(error == "service_id_missing", "wrong error");
    PASS();
}

void test_add_shift_rejects_invalid_range() {
    TEST(add_shift_rejects_invalid_range);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(!planner.addShift(shift("s1", "svc-a", 8, 8), &error), "add should fail");
    CHECK(error == "shift_range_invalid", "wrong error");
    PASS();
}

void test_add_shift_rejects_duplicate() {
    TEST(add_shift_rejects_duplicate);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 0, 8), &error), "first add failed");
    CHECK(!planner.addShift(shift("s1", "svc-a", 8, 16), &error), "duplicate should fail");
    CHECK(error == "shift_duplicate", "wrong error");
    PASS();
}

void test_covered_true_within_shift() {
    TEST(covered_true_within_shift);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 0, 8), &error), "add failed");
    CHECK(planner.covered("svc-a", 4, &error), "should be covered");
    PASS();
}

void test_covered_false_outside_shift() {
    TEST(covered_false_outside_shift);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 0, 8), &error), "add failed");
    CHECK(!planner.covered("svc-a", 9, &error), "should not be covered");
    PASS();
}

void test_covered_rejects_invalid_hour() {
    TEST(covered_rejects_invalid_hour);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(!planner.covered("svc-a", 24, &error), "query should fail");
    CHECK(error == "hour_invalid", "wrong error");
    PASS();
}

void test_shift_count_empty_returns_all() {
    TEST(shift_count_empty_returns_all);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 0, 8), &error), "add s1 failed");
    CHECK(planner.addShift(shift("s2", "svc-b", 0, 8), &error), "add s2 failed");
    CHECK(planner.shiftCount("") == 2, "all shift count mismatch");
    PASS();
}

void test_coverage_gaps_full_day_zero() {
    TEST(coverage_gaps_full_day_zero);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 0, 12), &error), "add s1 failed");
    CHECK(planner.addShift(shift("s2", "svc-a", 12, 24), &error), "add s2 failed");
    CHECK(planner.coverageGaps("svc-a") == 0, "gaps should be zero");
    PASS();
}

void test_coverage_gaps_partial_day() {
    TEST(coverage_gaps_partial_day);
    OnCallCoveragePlanner planner;
    std::string error;
    CHECK(planner.addShift(shift("s1", "svc-a", 8, 16), &error), "add failed");
    CHECK(planner.coverageGaps("svc-a") == 16, "gap count mismatch");
    PASS();
}

void test_coverage_gaps_unknown_service_full_day() {
    TEST(coverage_gaps_unknown_service_full_day);
    OnCallCoveragePlanner planner;
    CHECK(planner.coverageGaps("missing") == 24, "unknown service should have full-day gaps");
    PASS();
}

int main() {
    std::cout << "Step 611: On-Call Coverage Planner\n";

    test_add_shift_success();                    // 1
    test_add_shift_rejects_missing_shift_id();  // 2
    test_add_shift_rejects_missing_service_id();// 3
    test_add_shift_rejects_invalid_range();     // 4
    test_add_shift_rejects_duplicate();         // 5
    test_covered_true_within_shift();           // 6
    test_covered_false_outside_shift();         // 7
    test_covered_rejects_invalid_hour();        // 8
    test_shift_count_empty_returns_all();       // 9
    test_coverage_gaps_full_day_zero();         // 10
    test_coverage_gaps_partial_day();           // 11
    test_coverage_gaps_unknown_service_full_day();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
