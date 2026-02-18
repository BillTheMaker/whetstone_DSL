// Step 623: Sprint 36 integration + summary (8 tests)

#include "Sprint36IntegrationSummary.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string clearSpec() {
    return
        "## Goals\n"
        "- Build architect intake pipeline\n"
        "\n"
        "## Constraints\n"
        "- Keep tool registration deterministic\n"
        "\n"
        "## Dependencies\n"
        "- cmake\n"
        "\n"
        "## Acceptance Criteria\n"
        "- queue readiness can be computed\n";
}

static std::string riskySpec() {
    return
        "## Goals\n"
        "- maybe improve pipeline maybe maybe\n"
        "\n"
        "## Constraints\n"
        "- Do not use dynamic allocation in hot path\n"
        "- Use dynamic allocation in hot path for warmup\n"
        "\n"
        "## Dependencies\n"
        "- maybe add dependency\n"
        "\n"
        "## Acceptance Criteria\n"
        "- maybe pass tests\n";
}

static bool hasBlocker(const Sprint36IntegrationResult& r, const std::string& blocker) {
    for (const auto& b : r.blockers) if (b == blocker) return true;
    return false;
}

void test_initialize_instructions_available() {
    TEST(initialize_instructions_available);
    auto result = Sprint36IntegrationSummary::run(clearSpec());
    CHECK(result.initializeOk, "initialize instructions should be available");
    PASS();
}

void test_intake_stage_succeeds_for_clear_spec() {
    TEST(intake_stage_succeeds_for_clear_spec);
    auto result = Sprint36IntegrationSummary::run(clearSpec());
    CHECK(result.intakeOk, "intake should succeed");
    PASS();
}

void test_generate_stage_succeeds_for_clear_spec() {
    TEST(generate_stage_succeeds_for_clear_spec);
    auto result = Sprint36IntegrationSummary::run(clearSpec());
    CHECK(result.generateOk, "generate should succeed");
    PASS();
}

void test_queue_stage_succeeds_for_clear_spec() {
    TEST(queue_stage_succeeds_for_clear_spec);
    auto result = Sprint36IntegrationSummary::run(clearSpec());
    CHECK(result.queueOk, "queue should succeed");
    PASS();
}

void test_clear_spec_is_queue_ready() {
    TEST(clear_spec_is_queue_ready);
    auto result = Sprint36IntegrationSummary::run(clearSpec());
    CHECK(result.queueReady, "clear spec should be queue ready");
    CHECK(result.readyCount >= 1, "readyCount should be >= 1");
    PASS();
}

void test_summary_exposes_task_count() {
    TEST(summary_exposes_task_count);
    auto result = Sprint36IntegrationSummary::run(clearSpec());
    CHECK(result.taskCount >= 1, "taskCount should be >= 1");
    PASS();
}

void test_risky_spec_not_queue_ready() {
    TEST(risky_spec_not_queue_ready);
    auto result = Sprint36IntegrationSummary::run(riskySpec());
    CHECK(result.intakeOk && result.generateOk && result.queueOk, "all stages should execute");
    CHECK(!result.queueReady, "risky spec should not be queue ready");
    CHECK(result.escalateCount >= 1, "escalateCount should be >= 1");
    PASS();
}

void test_risky_spec_has_escalation_blocker() {
    TEST(risky_spec_has_escalation_blocker);
    auto result = Sprint36IntegrationSummary::run(riskySpec());
    CHECK(hasBlocker(result, "escalations_present"), "expected escalations_present blocker");
    PASS();
}

int main() {
    std::cout << "Step 623: Sprint 36 integration + summary\n";

    test_initialize_instructions_available();          // 1
    test_intake_stage_succeeds_for_clear_spec();      // 2
    test_generate_stage_succeeds_for_clear_spec();    // 3
    test_queue_stage_succeeds_for_clear_spec();       // 4
    test_clear_spec_is_queue_ready();                 // 5
    test_summary_exposes_task_count();                // 6
    test_risky_spec_not_queue_ready();                // 7
    test_risky_spec_has_escalation_blocker();         // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
