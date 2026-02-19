// Step 688: Sprint 45 integration summary (8 tests)

#include "Sprint45IntegrationSummary.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void t1() {
    TEST(struct_constructable);
    auto r = Sprint45IntegrationSummary::run();
    CHECK(r.stepsCompleted >= 0, "not constructable");
    PASS();
}

void t2() {
    TEST(recorder_works);
    auto r = Sprint45IntegrationSummary::run();
    CHECK(r.recorderWorks, "recorder false");
    PASS();
}

void t3() {
    TEST(metrics_works);
    auto r = Sprint45IntegrationSummary::run();
    CHECK(r.metricsWorks, "metrics false");
    PASS();
}

void t4() {
    TEST(comparison_works);
    auto r = Sprint45IntegrationSummary::run();
    CHECK(r.comparisonWorks, "comparison false");
    PASS();
}

void t5() {
    TEST(steps_completed_is_5);
    auto r = Sprint45IntegrationSummary::run();
    CHECK(r.stepsCompleted == 5, "expected 5");
    PASS();
}

void t6() {
    TEST(files_added_contains_recorder);
    auto r = Sprint45IntegrationSummary::run();
    bool found = false;
    for (const auto& f : r.filesAdded) {
        if (f.find("AgentSessionRecorder.h") != std::string::npos) { found = true; break; }
    }
    CHECK(found, "recorder file missing");
    PASS();
}

void t7() {
    TEST(files_added_contains_ab_comparison);
    auto r = Sprint45IntegrationSummary::run();
    bool found = false;
    for (const auto& f : r.filesAdded) {
        if (f.find("ABTestComparison.h") != std::string::npos) { found = true; break; }
    }
    CHECK(found, "AB comparison file missing");
    PASS();
}

void t8() {
    TEST(success_true);
    auto r = Sprint45IntegrationSummary::run();
    CHECK(r.success, "success false");
    PASS();
}

int main() {
    std::cout << "Step 688: Sprint 45 integration summary\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

