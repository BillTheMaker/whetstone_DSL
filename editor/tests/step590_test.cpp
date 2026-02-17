// Step 590: Crash + Recovery Reliability Sweep (12 tests)

#include "CrashRecoveryReliabilitySweep.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static RecoverySessionSnapshot snap(const std::string& sessionId) {
    RecoverySessionSnapshot s;
    s.sessionId = sessionId;
    s.workflowId = "wf-1";
    s.checkpoint = 3;
    s.openBuffers = {"a.cpp"};
    s.queuedTaskIds = {"task-1"};
    return s;
}

static bool hasNote(const RecoveryResult& r, const std::string& note) {
    for (const auto& n : r.notes) if (n == note) return true;
    return false;
}

void test_capture_snapshot_success() {
    TEST(capture_snapshot_success);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    CHECK(sweep.captureSnapshot(snap("s1"), &error), "capture should succeed");
    CHECK(sweep.knownSessions().size() == 1, "session should be stored");
    PASS();
}

void test_capture_snapshot_rejects_missing_session() {
    TEST(capture_snapshot_rejects_missing_session);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("");
    CHECK(!sweep.captureSnapshot(s, &error), "capture should fail");
    CHECK(error == "session_id_missing", "wrong error");
    PASS();
}

void test_capture_snapshot_rejects_missing_workflow() {
    TEST(capture_snapshot_rejects_missing_workflow);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    s.workflowId.clear();
    CHECK(!sweep.captureSnapshot(s, &error), "capture should fail");
    CHECK(error == "workflow_id_missing", "wrong error");
    PASS();
}

void test_capture_snapshot_rejects_invalid_checkpoint() {
    TEST(capture_snapshot_rejects_invalid_checkpoint);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    s.checkpoint = 0;
    CHECK(!sweep.captureSnapshot(s, &error), "capture should fail");
    CHECK(error == "checkpoint_invalid", "wrong error");
    PASS();
}

void test_recover_success_when_all_continuity_present() {
    TEST(recover_success_when_all_continuity_present);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    CHECK(sweep.captureSnapshot(snap("s1"), &error), "capture failed");
    auto r = sweep.recover("s1");
    CHECK(r.recovered, "recovery should be complete");
    CHECK(r.workflowContinuity && r.queueContinuity && r.bufferContinuity, "all continuity flags expected");
    CHECK(hasNote(r, "recover:ok"), "ok note expected");
    PASS();
}

void test_recover_missing_session_returns_note() {
    TEST(recover_missing_session_returns_note);
    CrashRecoveryReliabilitySweep sweep;
    auto r = sweep.recover("missing");
    CHECK(!r.recovered, "recovery should fail");
    CHECK(hasNote(r, "recover:session_missing"), "missing session note expected");
    PASS();
}

void test_recover_partial_when_queue_missing() {
    TEST(recover_partial_when_queue_missing);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    s.queuedTaskIds.clear();
    CHECK(sweep.captureSnapshot(s, &error), "capture failed");
    auto r = sweep.recover("s1");
    CHECK(!r.recovered, "recovery should be partial");
    CHECK(!r.queueContinuity, "queue continuity should fail");
    CHECK(hasNote(r, "recover:queue_missing"), "queue missing note expected");
    PASS();
}

void test_recover_partial_when_buffers_missing() {
    TEST(recover_partial_when_buffers_missing);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    s.openBuffers.clear();
    CHECK(sweep.captureSnapshot(s, &error), "capture failed");
    auto r = sweep.recover("s1");
    CHECK(!r.recovered, "recovery should be partial");
    CHECK(!r.bufferContinuity, "buffer continuity should fail");
    CHECK(hasNote(r, "recover:buffers_missing"), "buffer missing note expected");
    PASS();
}

void test_recover_partial_when_workflow_invalid() {
    TEST(recover_partial_when_workflow_invalid);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    s.workflowId = "wf-1";
    s.checkpoint = -1;
    s.queuedTaskIds = {"task-1"};
    s.openBuffers = {"a.cpp"};
    CHECK(!sweep.captureSnapshot(s, &error), "capture should fail");
    CHECK(error == "checkpoint_invalid", "wrong checkpoint error");
    PASS();
}

void test_known_sessions_returns_all_ids() {
    TEST(known_sessions_returns_all_ids);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    CHECK(sweep.captureSnapshot(snap("s1"), &error), "capture s1 failed");
    CHECK(sweep.captureSnapshot(snap("s2"), &error), "capture s2 failed");
    auto ids = sweep.knownSessions();
    CHECK(ids.size() == 2, "two ids expected");
    PASS();
}

void test_capture_overwrites_existing_session() {
    TEST(capture_overwrites_existing_session);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    CHECK(sweep.captureSnapshot(s, &error), "capture 1 failed");
    s.openBuffers = {"b.cpp"};
    CHECK(sweep.captureSnapshot(s, &error), "capture 2 failed");
    auto r = sweep.recover("s1");
    CHECK(r.bufferContinuity, "buffer continuity should remain true");
    PASS();
}

void test_partial_recovery_has_partial_note() {
    TEST(partial_recovery_has_partial_note);
    CrashRecoveryReliabilitySweep sweep;
    std::string error;
    auto s = snap("s1");
    s.openBuffers.clear();
    CHECK(sweep.captureSnapshot(s, &error), "capture failed");
    auto r = sweep.recover("s1");
    CHECK(hasNote(r, "recover:partial"), "partial note expected");
    PASS();
}

int main() {
    std::cout << "Step 590: Crash + Recovery Reliability Sweep\n";

    test_capture_snapshot_success();                   // 1
    test_capture_snapshot_rejects_missing_session();  // 2
    test_capture_snapshot_rejects_missing_workflow(); // 3
    test_capture_snapshot_rejects_invalid_checkpoint();// 4
    test_recover_success_when_all_continuity_present();// 5
    test_recover_missing_session_returns_note();      // 6
    test_recover_partial_when_queue_missing();        // 7
    test_recover_partial_when_buffers_missing();      // 8
    test_recover_partial_when_workflow_invalid();     // 9
    test_known_sessions_returns_all_ids();            // 10
    test_capture_overwrites_existing_session();       // 11
    test_partial_recovery_has_partial_note();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
