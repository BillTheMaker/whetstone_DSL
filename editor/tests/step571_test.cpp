// Step 571: Time-Travel Debug Event Buffer (12 tests)

#include "TimeTravelDebugEventBuffer.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static DebugReplayEvent event(const std::string& id,
                              const std::string& session,
                              const std::string& type,
                              std::uint64_t ip,
                              std::uint64_t ts) {
    DebugReplayEvent e;
    e.eventId = id;
    e.sessionId = session;
    e.type = type;
    e.instructionPointer = ip;
    e.timestamp = ts;
    e.payload = "payload-" + id;
    return e;
}

void test_append_success() {
    TEST(append_success);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append should succeed");
    CHECK(buffer.size() == 1, "size mismatch");
    PASS();
}

void test_append_rejects_missing_ids() {
    TEST(append_rejects_missing_ids);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(!buffer.append(event("", "sess-1", "step", 0x1000, 10), &error), "append should fail");
    CHECK(error == "event_or_session_missing", "wrong error");
    PASS();
}

void test_append_rejects_missing_type() {
    TEST(append_rejects_missing_type);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(!buffer.append(event("e1", "sess-1", "", 0x1000, 10), &error), "append should fail");
    CHECK(error == "event_type_missing", "wrong error");
    PASS();
}

void test_append_rejects_invalid_timestamp() {
    TEST(append_rejects_invalid_timestamp);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(!buffer.append(event("e1", "sess-1", "step", 0x1000, 0), &error), "append should fail");
    CHECK(error == "event_timestamp_invalid", "wrong error");
    PASS();
}

void test_append_rejects_invalid_ip() {
    TEST(append_rejects_invalid_ip);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(!buffer.append(event("e1", "sess-1", "step", 0, 10), &error), "append should fail");
    CHECK(error == "event_ip_invalid", "wrong error");
    PASS();
}

void test_append_rejects_duplicate_event_id() {
    TEST(append_rejects_duplicate_event_id);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "first append failed");
    CHECK(!buffer.append(event("e1", "sess-1", "step", 0x1001, 11), &error), "duplicate append should fail");
    CHECK(error == "event_duplicate", "wrong error");
    PASS();
}

void test_capacity_eviction_tracks_dropped_count() {
    TEST(capacity_eviction_tracks_dropped_count);
    TimeTravelDebugEventBuffer buffer(2);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append e1 failed");
    CHECK(buffer.append(event("e2", "sess-1", "step", 0x1001, 11), &error), "append e2 failed");
    CHECK(buffer.append(event("e3", "sess-1", "step", 0x1002, 12), &error), "append e3 failed");
    CHECK(buffer.size() == 2, "size should cap at capacity");
    CHECK(buffer.droppedCount() == 1, "dropped count mismatch");
    PASS();
}

void test_latest_returns_most_recent_events() {
    TEST(latest_returns_most_recent_events);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append e1 failed");
    CHECK(buffer.append(event("e2", "sess-1", "step", 0x1001, 11), &error), "append e2 failed");
    CHECK(buffer.append(event("e3", "sess-1", "step", 0x1002, 12), &error), "append e3 failed");
    const auto latest = buffer.latest(2);
    CHECK(latest.size() == 2, "latest count mismatch");
    CHECK(latest[0].eventId == "e2", "latest[0] mismatch");
    CHECK(latest[1].eventId == "e3", "latest[1] mismatch");
    PASS();
}

void test_for_session_filters_events() {
    TEST(for_session_filters_events);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append e1 failed");
    CHECK(buffer.append(event("e2", "sess-2", "step", 0x1001, 11), &error), "append e2 failed");
    const auto filtered = buffer.forSession("sess-2");
    CHECK(filtered.size() == 1, "filter count mismatch");
    CHECK(filtered[0].eventId == "e2", "filter event mismatch");
    PASS();
}

void test_replay_window_returns_slice() {
    TEST(replay_window_returns_slice);
    TimeTravelDebugEventBuffer buffer(8);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append e1 failed");
    CHECK(buffer.append(event("e2", "sess-1", "step", 0x1001, 11), &error), "append e2 failed");
    CHECK(buffer.append(event("e3", "sess-1", "step", 0x1002, 12), &error), "append e3 failed");
    const auto window = buffer.replayWindow(1, 2);
    CHECK(window.size() == 2, "replay window size mismatch");
    CHECK(window[0].eventId == "e2", "window[0] mismatch");
    CHECK(window[1].eventId == "e3", "window[1] mismatch");
    PASS();
}

void test_replay_window_out_of_range_is_empty() {
    TEST(replay_window_out_of_range_is_empty);
    TimeTravelDebugEventBuffer buffer(4);
    std::string error;
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append failed");
    const auto window = buffer.replayWindow(5, 2);
    CHECK(window.empty(), "window should be empty");
    PASS();
}

void test_zero_capacity_defaults_to_one() {
    TEST(zero_capacity_defaults_to_one);
    TimeTravelDebugEventBuffer buffer(0);
    std::string error;
    CHECK(buffer.capacity() == 1, "capacity should default to one");
    CHECK(buffer.append(event("e1", "sess-1", "step", 0x1000, 10), &error), "append e1 failed");
    CHECK(buffer.append(event("e2", "sess-1", "step", 0x1001, 11), &error), "append e2 failed");
    CHECK(buffer.size() == 1, "size should remain one");
    PASS();
}

int main() {
    std::cout << "Step 571: Time-Travel Debug Event Buffer\n";

    test_append_success();                         // 1
    test_append_rejects_missing_ids();            // 2
    test_append_rejects_missing_type();           // 3
    test_append_rejects_invalid_timestamp();      // 4
    test_append_rejects_invalid_ip();             // 5
    test_append_rejects_duplicate_event_id();     // 6
    test_capacity_eviction_tracks_dropped_count();// 7
    test_latest_returns_most_recent_events();     // 8
    test_for_session_filters_events();            // 9
    test_replay_window_returns_slice();           // 10
    test_replay_window_out_of_range_is_empty();   // 11
    test_zero_capacity_defaults_to_one();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
