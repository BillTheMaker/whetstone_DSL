// Step 618: Sprint 36a Integration (8 tests)

#include "Sprint36aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string clearSpec() {
    return
        "## Goals\n"
        "- Ship architect intake MCP tools\n"
        "\n"
        "## Constraints\n"
        "- Keep code header only\n"
        "\n"
        "## Dependencies\n"
        "- nlohmann json\n"
        "\n"
        "## Acceptance Criteria\n"
        "- queue includes acceptance checks\n";
}

static std::string riskySpec() {
    return
        "## Goals\n"
        "- maybe improve intake maybe maybe\n"
        "\n"
        "## Constraints\n"
        "- Do not use dynamic allocation in hot path\n"
        "- Use dynamic allocation in hot path for warmup\n"
        "\n"
        "## Dependencies\n"
        "- maybe add extra libs\n"
        "\n"
        "## Acceptance Criteria\n"
        "- maybe all tests pass\n";
}

static bool hasBlocker(const Sprint36aIntegrationResult& r, const std::string& blocker) {
    for (const auto& b : r.blockers) if (b == blocker) return true;
    return false;
}

void test_clear_spec_reaches_done_stage() {
    TEST(clear_spec_reaches_done_stage);
    auto result = Sprint36aIntegration::run(clearSpec());
    CHECK(result.stage == "done", "expected done stage");
    PASS();
}

void test_clear_spec_integration_success() {
    TEST(clear_spec_integration_success);
    auto result = Sprint36aIntegration::run(clearSpec());
    CHECK(result.success, "clear spec should be queue ready");
    PASS();
}

void test_clear_spec_produces_tasks() {
    TEST(clear_spec_produces_tasks);
    auto result = Sprint36aIntegration::run(clearSpec());
    CHECK(result.taskCount >= 1, "taskCount should be >= 1");
    PASS();
}

void test_clear_spec_has_ready_items() {
    TEST(clear_spec_has_ready_items);
    auto result = Sprint36aIntegration::run(clearSpec());
    CHECK(result.readyCount >= 1, "readyCount should be >= 1");
    PASS();
}

void test_clear_spec_has_no_escalations() {
    TEST(clear_spec_has_no_escalations);
    auto result = Sprint36aIntegration::run(clearSpec());
    CHECK(result.escalateCount == 0, "escalateCount should be zero");
    PASS();
}

void test_empty_markdown_fails_at_intake_stage() {
    TEST(empty_markdown_fails_at_intake_stage);
    auto result = Sprint36aIntegration::run("");
    CHECK(!result.success, "empty markdown should fail");
    CHECK(result.stage == "intake", "should fail at intake");
    PASS();
}

void test_risky_spec_not_queue_ready() {
    TEST(risky_spec_not_queue_ready);
    auto result = Sprint36aIntegration::run(riskySpec());
    CHECK(!result.success, "risky spec should not be queue ready");
    CHECK(result.escalateCount >= 1, "should produce escalations");
    PASS();
}

void test_risky_spec_has_escalation_blocker() {
    TEST(risky_spec_has_escalation_blocker);
    auto result = Sprint36aIntegration::run(riskySpec());
    CHECK(hasBlocker(result, "escalations_present"), "expected escalations_present blocker");
    PASS();
}

int main() {
    std::cout << "Step 618: Sprint 36a Integration\n";

    test_clear_spec_reaches_done_stage();             // 1
    test_clear_spec_integration_success();            // 2
    test_clear_spec_produces_tasks();                 // 3
    test_clear_spec_has_ready_items();                // 4
    test_clear_spec_has_no_escalations();             // 5
    test_empty_markdown_fails_at_intake_stage();      // 6
    test_risky_spec_not_queue_ready();                // 7
    test_risky_spec_has_escalation_blocker();         // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
