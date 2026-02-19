// Step 684: AgentSessionRecorder (12 tests)

#include "AgentSessionRecorder.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

void t1() {
    TEST(start_sets_session_id);
    AgentSessionRecorder r;
    r.start("S1", "task");
    CHECK(r.current().sessionId == "S1", "session id mismatch");
    PASS();
}

void t2() {
    TEST(record_adds_call);
    AgentSessionRecorder r;
    r.start("S1", "task");
    r.record(AgentSessionRecorder::makeRecord("tool", "{}", "{}", 1));
    CHECK(r.current().calls.size() == 1, "call not recorded");
    PASS();
}

void t3() {
    TEST(finish_returns_complete_record);
    AgentSessionRecorder r;
    r.start("S1", "task");
    r.record(AgentSessionRecorder::makeRecord("tool", "{}", "{}", 1));
    auto out = r.finish();
    CHECK(out.sessionId == "S1", "wrong session");
    CHECK(out.calls.size() == 1, "call count mismatch");
    PASS();
}

void t4() {
    TEST(total_tool_calls_matches_count);
    AgentSessionRecorder r;
    r.start("S1", "task");
    r.record(AgentSessionRecorder::makeRecord("a", "{}", "{}", 1));
    r.record(AgentSessionRecorder::makeRecord("b", "{}", "{}", 1));
    auto out = r.finish();
    CHECK(out.totalToolCalls == 2, "expected 2");
    PASS();
}

void t5() {
    TEST(total_estimated_tokens_sum);
    AgentSessionRecorder r;
    r.start("S1", "task");
    r.record(AgentSessionRecorder::makeRecord("a", "1234", "1234", 1));
    r.record(AgentSessionRecorder::makeRecord("b", "12", "12", 1));
    auto out = r.finish();
    CHECK(out.totalEstimatedTokens == 3, "token sum mismatch");
    PASS();
}

void t6() {
    TEST(file_read_count_tracks_file_read_tools);
    AgentSessionRecorder r;
    r.start("S1", "task");
    r.record(AgentSessionRecorder::makeRecord("whetstone_file_read", "{}", "{}", 1));
    r.record(AgentSessionRecorder::makeRecord("whetstone_generate_code", "{}", "{}", 1));
    auto out = r.finish();
    CHECK(out.fileReadCount == 1, "expected one file read");
    PASS();
}

void t7() {
    TEST(was_file_read_true_for_read_tools);
    auto rec = AgentSessionRecorder::makeRecord("whetstone_file_read", "{}", "{}", 1);
    CHECK(rec.wasFileRead, "expected true");
    PASS();
}

void t8() {
    TEST(was_file_read_false_for_non_read_tools);
    auto rec = AgentSessionRecorder::makeRecord("whetstone_generate_code", "{}", "{}", 1);
    CHECK(!rec.wasFileRead, "expected false");
    PASS();
}

void t9() {
    TEST(estimated_tokens_formula);
    auto rec = AgentSessionRecorder::makeRecord("t", "1234", "1234", 1);
    CHECK(rec.estimatedTokens == 2, "token formula mismatch");
    PASS();
}

void t10() {
    TEST(duration_preserved);
    auto rec = AgentSessionRecorder::makeRecord("t", "{}", "{}", 55);
    CHECK(rec.durationMs == 55, "duration mismatch");
    PASS();
}

void t11() {
    TEST(task_description_preserved);
    AgentSessionRecorder r;
    r.start("S1", "Refactor parser");
    auto out = r.finish();
    CHECK(out.taskDescription == "Refactor parser", "task description mismatch");
    PASS();
}

void t12() {
    TEST(empty_session_zero_counts);
    AgentSessionRecorder r;
    auto out = r.finish();
    CHECK(out.totalToolCalls == 0, "expected zero calls");
    CHECK(out.totalEstimatedTokens == 0, "expected zero tokens");
    PASS();
}

int main() {
    std::cout << "Step 684: AgentSessionRecorder\n";
    t1(); t2(); t3(); t4(); t5(); t6();
    t7(); t8(); t9(); t10(); t11(); t12();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

