// Step 568: Phase 31a Integration (8 tests)

#include "Phase31aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasNote(const Phase31aResult& result, const std::string& note) {
    for (const auto& n : result.notes) if (n == note) return true;
    return false;
}

void test_full_cycle_passes() {
    TEST(full_cycle_passes);
    Phase31aInput input;
    const auto result = Phase31aIntegration::run(input);
    CHECK(result.phase31aPass, "phase should pass");
    CHECK(result.snapshotReady, "snapshot should be ready");
    CHECK(result.inspectorReady, "inspector should be ready");
    CHECK(result.traceReady, "trace should be ready");
    CHECK(result.signalBridgeReady, "signal bridge should be ready");
    CHECK(result.stackCorrelationReady, "stack correlation should be ready");
    CHECK(hasNote(result, "phase31a:memory_reflection_cycle_ready"), "success note missing");
    PASS();
}

void test_debug_session_start_failure_blocks_phase() {
    TEST(debug_session_start_failure_blocks_phase);
    Phase31aInput input;
    input.targetId.clear();
    const auto result = Phase31aIntegration::run(input);
    CHECK(!result.phase31aPass, "phase should fail");
    CHECK(hasNote(result, "fail:debug_session_start"), "expected debug start failure");
    PASS();
}

void test_snapshot_creation_failure_blocks_phase() {
    TEST(snapshot_creation_failure_blocks_phase);
    Phase31aInput input;
    input.snapshotId.clear();
    const auto result = Phase31aIntegration::run(input);
    CHECK(!result.phase31aPass, "phase should fail");
    CHECK(hasNote(result, "fail:snapshot_create"), "expected snapshot creation failure");
    PASS();
}

void test_invalid_pointer_text_blocks_inspector_resolution() {
    TEST(invalid_pointer_text_blocks_inspector_resolution);
    Phase31aInput input;
    input.pointerText = "invalid-pointer";
    const auto result = Phase31aIntegration::run(input);
    CHECK(!result.phase31aPass, "phase should fail");
    CHECK(hasNote(result, "fail:inspector_pointer_resolve"), "expected pointer resolve failure");
    PASS();
}

void test_invalid_transfer_owner_blocks_trace() {
    TEST(invalid_transfer_owner_blocks_trace);
    Phase31aInput input;
    input.transferredOwner.clear();
    const auto result = Phase31aIntegration::run(input);
    CHECK(!result.phase31aPass, "phase should fail");
    CHECK(hasNote(result, "fail:allocation_trace"), "expected allocation trace failure");
    PASS();
}

void test_signal_bridge_failure_blocks_phase() {
    TEST(signal_bridge_failure_blocks_phase);
    Phase31aInput input;
    input.omitSignalMessage = true;
    const auto result = Phase31aIntegration::run(input);
    CHECK(!result.phase31aPass, "phase should fail");
    CHECK(hasNote(result, "fail:signal_bridge_ingest"), "expected signal bridge failure");
    PASS();
}

void test_invalid_stack_frame_blocks_correlation() {
    TEST(invalid_stack_frame_blocks_correlation);
    Phase31aInput input;
    input.frameLine = 0;
    const auto result = Phase31aIntegration::run(input);
    CHECK(!result.phase31aPass, "phase should fail");
    CHECK(hasNote(result, "fail:stack_correlation"), "expected stack correlation failure");
    PASS();
}

void test_corruption_signal_raises_max_severity() {
    TEST(corruption_signal_raises_max_severity);
    Phase31aInput input;
    input.emitCorruptionSignal = true;
    const auto result = Phase31aIntegration::run(input);
    CHECK(result.phase31aPass, "phase should pass");
    CHECK(result.maxSeverity == MemorySignalSeverity::Critical, "max severity should be critical");
    PASS();
}

int main() {
    std::cout << "Step 568: Phase 31a Integration\n";

    test_full_cycle_passes();                              // 1
    test_debug_session_start_failure_blocks_phase();       // 2
    test_snapshot_creation_failure_blocks_phase();         // 3
    test_invalid_pointer_text_blocks_inspector_resolution();// 4
    test_invalid_transfer_owner_blocks_trace();            // 5
    test_signal_bridge_failure_blocks_phase();             // 6
    test_invalid_stack_frame_blocks_correlation();         // 7
    test_corruption_signal_raises_max_severity();          // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
