// Step 320: WorkItem — Core Execution Model (12 tests)
// Tests lifecycle state machine, JSON roundtrip, timestamps, priority ordering.

#include "WorkItem.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <set>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. WorkItem construction from SkeletonTask — fields copied correctly
void test_create_from_skeleton() {
    TEST(create_from_skeleton);
    SkeletonTask task;
    task.nodeId = "fn_42";
    task.nodeName = "processData";
    task.nodeType = "Function";
    task.contextWidth = "module";
    task.automatability = "llm";
    task.priority = "high";
    task.reviewRequired = true;
    task.dependencies = {"fn_10", "fn_20"};

    WorkItem wi = createWorkItem(task, "buf-1");
    CHECK(wi.nodeId == "fn_42", "nodeId");
    CHECK(wi.nodeName == "processData", "nodeName");
    CHECK(wi.nodeType == "Function", "nodeType");
    CHECK(wi.bufferId == "buf-1", "bufferId");
    CHECK(wi.contextWidth == "module", "contextWidth");
    CHECK(wi.workerType == "llm", "workerType");
    CHECK(wi.priority == "high", "priority");
    CHECK(wi.reviewRequired == true, "reviewRequired");
    CHECK(wi.dependencies.size() == 2, "dependencies count");
    CHECK(wi.status == "pending", "initial status");
    PASS();
}

// 2. ID generation — unique, non-empty
void test_id_generation() {
    TEST(id_generation);
    SkeletonTask task;
    task.nodeId = "n1";
    task.nodeName = "foo";
    task.nodeType = "Function";

    std::set<std::string> ids;
    for (int i = 0; i < 100; ++i) {
        WorkItem wi = createWorkItem(task, "buf");
        CHECK(!wi.id.empty(), "ID non-empty");
        ids.insert(wi.id);
    }
    CHECK(ids.size() == 100, "All 100 IDs unique, got " + std::to_string(ids.size()));
    PASS();
}

// 3. Valid state transitions
void test_valid_transitions() {
    TEST(valid_transitions);
    CHECK(canTransition("pending", "ready"), "pending→ready");
    CHECK(canTransition("ready", "assigned"), "ready→assigned");
    CHECK(canTransition("assigned", "in-progress"), "assigned→in-progress");
    CHECK(canTransition("in-progress", "review"), "in-progress→review");
    CHECK(canTransition("in-progress", "complete"), "in-progress→complete");
    CHECK(canTransition("review", "complete"), "review→complete");
    CHECK(canTransition("review", "rejected"), "review→rejected");
    CHECK(canTransition("rejected", "ready"), "rejected→ready");
    PASS();
}

// 4. Invalid state transitions rejected
void test_invalid_transitions() {
    TEST(invalid_transitions);
    CHECK(!canTransition("pending", "complete"), "pending→complete blocked");
    CHECK(!canTransition("complete", "ready"), "complete→ready blocked");
    CHECK(!canTransition("pending", "in-progress"), "pending→in-progress blocked");
    CHECK(!canTransition("assigned", "complete"), "assigned→complete blocked");
    CHECK(!canTransition("ready", "review"), "ready→review blocked");
    CHECK(!canTransition("complete", "pending"), "complete→pending blocked");
    PASS();
}

// 5. transitionWorkItem populates timestamps
void test_transition_timestamps() {
    TEST(transition_timestamps);
    SkeletonTask task;
    task.nodeId = "n1";
    task.nodeName = "foo";
    task.nodeType = "Function";
    WorkItem wi = createWorkItem(task, "buf");

    CHECK(wi.assignedAt.empty(), "assignedAt initially empty");
    CHECK(wi.completedAt.empty(), "completedAt initially empty");

    transitionWorkItem(wi, "ready");
    transitionWorkItem(wi, "assigned");
    CHECK(!wi.assignedAt.empty(), "assignedAt set on assigned");

    transitionWorkItem(wi, "in-progress");
    transitionWorkItem(wi, "complete");
    CHECK(!wi.completedAt.empty(), "completedAt set on complete");
    PASS();
}

// 6. WorkItemResult toJson/fromJson roundtrip
void test_result_roundtrip() {
    TEST(result_roundtrip);
    WorkItemResult r;
    r.generatedCode = "def foo(): return 42";
    r.astJson = json::parse(R"({"type":"Function","name":"foo"})");
    r.diagnostics = {json{{"level", "warning"}, {"msg", "unused var"}}};
    r.confidence = 0.95f;
    r.tokensBudget = 4000;
    r.tokensGenerated = 1200;
    r.reasoning = "Simple pure function";

    json j = r.toJson();
    WorkItemResult r2 = WorkItemResult::fromJson(j);
    CHECK(r2.generatedCode == r.generatedCode, "generatedCode");
    CHECK(r2.astJson == r.astJson, "astJson");
    CHECK(r2.diagnostics.size() == 1, "diagnostics count");
    CHECK(r2.confidence > 0.94f && r2.confidence < 0.96f, "confidence");
    CHECK(r2.tokensBudget == 4000, "tokensBudget");
    CHECK(r2.tokensGenerated == 1200, "tokensGenerated");
    CHECK(r2.reasoning == "Simple pure function", "reasoning");
    PASS();
}

// 7. WorkItem toJson/fromJson roundtrip — all fields preserved
void test_workitem_roundtrip() {
    TEST(workitem_roundtrip);
    WorkItem wi;
    wi.id = "wi-abc123";
    wi.nodeId = "fn_1";
    wi.nodeName = "compute";
    wi.nodeType = "Function";
    wi.bufferId = "buf-main";
    wi.contextWidth = "project";
    wi.workerType = "llm";
    wi.reviewRequired = true;
    wi.priority = "critical";
    wi.dependencies = {"fn_0"};
    wi.status = "in-progress";
    wi.assignee = "worker-7";
    wi.createdAt = "2026-01-01T00:00:00Z";
    wi.assignedAt = "2026-01-01T00:01:00Z";
    wi.completedAt = "";
    wi.result.generatedCode = "int x = 1;";
    wi.result.confidence = 0.8f;

    json j = workItemToJson(wi);
    WorkItem wi2 = workItemFromJson(j);

    CHECK(wi2.id == wi.id, "id");
    CHECK(wi2.nodeId == wi.nodeId, "nodeId");
    CHECK(wi2.nodeName == wi.nodeName, "nodeName");
    CHECK(wi2.nodeType == wi.nodeType, "nodeType");
    CHECK(wi2.bufferId == wi.bufferId, "bufferId");
    CHECK(wi2.contextWidth == wi.contextWidth, "contextWidth");
    CHECK(wi2.workerType == wi.workerType, "workerType");
    CHECK(wi2.reviewRequired == wi.reviewRequired, "reviewRequired");
    CHECK(wi2.priority == wi.priority, "priority");
    CHECK(wi2.dependencies.size() == 1, "dependencies");
    CHECK(wi2.status == wi.status, "status");
    CHECK(wi2.assignee == wi.assignee, "assignee");
    CHECK(wi2.createdAt == wi.createdAt, "createdAt");
    CHECK(wi2.assignedAt == wi.assignedAt, "assignedAt");
    CHECK(wi2.result.generatedCode == "int x = 1;", "result.generatedCode");
    PASS();
}

// 8. isTerminal — complete=true, rejected=true, others=false
void test_is_terminal() {
    TEST(is_terminal);
    CHECK(isTerminal("complete"), "complete is terminal");
    CHECK(isTerminal("rejected"), "rejected is terminal");
    CHECK(!isTerminal("pending"), "pending not terminal");
    CHECK(!isTerminal("ready"), "ready not terminal");
    CHECK(!isTerminal("assigned"), "assigned not terminal");
    CHECK(!isTerminal("in-progress"), "in-progress not terminal");
    CHECK(!isTerminal("review"), "review not terminal");
    PASS();
}

// 9. createWorkItem sets createdAt timestamp
void test_created_at_timestamp() {
    TEST(created_at_timestamp);
    SkeletonTask task;
    task.nodeId = "n1";
    task.nodeName = "bar";
    task.nodeType = "Function";
    WorkItem wi = createWorkItem(task, "buf");
    CHECK(!wi.createdAt.empty(), "createdAt non-empty");
    // Should look like ISO timestamp: starts with 20
    CHECK(wi.createdAt.substr(0, 2) == "20", "createdAt starts with year");
    CHECK(wi.createdAt.find('T') != std::string::npos, "createdAt has T separator");
    PASS();
}

// 10. Dependencies preserved from SkeletonTask
void test_dependencies_preserved() {
    TEST(dependencies_preserved);
    SkeletonTask task;
    task.nodeId = "n5";
    task.nodeName = "aggregate";
    task.nodeType = "Function";
    task.dependencies = {"n1", "n2", "n3"};

    WorkItem wi = createWorkItem(task, "buf");
    CHECK(wi.dependencies.size() == 3, "3 dependencies");
    CHECK(wi.dependencies[0] == "n1", "dep 0");
    CHECK(wi.dependencies[1] == "n2", "dep 1");
    CHECK(wi.dependencies[2] == "n3", "dep 2");
    PASS();
}

// 11. priorityToInt ordering (critical < high < medium < low)
void test_priority_ordering() {
    TEST(priority_ordering);
    CHECK(priorityToInt("critical") < priorityToInt("high"), "critical < high");
    CHECK(priorityToInt("high") < priorityToInt("medium"), "high < medium");
    CHECK(priorityToInt("medium") < priorityToInt("low"), "medium < low");
    CHECK(priorityToInt("critical") == 0, "critical=0");
    CHECK(priorityToInt("low") == 3, "low=3");
    PASS();
}

// 12. Result attachment — set result, verify in JSON roundtrip
void test_result_attachment() {
    TEST(result_attachment);
    SkeletonTask task;
    task.nodeId = "n1";
    task.nodeName = "gen";
    task.nodeType = "Function";
    task.contextWidth = "local";
    task.automatability = "llm";
    task.priority = "medium";

    WorkItem wi = createWorkItem(task, "buf-2");
    wi.result.generatedCode = "return 42;";
    wi.result.confidence = 0.99f;
    wi.result.tokensBudget = 2000;
    wi.result.tokensGenerated = 50;
    wi.result.reasoning = "trivial";
    wi.result.diagnostics = {json{{"level", "info"}, {"msg", "ok"}}};

    json j = workItemToJson(wi);
    WorkItem wi2 = workItemFromJson(j);
    CHECK(wi2.result.generatedCode == "return 42;", "code preserved");
    CHECK(wi2.result.confidence > 0.98f, "confidence preserved");
    CHECK(wi2.result.tokensBudget == 2000, "budget preserved");
    CHECK(wi2.result.tokensGenerated == 50, "generated preserved");
    CHECK(wi2.result.reasoning == "trivial", "reasoning preserved");
    CHECK(wi2.result.diagnostics.size() == 1, "diagnostics preserved");
    PASS();
}

int main() {
    std::cout << "=== Step 320: WorkItem Core Execution Model ===\n";
    try {
        test_create_from_skeleton();
        test_id_generation();
        test_valid_transitions();
        test_invalid_transitions();
        test_transition_timestamps();
        test_result_roundtrip();
        test_workitem_roundtrip();
        test_is_terminal();
        test_created_at_timestamp();
        test_dependencies_preserved();
        test_priority_ordering();
        test_result_attachment();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
