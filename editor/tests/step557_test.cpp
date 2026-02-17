// Step 557: Exception + Stack Trace Capture (12 tests)

#include "ExceptionStackTraceCapture.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasError(const ExceptionCaptureResult& r, const std::string& code) {
    for (const auto& e : r.errors) {
        if (e == code || e.find(code) == 0) return true;
    }
    return false;
}

void test_valid_exception_capture_passes() {
    TEST(valid_exception_capture_passes);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1",
        { {"foo", "main.cpp", 12, ""}, {"bar", "lib.cpp", 44, ""} });
    CHECK(r.valid, "valid exception should pass");
    CHECK(r.event.frames.size() == 2, "two frames expected");
    PASS();
}

void test_missing_type_rejected() {
    TEST(missing_type_rejected);
    auto r = ExceptionStackTraceCapture::capture(
        "", "boom", "thread-1", {{"foo", "main.cpp", 12, ""}});
    CHECK(!r.valid, "missing type should fail");
    CHECK(hasError(r, "missing_type"), "missing_type expected");
    PASS();
}

void test_missing_message_rejected() {
    TEST(missing_message_rejected);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "", "thread-1", {{"foo", "main.cpp", 12, ""}});
    CHECK(!r.valid, "missing message should fail");
    CHECK(hasError(r, "missing_message"), "missing_message expected");
    PASS();
}

void test_missing_thread_id_rejected() {
    TEST(missing_thread_id_rejected);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "", {{"foo", "main.cpp", 12, ""}});
    CHECK(!r.valid, "missing thread id should fail");
    CHECK(hasError(r, "missing_thread_id"), "missing_thread_id expected");
    PASS();
}

void test_missing_frames_rejected() {
    TEST(missing_frames_rejected);
    auto r = ExceptionStackTraceCapture::capture("RuntimeError", "boom", "thread-1", {});
    CHECK(!r.valid, "missing frames should fail");
    CHECK(hasError(r, "missing_stack_frames"), "missing_stack_frames expected");
    PASS();
}

void test_invalid_frame_function_rejected() {
    TEST(invalid_frame_function_rejected);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1", {{"", "main.cpp", 12, ""}});
    CHECK(!r.valid, "invalid frame should fail");
    CHECK(hasError(r, "invalid_frame:0"), "invalid_frame expected");
    PASS();
}

void test_invalid_frame_file_rejected() {
    TEST(invalid_frame_file_rejected);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1", {{"foo", "", 12, ""}});
    CHECK(!r.valid, "invalid frame should fail");
    CHECK(hasError(r, "invalid_frame:0"), "invalid_frame expected");
    PASS();
}

void test_invalid_frame_line_rejected() {
    TEST(invalid_frame_line_rejected);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1", {{"foo", "main.cpp", 0, ""}});
    CHECK(!r.valid, "invalid frame should fail");
    CHECK(hasError(r, "invalid_frame:0"), "invalid_frame expected");
    PASS();
}

void test_navigation_anchor_is_generated() {
    TEST(navigation_anchor_is_generated);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1", {{"foo", "main.cpp", 12, ""}});
    CHECK(r.valid, "capture should pass");
    CHECK(r.event.frames[0].navigationAnchor == "main.cpp:12", "anchor mismatch");
    PASS();
}

void test_multiple_invalid_frames_reported() {
    TEST(multiple_invalid_frames_reported);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1",
        {{"", "main.cpp", 10, ""}, {"ok", "ok.cpp", 2, ""}, {"bad", "", 2, ""}});
    CHECK(!r.valid, "invalid frames should fail");
    CHECK(hasError(r, "invalid_frame:0"), "first invalid frame expected");
    CHECK(hasError(r, "invalid_frame:2"), "third invalid frame expected");
    CHECK(r.event.frames.size() == 1, "only one valid frame expected");
    PASS();
}

void test_event_fields_persist() {
    TEST(event_fields_persist);
    auto r = ExceptionStackTraceCapture::capture(
        "TypeError", "bad arg", "thread-9", {{"foo", "main.cpp", 12, ""}});
    CHECK(r.event.type == "TypeError", "type mismatch");
    CHECK(r.event.message == "bad arg", "message mismatch");
    CHECK(r.event.threadId == "thread-9", "thread id mismatch");
    PASS();
}

void test_single_valid_frame_is_sufficient() {
    TEST(single_valid_frame_is_sufficient);
    auto r = ExceptionStackTraceCapture::capture(
        "RuntimeError", "boom", "thread-1", {{"foo", "main.cpp", 1, ""}});
    CHECK(r.valid, "single valid frame should pass");
    PASS();
}

int main() {
    std::cout << "Step 557: Exception + Stack Trace Capture\n";

    test_valid_exception_capture_passes();       // 1
    test_missing_type_rejected();                // 2
    test_missing_message_rejected();             // 3
    test_missing_thread_id_rejected();           // 4
    test_missing_frames_rejected();              // 5
    test_invalid_frame_function_rejected();      // 6
    test_invalid_frame_file_rejected();          // 7
    test_invalid_frame_line_rejected();          // 8
    test_navigation_anchor_is_generated();       // 9
    test_multiple_invalid_frames_reported();     // 10
    test_event_fields_persist();                 // 11
    test_single_valid_frame_is_sufficient();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
