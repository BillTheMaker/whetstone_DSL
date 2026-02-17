// Step 555: Breakpoint System (12 tests)

#include "BreakpointSystem.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static const Breakpoint* findBp(const std::vector<Breakpoint>& list, const std::string& id) {
    for (const auto& bp : list) if (bp.id == id) return &bp;
    return nullptr;
}

void test_add_line_breakpoint() {
    TEST(add_line_breakpoint);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 10), "line breakpoint should add");
    CHECK(b.list().size() == 1, "one breakpoint expected");
    PASS();
}

void test_add_conditional_breakpoint() {
    TEST(add_conditional_breakpoint);
    BreakpointSystem b;
    CHECK(b.addConditionalBreakpoint("bp1", "a.cpp", 12, "x > 0"), "conditional breakpoint should add");
    auto bp = findBp(b.list(), "bp1");
    CHECK(bp && bp->condition == "x > 0", "condition should persist");
    PASS();
}

void test_add_hit_count_breakpoint() {
    TEST(add_hit_count_breakpoint);
    BreakpointSystem b;
    CHECK(b.addHitCountBreakpoint("bp1", "a.cpp", 5, 3), "hit-count breakpoint should add");
    auto bp = findBp(b.list(), "bp1");
    CHECK(bp && bp->hitCountTarget == 3, "hit target should persist");
    PASS();
}

void test_add_invalid_hit_count_breakpoint_fails() {
    TEST(add_invalid_hit_count_breakpoint_fails);
    BreakpointSystem b;
    CHECK(!b.addHitCountBreakpoint("bp1", "a.cpp", 5, 0), "zero hit count should fail");
    PASS();
}

void test_enable_disable_persistence() {
    TEST(enable_disable_persistence);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 10), "add should work");
    CHECK(b.disable("bp1"), "disable should work");
    auto bp = findBp(b.list(), "bp1");
    CHECK(bp && !bp->enabled, "breakpoint should be disabled");
    CHECK(b.enable("bp1"), "enable should work");
    bp = findBp(b.list(), "bp1");
    CHECK(bp && bp->enabled, "breakpoint should be re-enabled");
    PASS();
}

void test_should_break_simple_enabled_breakpoint() {
    TEST(should_break_simple_enabled_breakpoint);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 10), "add should work");
    CHECK(b.shouldBreak("bp1", true), "enabled line breakpoint should break");
    PASS();
}

void test_should_not_break_when_disabled() {
    TEST(should_not_break_when_disabled);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 10), "add should work");
    CHECK(b.disable("bp1"), "disable should work");
    CHECK(!b.shouldBreak("bp1", true), "disabled breakpoint should not break");
    PASS();
}

void test_conditional_breakpoint_requires_true_condition() {
    TEST(conditional_breakpoint_requires_true_condition);
    BreakpointSystem b;
    CHECK(b.addConditionalBreakpoint("bp1", "a.cpp", 10, "x > 0"), "add should work");
    CHECK(!b.shouldBreak("bp1", false), "false condition should not break");
    CHECK(b.shouldBreak("bp1", true), "true condition should break");
    PASS();
}

void test_hit_count_breakpoint_triggers_at_threshold() {
    TEST(hit_count_breakpoint_triggers_at_threshold);
    BreakpointSystem b;
    CHECK(b.addHitCountBreakpoint("bp1", "a.cpp", 10, 3), "add should work");
    CHECK(!b.shouldBreak("bp1", true), "hit1 should not break");
    CHECK(!b.shouldBreak("bp1", true), "hit2 should not break");
    CHECK(b.shouldBreak("bp1", true), "hit3 should break");
    PASS();
}

void test_reconcile_on_file_edit_shifts_lines() {
    TEST(reconcile_on_file_edit_shifts_lines);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 10), "add bp1");
    CHECK(b.addLineBreakpoint("bp2", "a.cpp", 20), "add bp2");
    b.reconcileOnFileEdit("a.cpp", 15, 3);
    auto bp1 = findBp(b.list(), "bp1");
    auto bp2 = findBp(b.list(), "bp2");
    CHECK(bp1 && bp1->line == 10, "bp1 should remain before edit line");
    CHECK(bp2 && bp2->line == 23, "bp2 should shift by +3");
    PASS();
}

void test_reconcile_clamps_line_to_one() {
    TEST(reconcile_clamps_line_to_one);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 2), "add should work");
    b.reconcileOnFileEdit("a.cpp", 1, -10);
    auto bp = findBp(b.list(), "bp1");
    CHECK(bp && bp->line == 1, "line should clamp to 1");
    PASS();
}

void test_remove_breakpoint() {
    TEST(remove_breakpoint);
    BreakpointSystem b;
    CHECK(b.addLineBreakpoint("bp1", "a.cpp", 10), "add should work");
    CHECK(b.remove("bp1"), "remove should work");
    CHECK(b.list().empty(), "list should be empty after remove");
    PASS();
}

int main() {
    std::cout << "Step 555: Breakpoint System\n";

    test_add_line_breakpoint();                            // 1
    test_add_conditional_breakpoint();                     // 2
    test_add_hit_count_breakpoint();                       // 3
    test_add_invalid_hit_count_breakpoint_fails();         // 4
    test_enable_disable_persistence();                     // 5
    test_should_break_simple_enabled_breakpoint();         // 6
    test_should_not_break_when_disabled();                 // 7
    test_conditional_breakpoint_requires_true_condition(); // 8
    test_hit_count_breakpoint_triggers_at_threshold();     // 9
    test_reconcile_on_file_edit_shifts_lines();            // 10
    test_reconcile_clamps_line_to_one();                   // 11
    test_remove_breakpoint();                              // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
