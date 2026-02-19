// Step 678: Sprint 43 integration summary (8 tests)

#include "Sprint43IntegrationSummary.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void t1() {
    TEST(struct_constructable);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.stepsCompleted >= 0, "not constructable");
    PASS();
}

void t2() {
    TEST(workspace_index_builds);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.workspaceIndexBuilds, "workspace index failed");
    PASS();
}

void t3() {
    TEST(slice_assembler_works);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.sliceAssemblerWorks, "slice assembler failed");
    PASS();
}

void t4() {
    TEST(budget_enforcer_works);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.budgetEnforcerWorks, "budget enforcer failed");
    PASS();
}

void t5() {
    TEST(tool_count_before_is_86);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.toolCountBefore == 86, "expected 86");
    PASS();
}

void t6() {
    TEST(tool_count_after_is_87);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.toolCountAfter == 87, "expected 87");
    PASS();
}

void t7() {
    TEST(steps_completed_is_5);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.stepsCompleted == 5, "expected 5");
    PASS();
}

void t8() {
    TEST(success_is_true);
    auto out = Sprint43IntegrationSummary::run();
    CHECK(out.success, "success false");
    PASS();
}

int main() {
    std::cout << "Step 678: Sprint 43 integration summary\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

