// Step 572: Performance Probe Overlay (12 tests)

#include "PerformanceProbeOverlay.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ProbeSample sample(const std::string& id,
                          const std::string& session,
                          const std::string& buffer,
                          int line,
                          std::uint64_t duration,
                          std::uint64_t ts) {
    ProbeSample s;
    s.sampleId = id;
    s.sessionId = session;
    s.bufferId = buffer;
    s.line = line;
    s.durationMicros = duration;
    s.timestamp = ts;
    return s;
}

void test_add_sample_success() {
    TEST(add_sample_success);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 500, 1), &error), "sample should add");
    PASS();
}

void test_add_sample_rejects_missing_ids() {
    TEST(add_sample_rejects_missing_ids);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(!overlay.addSample(sample("", "sess-1", "buf-1", 10, 500, 1), &error), "sample should fail");
    CHECK(error == "sample_or_session_missing", "wrong error");
    PASS();
}

void test_add_sample_rejects_invalid_line() {
    TEST(add_sample_rejects_invalid_line);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(!overlay.addSample(sample("s1", "sess-1", "buf-1", 0, 500, 1), &error), "sample should fail");
    CHECK(error == "sample_line_invalid", "wrong error");
    PASS();
}

void test_add_sample_rejects_invalid_duration() {
    TEST(add_sample_rejects_invalid_duration);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(!overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 0, 1), &error), "sample should fail");
    CHECK(error == "sample_duration_invalid", "wrong error");
    PASS();
}

void test_add_sample_rejects_duplicate_id() {
    TEST(add_sample_rejects_duplicate_id);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 500, 1), &error), "first sample failed");
    CHECK(!overlay.addSample(sample("s1", "sess-1", "buf-1", 11, 400, 2), &error), "duplicate should fail");
    CHECK(error == "sample_duplicate", "wrong error");
    PASS();
}

void test_aggregates_group_by_line() {
    TEST(aggregates_group_by_line);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 100, 1), &error), "s1 failed");
    CHECK(overlay.addSample(sample("s2", "sess-1", "buf-1", 10, 300, 2), &error), "s2 failed");
    CHECK(overlay.addSample(sample("s3", "sess-1", "buf-1", 20, 500, 3), &error), "s3 failed");
    const auto aggs = overlay.aggregatesFor("sess-1", "buf-1");
    CHECK(aggs.size() == 2, "aggregate count mismatch");
    CHECK(aggs[0].line == 10 && aggs[0].totalDurationMicros == 400, "line 10 aggregate mismatch");
    CHECK(aggs[1].line == 20 && aggs[1].totalDurationMicros == 500, "line 20 aggregate mismatch");
    PASS();
}

void test_aggregates_filter_by_session_and_buffer() {
    TEST(aggregates_filter_by_session_and_buffer);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 100, 1), &error), "s1 failed");
    CHECK(overlay.addSample(sample("s2", "sess-2", "buf-1", 10, 300, 2), &error), "s2 failed");
    CHECK(overlay.addSample(sample("s3", "sess-1", "buf-2", 20, 500, 3), &error), "s3 failed");
    const auto aggs = overlay.aggregatesFor("sess-1", "buf-1");
    CHECK(aggs.size() == 1, "filtered aggregate count mismatch");
    CHECK(aggs[0].line == 10, "line mismatch");
    PASS();
}

void test_top_hotspots_order_by_total_duration() {
    TEST(top_hotspots_order_by_total_duration);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 100, 1), &error), "s1 failed");
    CHECK(overlay.addSample(sample("s2", "sess-1", "buf-1", 20, 600, 2), &error), "s2 failed");
    CHECK(overlay.addSample(sample("s3", "sess-1", "buf-1", 30, 300, 3), &error), "s3 failed");
    const auto top = overlay.topHotspots("sess-1", "buf-1", 2, 0);
    CHECK(top.size() == 2, "top hotspot count mismatch");
    CHECK(top[0].line == 20, "highest hotspot should be line 20");
    CHECK(top[1].line == 30, "second hotspot should be line 30");
    PASS();
}

void test_top_hotspots_applies_threshold() {
    TEST(top_hotspots_applies_threshold);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 10, 100, 1), &error), "s1 failed");
    CHECK(overlay.addSample(sample("s2", "sess-1", "buf-1", 20, 600, 2), &error), "s2 failed");
    const auto top = overlay.topHotspots("sess-1", "buf-1", 5, 500);
    CHECK(top.size() == 1, "threshold should filter to one hotspot");
    CHECK(top[0].line == 20, "threshold hotspot mismatch");
    PASS();
}

void test_overlay_window_returns_pause_neighborhood() {
    TEST(overlay_window_returns_pause_neighborhood);
    PerformanceProbeOverlay overlay;
    std::string error;
    CHECK(overlay.addSample(sample("s1", "sess-1", "buf-1", 8, 100, 1), &error), "s1 failed");
    CHECK(overlay.addSample(sample("s2", "sess-1", "buf-1", 10, 200, 2), &error), "s2 failed");
    CHECK(overlay.addSample(sample("s3", "sess-1", "buf-1", 13, 300, 3), &error), "s3 failed");
    const auto window = overlay.overlayWindow("sess-1", "buf-1", 10, 2);
    CHECK(window.size() == 2, "window count mismatch");
    CHECK(window[0].line == 8, "window first line mismatch");
    CHECK(window[1].line == 10, "window second line mismatch");
    PASS();
}

void test_overlay_window_invalid_pause_line_is_empty() {
    TEST(overlay_window_invalid_pause_line_is_empty);
    PerformanceProbeOverlay overlay;
    const auto window = overlay.overlayWindow("sess-1", "buf-1", 0, 2);
    CHECK(window.empty(), "window should be empty");
    PASS();
}

void test_overlay_window_negative_radius_is_empty() {
    TEST(overlay_window_negative_radius_is_empty);
    PerformanceProbeOverlay overlay;
    const auto window = overlay.overlayWindow("sess-1", "buf-1", 10, -1);
    CHECK(window.empty(), "window should be empty");
    PASS();
}

int main() {
    std::cout << "Step 572: Performance Probe Overlay\n";

    test_add_sample_success();                          // 1
    test_add_sample_rejects_missing_ids();             // 2
    test_add_sample_rejects_invalid_line();            // 3
    test_add_sample_rejects_invalid_duration();        // 4
    test_add_sample_rejects_duplicate_id();            // 5
    test_aggregates_group_by_line();                   // 6
    test_aggregates_filter_by_session_and_buffer();    // 7
    test_top_hotspots_order_by_total_duration();       // 8
    test_top_hotspots_applies_threshold();             // 9
    test_overlay_window_returns_pause_neighborhood();  // 10
    test_overlay_window_invalid_pause_line_is_empty(); // 11
    test_overlay_window_negative_radius_is_empty();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
