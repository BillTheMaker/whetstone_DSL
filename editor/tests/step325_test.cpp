// Step 325: Phase 12a Integration Tests (8 tests)
// End-to-end workflow lifecycle from skeleton to completion.

#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "WorkflowPersistence.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;
using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json rpcCall(HeadlessEditorState& state, const std::string& method,
                    const json& params = json::object()) {
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}};
    if (!params.empty()) req["params"] = params;
    return handleHeadlessAgentRequest(state, req, "test-session");
}

// Helper: make state with skeleton and Generator role
static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.setAgentRole("test-session", AgentRole::Generator);
    return state;
}

// Helper: build skeleton module with annotations on a state
static void setupSkeleton(HeadlessEditorState& state, Module* mod) {
    state.openBuffer("test", "# skeleton", "python");
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
}

// Helper: transition a work item through assign → in-progress via queue
static void advanceToInProgress(HeadlessEditorState& state, const std::string& itemId) {
    rpcCall(state, "assignTask", {{"itemId", itemId}, {"assignee", "worker"}});
    auto item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_IN_PROGRESS);
    state.workflow->queue.updateItem(itemId, item);
}

// 1. Create skeleton module → createWorkflow → verify item count
void test_skeleton_to_workflow() {
    TEST(skeleton_to_workflow);
    auto state = makeState();
    auto* mod = createSkeletonModule("proj", "python");
    addSkeletonFunction(mod, "alpha", {"x"}, "int");
    addSkeletonFunction(mod, "beta", {"y"}, "str");
    addSkeletonFunction(mod, "gamma", {}, "void");
    addSkeletonFunction(mod, "delta", {"a", "b"}, "float");
    setupSkeleton(state, mod);

    auto resp = rpcCall(state, "createWorkflow", {{"projectName", "proj"}});
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["itemCount"].get<int>() == 4, "4 items from 4 functions");
    PASS();
}

// 2. Priority ordering: critical before low
void test_priority_ordering() {
    TEST(priority_ordering);
    auto state = makeState();
    auto* mod = createSkeletonModule("proj", "python");

    // Add functions with different priorities via annotations
    auto* lowFn = addSkeletonFunction(mod, "lowPri", {"x"}, "int");
    auto* lowPrio = new PriorityAnnotation();
    lowPrio->id = "pa1"; lowPrio->level = "low";
    lowFn->addChild("annotations", lowPrio);

    auto* critFn = addSkeletonFunction(mod, "critPri", {"x"}, "int");
    auto* critPrio = new PriorityAnnotation();
    critPrio->id = "pa2"; critPrio->level = "critical";
    critFn->addChild("annotations", critPrio);

    setupSkeleton(state, mod);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    auto resp = rpcCall(state, "getReadyTasks");
    auto items = resp["result"]["items"];
    CHECK(items.size() == 2, "2 items");

    // Critical should be first
    CHECK(items[0]["priority"] == "critical", "first=critical, got " +
          items[0]["priority"].get<std::string>());
    CHECK(items[1]["priority"] == "low", "second=low");
    PASS();
}

// 3. Dependency chain: A blocks B blocks C
void test_dependency_chain() {
    TEST(dependency_chain);
    auto state = makeState();

    // Manually create workflow with dependency chain
    state.openBuffer("test", "# dep chain", "python");
    auto* mod = createSkeletonModule("deps", "python");
    addSkeletonFunction(mod, "funcA", {}, "void");
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));

    // Create workflow then manually add items with deps
    state.workflow = WorkflowState("deps");
    WorkItem a, b, c;
    a.id = "A"; a.nodeId = "nA"; a.nodeName = "funcA"; a.nodeType = "Function";
    a.priority = "high"; a.status = WI_PENDING; a.createdAt = workItemTimestamp();

    b.id = "B"; b.nodeId = "nB"; b.nodeName = "funcB"; b.nodeType = "Function";
    b.priority = "high"; b.status = WI_PENDING; b.dependencies = {"A"};
    b.createdAt = workItemTimestamp();

    c.id = "C"; c.nodeId = "nC"; c.nodeName = "funcC"; c.nodeType = "Function";
    c.priority = "high"; c.status = WI_PENDING; c.dependencies = {"B"};
    c.createdAt = workItemTimestamp();

    state.workflow->queue.enqueue(a);
    state.workflow->queue.enqueue(b);
    state.workflow->queue.enqueue(c);

    CHECK(state.workflow->queue.readyCount() == 1, "only A ready");

    // Complete A
    advanceToInProgress(state, "A");
    rpcCall(state, "completeTask", {{"itemId", "A"}});
    CHECK(state.workflow->queue.readyCount() == 1, "B now ready");

    // Complete B
    advanceToInProgress(state, "B");
    rpcCall(state, "completeTask", {{"itemId", "B"}});
    CHECK(state.workflow->queue.readyCount() == 1, "C now ready");

    auto peek = state.workflow->queue.peek();
    CHECK(peek.has_value() && peek->id == "C", "C is next");
    PASS();
}

// 4. Full lifecycle: create → assign → in-progress → complete with result
void test_full_lifecycle() {
    TEST(full_lifecycle);
    auto state = makeState();
    auto* mod = createSkeletonModule("life", "python");
    addSkeletonFunction(mod, "compute", {"x"}, "int");
    setupSkeleton(state, mod);
    rpcCall(state, "createWorkflow", {{"projectName", "life"}});

    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    // Assign
    rpcCall(state, "assignTask", {{"itemId", itemId}, {"assignee", "llm-worker"}});
    auto item = rpcCall(state, "getWorkItem", {{"itemId", itemId}})["result"];
    CHECK(item["status"] == "assigned", "assigned");

    // In-progress
    advanceToInProgress(state, itemId);

    // Complete with result
    auto resp = rpcCall(state, "completeTask", {
        {"itemId", itemId},
        {"result", {{"generatedCode", "return x * 2"}, {"confidence", 0.9}, {"reasoning", "doubling"}}}
    });
    CHECK(resp["result"]["success"].get<bool>(), "complete success");

    // Verify result attached
    item = rpcCall(state, "getWorkItem", {{"itemId", itemId}})["result"];
    CHECK(item["status"] == "complete", "complete");
    CHECK(item["result"]["generatedCode"] == "return x * 2", "code attached");
    CHECK(item["result"]["confidence"].get<float>() > 0.89f, "confidence");

    // Phase should be Complete
    auto ws = rpcCall(state, "getWorkflowState")["result"];
    CHECK(ws["phase"] == "complete", "phase=complete");
    PASS();
}

// 5. Rejection flow: assign → reject → re-appears in ready
void test_rejection_flow() {
    TEST(rejection_flow);
    auto state = makeState();
    auto* mod = createSkeletonModule("rej", "python");
    addSkeletonFunction(mod, "validate", {"data"}, "bool");
    setupSkeleton(state, mod);
    rpcCall(state, "createWorkflow", {{"projectName", "rej"}});

    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    // assign → in-progress → review → reject
    rpcCall(state, "assignTask", {{"itemId", itemId}});
    auto item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_IN_PROGRESS);
    state.workflow->queue.updateItem(itemId, item);
    item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_REVIEW);
    state.workflow->queue.updateItem(itemId, item);

    rpcCall(state, "rejectTask", {{"itemId", itemId}, {"reason", "missing edge cases"}});

    // Should be back in ready queue
    ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    CHECK(ready.size() == 1, "1 ready after reject");
    CHECK(ready[0]["id"] == itemId, "same item back in queue");
    PASS();
}

// 6. Persistence: save → reload → verify items and history
void test_persistence_roundtrip() {
    TEST(persistence_roundtrip);
    std::string testDir = "/tmp/whetstone_step325_test_" + std::to_string(getpid());
    fs::create_directories(testDir);

    auto state = makeState();
    state.workspaceRoot = testDir;
    auto* mod = createSkeletonModule("persist", "python");
    addSkeletonFunction(mod, "f1", {}, "void");
    addSkeletonFunction(mod, "f2", {}, "void");
    setupSkeleton(state, mod);
    rpcCall(state, "createWorkflow", {{"projectName", "persist"}});

    // Make some changes
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string id0 = ready[0]["id"].get<std::string>();
    rpcCall(state, "assignTask", {{"itemId", id0}, {"assignee", "w1"}});

    // Save
    rpcCall(state, "saveWorkflow");

    // Load into new state
    auto loaded = loadWorkflow(testDir, "persist");
    CHECK(loaded.has_value(), "loaded");
    CHECK(loaded->queue.size() == 2, "2 items");
    CHECK(loaded->projectName == "persist", "name preserved");

    // Check history
    auto hist = loaded->getHistory(id0);
    CHECK(!hist.empty(), "history for assigned item");

    fs::remove_all(testDir);
    PASS();
}

// 7. Stats accuracy with mixed statuses
void test_stats_accuracy() {
    TEST(stats_accuracy);
    auto state = makeState();

    state.openBuffer("test", "# stats", "python");
    auto* mod = createSkeletonModule("stats", "python");
    addSkeletonFunction(mod, "f1", {}, "void");
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));

    // Build workflow manually with mixed statuses
    state.workflow = WorkflowState("stats");
    auto addItem = [&](const std::string& id, const std::string& status,
                       const std::string& prio, const std::string& wtype) {
        WorkItem wi;
        wi.id = id; wi.nodeId = "n" + id; wi.nodeName = "f" + id;
        wi.nodeType = "Function"; wi.priority = prio;
        wi.workerType = wtype; wi.status = status;
        wi.createdAt = workItemTimestamp();
        state.workflow->queue.enqueue(wi);
    };

    addItem("s1", WI_READY, "high", "llm");
    addItem("s2", WI_ASSIGNED, "medium", "llm");
    addItem("s3", WI_COMPLETE, "low", "deterministic");
    addItem("s4", WI_COMPLETE, "critical", "llm");

    auto resp = rpcCall(state, "getWorkflowState");
    auto stats = resp["result"]["stats"];
    CHECK(stats["total"].get<int>() == 4, "total=4");
    CHECK(stats["ready"].get<int>() == 1, "ready=1");
    CHECK(stats["assigned"].get<int>() == 1, "assigned=1");
    CHECK(stats["complete"].get<int>() == 2, "complete=2");
    CHECK(stats["completionPercent"].get<float>() > 49.0f, "50%");
    PASS();
}

// 8. Combined: 5 functions, mixed priorities + dependencies, complete in order
void test_combined_workflow() {
    TEST(combined_workflow);
    auto state = makeState();
    state.openBuffer("test", "# combined", "python");
    auto* mod = createSkeletonModule("combined", "python");
    addSkeletonFunction(mod, "init", {}, "void");
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));

    state.workflow = WorkflowState("combined");

    // 5 items: init (critical, no deps), parse (high, deps on init),
    // validate (medium, deps on parse), transform (medium, deps on parse),
    // output (low, deps on validate + transform)
    auto makeWI = [](const std::string& id, const std::string& prio,
                     const std::vector<std::string>& deps) {
        WorkItem wi;
        wi.id = id; wi.nodeId = "n_" + id; wi.nodeName = id;
        wi.nodeType = "Function"; wi.priority = prio;
        wi.workerType = "llm"; wi.status = WI_PENDING;
        wi.dependencies = deps;
        wi.createdAt = workItemTimestamp();
        return wi;
    };

    state.workflow->queue.enqueue(makeWI("init", "critical", {}));
    state.workflow->queue.enqueue(makeWI("parse", "high", {"init"}));
    state.workflow->queue.enqueue(makeWI("validate", "medium", {"parse"}));
    state.workflow->queue.enqueue(makeWI("transform", "medium", {"parse"}));
    state.workflow->queue.enqueue(makeWI("output", "low", {"validate", "transform"}));

    CHECK(state.workflow->queue.readyCount() == 1, "only init ready");
    CHECK(state.workflow->getPhase() == WorkflowPhase::Routing, "phase=Routing");

    // Complete init
    advanceToInProgress(state, "init");
    rpcCall(state, "completeTask", {{"itemId", "init"}});
    CHECK(state.workflow->queue.readyCount() == 1, "parse now ready");

    // Complete parse
    advanceToInProgress(state, "parse");
    rpcCall(state, "completeTask", {{"itemId", "parse"}});
    CHECK(state.workflow->queue.readyCount() == 2, "validate + transform ready");

    // Complete validate and transform
    advanceToInProgress(state, "validate");
    rpcCall(state, "completeTask", {{"itemId", "validate"}});
    advanceToInProgress(state, "transform");
    rpcCall(state, "completeTask", {{"itemId", "transform"}});
    CHECK(state.workflow->queue.readyCount() == 1, "output ready");

    // Complete output
    advanceToInProgress(state, "output");
    rpcCall(state, "completeTask", {{"itemId", "output"}});

    // All complete
    CHECK(state.workflow->getPhase() == WorkflowPhase::Complete, "phase=Complete");
    auto stats = state.workflow->getStats();
    CHECK(stats.complete == 5, "5 complete");
    CHECK(stats.completionPercent > 99.0f, "100%");
    PASS();
}

int main() {
    std::cout << "=== Step 325: Phase 12a Integration Tests ===\n";
    try {
        test_skeleton_to_workflow();
        test_priority_ordering();
        test_dependency_chain();
        test_full_lifecycle();
        test_rejection_flow();
        test_persistence_roundtrip();
        test_stats_accuracy();
        test_combined_workflow();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
