// Step 558: Phase 30a Integration (8 tests)

#include "Phase30aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasError(const Phase30aResult& r, const std::string& e) {
    for (const auto& x : r.errors) if (x == e) return true;
    return false;
}

static Phase30aRequest baseReq() {
    return {"target-1", "main.cpp", "/bin/app", "bp-1", 20, true, "step_over", false};
}

void test_full_cycle_without_exception_passes() {
    TEST(full_cycle_without_exception_passes);
    auto r = Phase30aIntegration::run(baseReq());
    CHECK(r.pass, "baseline phase30a cycle should pass");
    CHECK(r.breakpointHit, "breakpoint should be hit");
    PASS();
}

void test_full_cycle_with_exception_passes() {
    TEST(full_cycle_with_exception_passes);
    auto req = baseReq();
    req.triggerException = true;
    auto r = Phase30aIntegration::run(req);
    CHECK(r.pass, "exception cycle should pass");
    CHECK(r.exceptionResult.valid, "exception capture should be valid");
    PASS();
}

void test_invalid_breakpoint_line_fails() {
    TEST(invalid_breakpoint_line_fails);
    auto req = baseReq();
    req.breakpointLine = 0;
    auto r = Phase30aIntegration::run(req);
    CHECK(!r.pass, "invalid breakpoint line should fail");
    CHECK(hasError(r, "breakpoint_add_failed"), "breakpoint add error expected");
    PASS();
}

void test_condition_false_prevents_breakpoint_hit() {
    TEST(condition_false_prevents_breakpoint_hit);
    auto req = baseReq();
    req.triggerCondition = false;
    auto r = Phase30aIntegration::run(req);
    CHECK(!r.pass, "condition false should fail cycle");
    CHECK(hasError(r, "breakpoint_not_hit"), "breakpoint not hit error expected");
    PASS();
}

void test_unknown_step_command_fails_cycle() {
    TEST(unknown_step_command_fails_cycle);
    auto req = baseReq();
    req.stepCommand = "warp";
    auto r = Phase30aIntegration::run(req);
    CHECK(!r.pass, "unknown step command should fail");
    CHECK(hasError(r, "step_failed"), "step failure expected");
    PASS();
}

void test_session_start_failure_when_target_invalid() {
    TEST(session_start_failure_when_target_invalid);
    auto req = baseReq();
    req.targetId = "";
    auto r = Phase30aIntegration::run(req);
    CHECK(!r.pass, "invalid session start should fail");
    CHECK(hasError(r, "session_start_failed"), "session start failure expected");
    PASS();
}

void test_step_result_exposed_for_ui_sync() {
    TEST(step_result_exposed_for_ui_sync);
    auto req = baseReq();
    req.stepCommand = "continue";
    auto r = Phase30aIntegration::run(req);
    CHECK(r.pass, "continue flow should pass");
    CHECK(r.stepResult.ok, "step result should be ok");
    CHECK(r.stepResult.state.lastCommand == "continue", "last command should propagate");
    PASS();
}

void test_exception_frame_anchor_uses_buffer_path() {
    TEST(exception_frame_anchor_uses_buffer_path);
    auto req = baseReq();
    req.triggerException = true;
    auto r = Phase30aIntegration::run(req);
    CHECK(r.pass, "exception flow should pass");
    CHECK(!r.exceptionResult.event.frames.empty(), "frames expected");
    CHECK(r.exceptionResult.event.frames[0].navigationAnchor.find("main.cpp:") == 0,
          "anchor should use buffer path");
    PASS();
}

int main() {
    std::cout << "Step 558: Phase 30a Integration\n";

    test_full_cycle_without_exception_passes();      // 1
    test_full_cycle_with_exception_passes();         // 2
    test_invalid_breakpoint_line_fails();            // 3
    test_condition_false_prevents_breakpoint_hit();  // 4
    test_unknown_step_command_fails_cycle();         // 5
    test_session_start_failure_when_target_invalid();// 6
    test_step_result_exposed_for_ui_sync();          // 7
    test_exception_frame_anchor_uses_buffer_path();  // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
