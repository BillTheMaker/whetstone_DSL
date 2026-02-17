// Step 609: Change Freeze Calendar (12 tests)

#include "ChangeFreezeCalendar.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static FreezeWindow window(const std::string& id,
                           const std::string& env,
                           int startDay,
                           int endDay) {
    return {id, env, startDay, endDay, "release_freeze"};
}

void test_add_window_success() {
    TEST(add_window_success);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "add should succeed");
    CHECK(cal.windowCount("prod") == 1, "count mismatch");
    PASS();
}

void test_add_window_rejects_missing_window_id() {
    TEST(add_window_rejects_missing_window_id);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(!cal.addWindow(window("", "prod", 10, 12), &error), "add should fail");
    CHECK(error == "window_id_missing", "wrong error");
    PASS();
}

void test_add_window_rejects_missing_environment_id() {
    TEST(add_window_rejects_missing_environment_id);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(!cal.addWindow(window("w1", "", 10, 12), &error), "add should fail");
    CHECK(error == "environment_id_missing", "wrong error");
    PASS();
}

void test_add_window_rejects_invalid_range() {
    TEST(add_window_rejects_invalid_range);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(!cal.addWindow(window("w1", "prod", 12, 10), &error), "add should fail");
    CHECK(error == "window_range_invalid", "wrong error");
    PASS();
}

void test_add_window_rejects_duplicate() {
    TEST(add_window_rejects_duplicate);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "first add failed");
    CHECK(!cal.addWindow(window("w1", "prod", 13, 15), &error), "duplicate should fail");
    CHECK(error == "window_duplicate", "wrong error");
    PASS();
}

void test_frozen_true_inside_window() {
    TEST(frozen_true_inside_window);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "add failed");
    CHECK(cal.frozen("prod", 11, &error), "should be frozen");
    PASS();
}

void test_frozen_false_outside_window() {
    TEST(frozen_false_outside_window);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "add failed");
    CHECK(!cal.frozen("prod", 13, &error), "should not be frozen");
    PASS();
}

void test_frozen_rejects_missing_environment() {
    TEST(frozen_rejects_missing_environment);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(!cal.frozen("", 10, &error), "query should fail");
    CHECK(error == "environment_id_missing", "wrong error");
    PASS();
}

void test_frozen_rejects_invalid_day() {
    TEST(frozen_rejects_invalid_day);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(!cal.frozen("prod", -1, &error), "query should fail");
    CHECK(error == "day_invalid", "wrong error");
    PASS();
}

void test_window_count_empty_environment_returns_all() {
    TEST(window_count_empty_environment_returns_all);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "add w1 failed");
    CHECK(cal.addWindow(window("w2", "staging", 10, 12), &error), "add w2 failed");
    CHECK(cal.windowCount("") == 2, "all count mismatch");
    PASS();
}

void test_active_on_day_returns_matching_windows() {
    TEST(active_on_day_returns_matching_windows);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "add w1 failed");
    CHECK(cal.addWindow(window("w2", "staging", 11, 13), &error), "add w2 failed");
    CHECK(cal.activeOnDay(11).size() == 2, "active size mismatch");
    PASS();
}

void test_active_on_day_returns_empty_when_none() {
    TEST(active_on_day_returns_empty_when_none);
    ChangeFreezeCalendar cal;
    std::string error;
    CHECK(cal.addWindow(window("w1", "prod", 10, 12), &error), "add failed");
    CHECK(cal.activeOnDay(30).empty(), "active set should be empty");
    PASS();
}

int main() {
    std::cout << "Step 609: Change Freeze Calendar\n";

    test_add_window_success();                         // 1
    test_add_window_rejects_missing_window_id();      // 2
    test_add_window_rejects_missing_environment_id(); // 3
    test_add_window_rejects_invalid_range();          // 4
    test_add_window_rejects_duplicate();              // 5
    test_frozen_true_inside_window();                 // 6
    test_frozen_false_outside_window();               // 7
    test_frozen_rejects_missing_environment();        // 8
    test_frozen_rejects_invalid_day();                // 9
    test_window_count_empty_environment_returns_all();// 10
    test_active_on_day_returns_matching_windows();    // 11
    test_active_on_day_returns_empty_when_none();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
