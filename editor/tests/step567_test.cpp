// Step 567: Leak/Corruption Signal Bridge (12 tests)

#include "LeakCorruptionSignalBridge.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static MemorySignalRecord signal(const std::string& id,
                                 const std::string& session,
                                 const std::string& allocation,
                                 MemorySignalType type,
                                 std::uint64_t timestamp) {
    MemorySignalRecord s;
    s.signalId = id;
    s.sessionId = session;
    s.allocationId = allocation;
    s.type = type;
    s.message = "message-" + id;
    s.timestamp = timestamp;
    return s;
}

void test_ingest_leak_suspected_signal_success() {
    TEST(ingest_leak_suspected_signal_success);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    CHECK(bridge.ingestSignal(signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 5), &error), "ingest should succeed");
    const auto diagnostics = bridge.diagnosticsForSession("sess-1");
    CHECK(diagnostics.size() == 1, "expected one diagnostic");
    CHECK(diagnostics[0].paneKey == "memory-leaks", "leak pane expected");
    CHECK(diagnostics[0].severity == MemorySignalSeverity::Warning, "warning severity expected");
    PASS();
}

void test_ingest_corruption_confirmed_maps_to_critical() {
    TEST(ingest_corruption_confirmed_maps_to_critical);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    CHECK(bridge.ingestSignal(signal("s1", "sess-1", "alloc-1", MemorySignalType::CorruptionConfirmed, 5), &error), "ingest should succeed");
    const auto diagnostics = bridge.diagnosticsForSession("sess-1");
    CHECK(diagnostics[0].paneKey == "memory-corruption", "corruption pane expected");
    CHECK(diagnostics[0].severity == MemorySignalSeverity::Critical, "critical severity expected");
    PASS();
}

void test_ingest_rejects_missing_signal_or_session_id() {
    TEST(ingest_rejects_missing_signal_or_session_id);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    auto s = signal("", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 5);
    CHECK(!bridge.ingestSignal(s, &error), "ingest should fail");
    CHECK(error == "signal_or_session_missing", "wrong error");
    PASS();
}

void test_ingest_rejects_missing_allocation_id() {
    TEST(ingest_rejects_missing_allocation_id);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    auto s = signal("s1", "sess-1", "", MemorySignalType::LeakSuspected, 5);
    CHECK(!bridge.ingestSignal(s, &error), "ingest should fail");
    CHECK(error == "allocation_id_missing", "wrong error");
    PASS();
}

void test_ingest_rejects_missing_message() {
    TEST(ingest_rejects_missing_message);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    auto s = signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 5);
    s.message = "";
    CHECK(!bridge.ingestSignal(s, &error), "ingest should fail");
    CHECK(error == "signal_message_missing", "wrong error");
    PASS();
}

void test_ingest_rejects_zero_timestamp() {
    TEST(ingest_rejects_zero_timestamp);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    auto s = signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 0);
    CHECK(!bridge.ingestSignal(s, &error), "ingest should fail");
    CHECK(error == "signal_timestamp_invalid", "wrong error");
    PASS();
}

void test_ingest_rejects_duplicate_signal_id() {
    TEST(ingest_rejects_duplicate_signal_id);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    auto s = signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 5);
    CHECK(bridge.ingestSignal(s, &error), "first ingest should succeed");
    CHECK(!bridge.ingestSignal(s, &error), "duplicate ingest should fail");
    CHECK(error == "signal_duplicate", "wrong error");
    PASS();
}

void test_diagnostics_for_session_filters_results() {
    TEST(diagnostics_for_session_filters_results);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    CHECK(bridge.ingestSignal(signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakConfirmed, 5), &error), "ingest s1 failed");
    CHECK(bridge.ingestSignal(signal("s2", "sess-2", "alloc-2", MemorySignalType::LeakConfirmed, 6), &error), "ingest s2 failed");
    const auto diagnostics = bridge.diagnosticsForSession("sess-2");
    CHECK(diagnostics.size() == 1, "expected one filtered diagnostic");
    CHECK(diagnostics[0].sessionId == "sess-2", "wrong session");
    PASS();
}

void test_signals_for_allocation_filters_results() {
    TEST(signals_for_allocation_filters_results);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    CHECK(bridge.ingestSignal(signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakConfirmed, 5), &error), "ingest s1 failed");
    CHECK(bridge.ingestSignal(signal("s2", "sess-1", "alloc-2", MemorySignalType::CorruptionSuspected, 6), &error), "ingest s2 failed");
    const auto signals = bridge.signalsForAllocation("alloc-2");
    CHECK(signals.size() == 1, "expected one filtered signal");
    CHECK(signals[0].signalId == "s2", "wrong signal");
    PASS();
}

void test_highest_severity_for_session_aggregates() {
    TEST(highest_severity_for_session_aggregates);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    CHECK(bridge.ingestSignal(signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 5), &error), "ingest s1 failed");
    CHECK(bridge.ingestSignal(signal("s2", "sess-1", "alloc-1", MemorySignalType::LeakConfirmed, 6), &error), "ingest s2 failed");
    CHECK(bridge.ingestSignal(signal("s3", "sess-1", "alloc-1", MemorySignalType::CorruptionConfirmed, 7), &error), "ingest s3 failed");
    CHECK(bridge.highestSeverityForSession("sess-1") == MemorySignalSeverity::Critical, "critical severity expected");
    PASS();
}

void test_highest_severity_defaults_to_info_when_empty() {
    TEST(highest_severity_defaults_to_info_when_empty);
    LeakCorruptionSignalBridge bridge;
    CHECK(bridge.highestSeverityForSession("sess-1") == MemorySignalSeverity::Info, "default info expected");
    PASS();
}

void test_diagnostics_sorted_by_timestamp() {
    TEST(diagnostics_sorted_by_timestamp);
    LeakCorruptionSignalBridge bridge;
    std::string error;
    CHECK(bridge.ingestSignal(signal("s2", "sess-1", "alloc-1", MemorySignalType::LeakConfirmed, 20), &error), "ingest s2 failed");
    CHECK(bridge.ingestSignal(signal("s1", "sess-1", "alloc-1", MemorySignalType::LeakSuspected, 10), &error), "ingest s1 failed");
    const auto diagnostics = bridge.diagnosticsForSession("sess-1");
    CHECK(diagnostics.size() == 2, "diagnostic count mismatch");
    CHECK(diagnostics[0].diagnosticId == "diag-s1", "first diagnostic should be earliest");
    CHECK(diagnostics[1].diagnosticId == "diag-s2", "second diagnostic should be later");
    PASS();
}

int main() {
    std::cout << "Step 567: Leak/Corruption Signal Bridge\n";

    test_ingest_leak_suspected_signal_success();             // 1
    test_ingest_corruption_confirmed_maps_to_critical();     // 2
    test_ingest_rejects_missing_signal_or_session_id();      // 3
    test_ingest_rejects_missing_allocation_id();             // 4
    test_ingest_rejects_missing_message();                   // 5
    test_ingest_rejects_zero_timestamp();                    // 6
    test_ingest_rejects_duplicate_signal_id();               // 7
    test_diagnostics_for_session_filters_results();          // 8
    test_signals_for_allocation_filters_results();           // 9
    test_highest_severity_for_session_aggregates();          // 10
    test_highest_severity_defaults_to_info_when_empty();     // 11
    test_diagnostics_sorted_by_timestamp();                  // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
