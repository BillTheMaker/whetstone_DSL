// Step 561: Trace Timeline Model (12 tests)

#include "TraceTimelineModel.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_append_valid_event() {
    TEST(append_valid_event);
    TraceTimelineModel m;
    CHECK(m.append({10, "pause", "s1", "hit bp"}), "append should succeed");
    CHECK(m.all().size() == 1, "one event expected");
    PASS();
}

void test_reject_event_with_negative_timestamp() {
    TEST(reject_event_with_negative_timestamp);
    TraceTimelineModel m;
    CHECK(!m.append({-1, "pause", "s1", "bad"}), "negative ts should fail");
    PASS();
}

void test_reject_event_with_empty_type() {
    TEST(reject_event_with_empty_type);
    TraceTimelineModel m;
    CHECK(!m.append({1, "", "s1", "bad"}), "empty type should fail");
    PASS();
}

void test_reject_event_with_empty_session() {
    TEST(reject_event_with_empty_session);
    TraceTimelineModel m;
    CHECK(!m.append({1, "pause", "", "bad"}), "empty session should fail");
    PASS();
}

void test_events_are_sorted_by_timestamp() {
    TEST(events_are_sorted_by_timestamp);
    TraceTimelineModel m;
    CHECK(m.append({30, "step", "s1", "c"}), "append 30");
    CHECK(m.append({10, "pause", "s1", "a"}), "append 10");
    CHECK(m.append({20, "continue", "s1", "b"}), "append 20");
    auto all = m.all();
    CHECK(all[0].ts == 10 && all[1].ts == 20 && all[2].ts == 30, "timestamp ordering mismatch");
    PASS();
}

void test_query_by_type_filters_events() {
    TEST(query_by_type_filters_events);
    TraceTimelineModel m;
    m.append({10, "pause", "s1", "a"});
    m.append({20, "step", "s1", "b"});
    m.append({30, "pause", "s1", "c"});
    auto pauses = m.byType("pause");
    CHECK(pauses.size() == 2, "two pause events expected");
    PASS();
}

void test_query_by_session_filters_events() {
    TEST(query_by_session_filters_events);
    TraceTimelineModel m;
    m.append({10, "pause", "s1", "a"});
    m.append({20, "step", "s2", "b"});
    m.append({30, "pause", "s1", "c"});
    auto s1 = m.bySession("s1");
    CHECK(s1.size() == 2, "two s1 events expected");
    PASS();
}

void test_window_query_inclusive_bounds() {
    TEST(window_query_inclusive_bounds);
    TraceTimelineModel m;
    m.append({10, "pause", "s1", "a"});
    m.append({20, "step", "s1", "b"});
    m.append({30, "pause", "s1", "c"});
    auto w = m.window(10, 20);
    CHECK(w.size() == 2, "window should include bounds");
    PASS();
}

void test_window_query_empty_when_no_matches() {
    TEST(window_query_empty_when_no_matches);
    TraceTimelineModel m;
    m.append({10, "pause", "s1", "a"});
    auto w = m.window(100, 200);
    CHECK(w.empty(), "no matches expected");
    PASS();
}

void test_same_timestamp_orders_by_type_name() {
    TEST(same_timestamp_orders_by_type_name);
    TraceTimelineModel m;
    m.append({10, "step", "s1", "b"});
    m.append({10, "pause", "s1", "a"});
    auto all = m.all();
    CHECK(all[0].type == "pause" && all[1].type == "step", "type tiebreak ordering mismatch");
    PASS();
}

void test_detail_payload_preserved() {
    TEST(detail_payload_preserved);
    TraceTimelineModel m;
    m.append({10, "exception", "s1", "RuntimeError: boom"});
    auto all = m.all();
    CHECK(all[0].detail == "RuntimeError: boom", "detail payload mismatch");
    PASS();
}

void test_mixed_event_types_supported() {
    TEST(mixed_event_types_supported);
    TraceTimelineModel m;
    m.append({10, "pause", "s1", "a"});
    m.append({15, "step", "s1", "b"});
    m.append({20, "exception", "s1", "c"});
    m.append({25, "resume", "s1", "d"});
    m.append({30, "stop", "s1", "e"});
    CHECK(m.all().size() == 5, "five mixed events expected");
    PASS();
}

int main() {
    std::cout << "Step 561: Trace Timeline Model\n";

    test_append_valid_event();                 // 1
    test_reject_event_with_negative_timestamp();// 2
    test_reject_event_with_empty_type();       // 3
    test_reject_event_with_empty_session();    // 4
    test_events_are_sorted_by_timestamp();     // 5
    test_query_by_type_filters_events();       // 6
    test_query_by_session_filters_events();    // 7
    test_window_query_inclusive_bounds();      // 8
    test_window_query_empty_when_no_matches(); // 9
    test_same_timestamp_orders_by_type_name(); // 10
    test_detail_payload_preserved();           // 11
    test_mixed_event_types_supported();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
