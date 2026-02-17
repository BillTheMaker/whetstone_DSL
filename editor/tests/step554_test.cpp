// Step 554: Debug Session Model (12 tests)

#include "DebugSessionModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_start_creates_active_session() {
    TEST(start_creates_active_session);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.state().active, "session should be active");
    CHECK(!m.state().paused, "session should not be paused");
    PASS();
}

void test_start_fails_when_already_active() {
    TEST(start_fails_when_already_active);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "first start should succeed");
    CHECK(!m.start("t2", "buf2", "/bin/app2"), "second start should fail");
    PASS();
}

void test_attach_starts_session_when_inactive() {
    TEST(attach_starts_session_when_inactive);
    DebugSessionModel m;
    CHECK(m.attach("t1", "buf1", "/bin/app"), "attach should start inactive session");
    CHECK(m.state().active, "session should become active");
    CHECK(m.state().targets.at("t1").attached, "target should be attached");
    PASS();
}

void test_attach_adds_secondary_target() {
    TEST(attach_adds_secondary_target);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.attach("t2", "buf2", "/bin/app2"), "attach should add second target");
    CHECK(m.state().targets.size() == 2, "two targets expected");
    PASS();
}

void test_pause_transitions_to_paused() {
    TEST(pause_transitions_to_paused);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.pause(), "pause should succeed");
    CHECK(m.state().paused, "session should be paused");
    PASS();
}

void test_resume_transitions_from_paused() {
    TEST(resume_transitions_from_paused);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.pause(), "pause should succeed");
    CHECK(m.resume(), "resume should succeed");
    CHECK(!m.state().paused, "session should not be paused");
    PASS();
}

void test_pause_fails_when_inactive() {
    TEST(pause_fails_when_inactive);
    DebugSessionModel m;
    CHECK(!m.pause(), "pause should fail when inactive");
    PASS();
}

void test_resume_fails_when_not_paused() {
    TEST(resume_fails_when_not_paused);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(!m.resume(), "resume should fail when not paused");
    PASS();
}

void test_stop_clears_session_and_targets() {
    TEST(stop_clears_session_and_targets);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.attach("t2", "buf2", "/bin/app2"), "attach should succeed");
    CHECK(m.stop(), "stop should succeed");
    CHECK(!m.state().active, "session should be inactive");
    CHECK(m.state().targets.empty(), "targets should be cleared");
    PASS();
}

void test_switch_target_changes_current_target() {
    TEST(switch_target_changes_current_target);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.attach("t2", "buf2", "/bin/app2"), "attach should succeed");
    CHECK(m.switchTarget("t2"), "switch should succeed");
    CHECK(m.state().currentTargetId == "t2", "current target mismatch");
    PASS();
}

void test_switch_target_fails_for_unknown_target() {
    TEST(switch_target_fails_for_unknown_target);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(!m.switchTarget("missing"), "switch should fail for unknown target");
    PASS();
}

void test_has_target_for_buffer_maps_per_buffer_context() {
    TEST(has_target_for_buffer_maps_per_buffer_context);
    DebugSessionModel m;
    CHECK(m.start("t1", "buf1", "/bin/app"), "start should succeed");
    CHECK(m.attach("t2", "buf2", "/bin/app2"), "attach should succeed");
    CHECK(m.hasTargetForBuffer("buf1"), "buffer1 should map to target");
    CHECK(m.hasTargetForBuffer("buf2"), "buffer2 should map to target");
    CHECK(!m.hasTargetForBuffer("buf3"), "unknown buffer should not map");
    PASS();
}

int main() {
    std::cout << "Step 554: Debug Session Model\n";

    test_start_creates_active_session();                    // 1
    test_start_fails_when_already_active();                 // 2
    test_attach_starts_session_when_inactive();             // 3
    test_attach_adds_secondary_target();                    // 4
    test_pause_transitions_to_paused();                     // 5
    test_resume_transitions_from_paused();                  // 6
    test_pause_fails_when_inactive();                       // 7
    test_resume_fails_when_not_paused();                    // 8
    test_stop_clears_session_and_targets();                 // 9
    test_switch_target_changes_current_target();            // 10
    test_switch_target_fails_for_unknown_target();          // 11
    test_has_target_for_buffer_maps_per_buffer_context();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
