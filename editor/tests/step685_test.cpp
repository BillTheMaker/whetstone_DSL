// Step 685: TaskCompletionMetrics (12 tests)

#include "TaskCompletionMetrics.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void t1() {
    TEST(empty_before_after_zero_changes);
    auto o = TaskCompletionMetrics::diff("T", "", "", false, 50);
    CHECK(o.linesChanged == 0, "expected 0");
    PASS();
}

void t2() {
    TEST(added_lines_counted);
    auto o = TaskCompletionMetrics::diff("T", "", "a\nb\n", false, 50);
    CHECK(o.linesAdded == 2, "expected 2 added");
    PASS();
}

void t3() {
    TEST(removed_lines_counted);
    auto o = TaskCompletionMetrics::diff("T", "a\nb\n", "", false, 50);
    CHECK(o.linesRemoved == 2, "expected 2 removed");
    PASS();
}

void t4() {
    TEST(lines_changed_equals_added_plus_removed);
    auto o = TaskCompletionMetrics::diff("T", "a\n", "a\nb\n", false, 50);
    CHECK(o.linesChanged == o.linesAdded + o.linesRemoved, "sum mismatch");
    PASS();
}

void t5() {
    TEST(tests_passed_preserved);
    auto o = TaskCompletionMetrics::diff("T", "", "", true, 50);
    CHECK(o.testsPassed, "testsPassed mismatch");
    PASS();
}

void t6() {
    TEST(quality_clamped_range);
    auto low = TaskCompletionMetrics::diff("T", "", "", false, -5);
    auto high = TaskCompletionMetrics::diff("T", "", "", false, 500);
    CHECK(low.qualityRating == 0, "low clamp");
    CHECK(high.qualityRating == 100, "high clamp");
    PASS();
}

void t7() {
    TEST(notes_preserved);
    auto o = TaskCompletionMetrics::diff("T", "", "", false, 50, "note");
    CHECK(o.notes == "note", "notes mismatch");
    PASS();
}

void t8() {
    TEST(task_id_preserved);
    auto o = TaskCompletionMetrics::diff("ABC", "", "", false, 50);
    CHECK(o.taskId == "ABC", "task id mismatch");
    PASS();
}

void t9() {
    TEST(count_lines_empty_zero);
    CHECK(TaskCompletionMetrics::countLines("") == 0, "expected 0");
    PASS();
}

void t10() {
    TEST(count_lines_a_b_newline_is_two);
    CHECK(TaskCompletionMetrics::countLines("a\nb\n") == 2, "expected 2");
    PASS();
}

void t11() {
    TEST(count_lines_without_trailing_newline_is_two);
    CHECK(TaskCompletionMetrics::countLines("a\nb") == 2, "expected 2");
    PASS();
}

void t12() {
    TEST(tests_provided_false_when_not_given);
    auto o = TaskCompletionMetrics::diff("T", "", "", false, 50);
    CHECK(!o.testsProvided, "testsProvided should be false");
    PASS();
}

int main() {
    std::cout << "Step 685: TaskCompletionMetrics\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

