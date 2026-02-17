// Step 559: Call Stack and Frame Inspector (12 tests)

#include "CallStackFrameInspector.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasError(const FrameInspectorResult& r, const std::string& e) {
    for (const auto& x : r.errors) if (x == e) return true;
    return false;
}

static std::vector<FrameScope> sampleFrames() {
    return {
        {0, "main", "main.cpp", 10, {{"argc", "2"}, {"argv", "ptr"}}},
        {1, "run", "runtime.cpp", 25, {{"cursor", "12"}, {"state", "ok"}}},
        {2, "dispatch", "runtime.cpp", 42, {{"event", "tick"}}}
    };
}

void test_inspect_top_frame() {
    TEST(inspect_top_frame);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 0);
    CHECK(r.ok, "inspect should succeed");
    CHECK(r.selected.functionName == "main", "function mismatch");
    PASS();
}

void test_inspect_middle_frame() {
    TEST(inspect_middle_frame);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 1);
    CHECK(r.ok, "inspect should succeed");
    CHECK(r.selected.functionName == "run", "function mismatch");
    PASS();
}

void test_inspect_bottom_frame() {
    TEST(inspect_bottom_frame);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 2);
    CHECK(r.ok, "inspect should succeed");
    CHECK(r.selected.functionName == "dispatch", "function mismatch");
    PASS();
}

void test_empty_stack_fails() {
    TEST(empty_stack_fails);
    auto r = CallStackFrameInspector::inspect({}, 0);
    CHECK(!r.ok, "empty stack should fail");
    CHECK(hasError(r, "empty_stack"), "empty_stack expected");
    PASS();
}

void test_negative_index_fails() {
    TEST(negative_index_fails);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), -1);
    CHECK(!r.ok, "negative index should fail");
    CHECK(hasError(r, "invalid_frame_index"), "invalid_frame_index expected");
    PASS();
}

void test_index_out_of_range_fails() {
    TEST(index_out_of_range_fails);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 5);
    CHECK(!r.ok, "out-of-range index should fail");
    CHECK(hasError(r, "invalid_frame_index"), "invalid_frame_index expected");
    PASS();
}

void test_invalid_frame_payload_function_fails() {
    TEST(invalid_frame_payload_function_fails);
    auto frames = sampleFrames();
    frames[1].functionName.clear();
    auto r = CallStackFrameInspector::inspect(frames, 1);
    CHECK(!r.ok, "invalid frame payload should fail");
    CHECK(hasError(r, "invalid_frame_payload"), "invalid payload expected");
    PASS();
}

void test_invalid_frame_payload_file_fails() {
    TEST(invalid_frame_payload_file_fails);
    auto frames = sampleFrames();
    frames[1].filePath.clear();
    auto r = CallStackFrameInspector::inspect(frames, 1);
    CHECK(!r.ok, "invalid frame payload should fail");
    CHECK(hasError(r, "invalid_frame_payload"), "invalid payload expected");
    PASS();
}

void test_invalid_frame_payload_line_fails() {
    TEST(invalid_frame_payload_line_fails);
    auto frames = sampleFrames();
    frames[1].line = 0;
    auto r = CallStackFrameInspector::inspect(frames, 1);
    CHECK(!r.ok, "invalid frame payload should fail");
    CHECK(hasError(r, "invalid_frame_payload"), "invalid payload expected");
    PASS();
}

void test_variable_scope_mapping_preserved() {
    TEST(variable_scope_mapping_preserved);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 1);
    CHECK(r.ok, "inspect should succeed");
    CHECK(r.selected.variables.at("cursor") == "12", "variable mapping mismatch");
    PASS();
}

void test_variable_names_exposed() {
    TEST(variable_names_exposed);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 0);
    auto names = CallStackFrameInspector::variableNames(r.selected);
    CHECK(names.size() == 2, "two names expected");
    PASS();
}

void test_selected_index_reported() {
    TEST(selected_index_reported);
    auto r = CallStackFrameInspector::inspect(sampleFrames(), 2);
    CHECK(r.ok, "inspect should succeed");
    CHECK(r.selectedIndex == 2, "selected index mismatch");
    PASS();
}

int main() {
    std::cout << "Step 559: Call Stack and Frame Inspector\n";

    test_inspect_top_frame();                         // 1
    test_inspect_middle_frame();                      // 2
    test_inspect_bottom_frame();                      // 3
    test_empty_stack_fails();                         // 4
    test_negative_index_fails();                      // 5
    test_index_out_of_range_fails();                  // 6
    test_invalid_frame_payload_function_fails();      // 7
    test_invalid_frame_payload_file_fails();          // 8
    test_invalid_frame_payload_line_fails();          // 9
    test_variable_scope_mapping_preserved();          // 10
    test_variable_names_exposed();                    // 11
    test_selected_index_reported();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
