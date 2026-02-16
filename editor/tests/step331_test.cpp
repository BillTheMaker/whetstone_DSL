// Step 331: Phase 12b Integration Tests (8 tests)
// Full routing pipeline from skeleton through execution and review.

#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "ReviewGate.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

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

static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.setAgentRole("test-session", AgentRole::Generator);
    return state;
}

static void setupSkeleton(HeadlessEditorState& state, Module* mod) {
    state.openBuffer("test", "# skeleton", "python");
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
}

static void advanceToInProgress(HeadlessEditorState& state, const std::string& itemId) {
    rpcCall(state, "assignTask", {{"itemId", itemId}, {"assignee", "worker"}});
    auto item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_IN_PROGRESS);
    state.workflow->queue.updateItem(itemId, item);
}

// 1. Skeleton → workflow → route all → verify worker types
void test_skeleton_route_all() {
    TEST(skeleton_route_all);
    auto state = makeState();
    auto* mod = createSkeletonModule("proj", "python");
    addSkeletonFunction(mod, "getName", {}, "str");
    addSkeletonFunction(mod, "processData", {"x"}, "int");
    addSkeletonFunction(mod, "validate", {"data"}, "bool");
    setupSkeleton(state, mod);

    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});
    auto resp = rpcCall(state, "routeAllReady");
    CHECK(resp["result"]["routed"].get<int>() == 3, "3 routed");

    // Check that worker types were assigned
    auto items = rpcCall(state, "getReadyTasks")["result"]["items"];
    bool hasTemplate = false, hasSlm = false;
    for (const auto& item : items) {
        std::string wt = item["workerType"].get<std::string>();
        if (wt == "template") hasTemplate = true;
        if (wt == "slm") hasSlm = true;
    }
    CHECK(hasTemplate, "has template-routed item (getter)");
    CHECK(hasSlm, "has slm-routed item");
    PASS();
}

// 2. Getter → route → template → execute → auto-approve
void test_getter_full_lifecycle() {
    TEST(getter_full_lifecycle);
    auto state = makeState();
    auto* mod = createSkeletonModule("proj", "python");
    addSkeletonFunction(mod, "getValue", {}, "int");
    setupSkeleton(state, mod);

    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});
    rpcCall(state, "routeAllReady");

    auto items = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = items[0]["id"].get<std::string>();

    auto execResp = rpcCall(state, "executeTask", {{"itemId", itemId}});
    CHECK(execResp["result"]["workerType"] == "template", "template worker");
    CHECK(execResp["result"]["autoApproved"].get<bool>(), "auto-approved");

    // Check result has getter code
    auto code = execResp["result"]["result"]["generatedCode"].get<std::string>();
    CHECK(code.find("self.value") != std::string::npos, "getter code: " + code);
    PASS();
}

// 3. Complex function → route → LLM → prepared context
void test_complex_llm_context() {
    TEST(complex_llm_context);
    auto state = makeState();
    auto* mod = createSkeletonModule("proj", "python");
    auto* fn = addSkeletonFunction(mod, "buildGraph", {"data"}, "Graph");
    // Set project context width
    auto* cwAnno = new ContextWidthAnnotation();
    cwAnno->id = "cw1"; cwAnno->width = "project";
    fn->addChild("annotations", cwAnno);
    setupSkeleton(state, mod);

    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});
    rpcCall(state, "routeAllReady");

    auto items = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = items[0]["id"].get<std::string>();
    CHECK(items[0]["workerType"] == "llm", "routed to llm");

    auto execResp = rpcCall(state, "executeTask", {{"itemId", itemId}});
    CHECK(execResp["result"]["result"]["confidence"].get<float>() == 0.0f,
          "deferred confidence");
    PASS();
}

// 4. High ambiguity → human → marked for review
void test_high_ambiguity_human() {
    TEST(high_ambiguity_human);
    auto state = makeState();
    auto* mod = createSkeletonModule("proj", "python");
    auto* fn = addSkeletonFunction(mod, "handleEdgeCase", {"input"}, "Result");
    // Set automatability to human
    auto* autoAnno = new AutomatabilityAnnotation();
    autoAnno->id = "aa1"; autoAnno->strategy = "human";
    fn->addChild("annotations", autoAnno);
    setupSkeleton(state, mod);

    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});
    rpcCall(state, "routeAllReady");

    auto items = rpcCall(state, "getReadyTasks")["result"]["items"];
    CHECK(items[0]["workerType"] == "human", "routed to human");

    auto execResp = rpcCall(state, "executeTask", {{"itemId", items[0]["id"].get<std::string>()}});
    CHECK(execResp["result"]["autoApproved"].get<bool>() == false, "not auto-approved");
    PASS();
}

// 5. Dependency chain with mixed routing
void test_dependency_mixed_routing() {
    TEST(dependency_mixed_routing);
    auto state = makeState();
    state.openBuffer("test", "# deps", "python");
    auto* mod = createSkeletonModule("deps", "python");
    addSkeletonFunction(mod, "getName", {}, "str"); // template
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));

    state.workflow = WorkflowState("deps");
    auto makeWI = [](const std::string& id, const std::string& name,
                     const std::string& wtype, const std::vector<std::string>& deps) {
        WorkItem wi;
        wi.id = id; wi.nodeId = "n_" + id; wi.nodeName = name;
        wi.nodeType = "Function"; wi.workerType = wtype;
        wi.priority = "high"; wi.status = WI_PENDING;
        wi.contextWidth = "local"; wi.dependencies = deps;
        wi.createdAt = workItemTimestamp();
        return wi;
    };

    state.workflow->queue.enqueue(makeWI("A", "getName", "template", {}));
    state.workflow->queue.enqueue(makeWI("B", "process", "slm", {"A"}));
    state.workflow->queue.enqueue(makeWI("C", "review", "human", {"B"}));

    CHECK(state.workflow->queue.readyCount() == 1, "only A ready");

    // Execute A (template)
    auto execA = rpcCall(state, "executeTask", {{"itemId", "A"}});
    CHECK(execA["result"]["workerType"] == "template", "A=template");

    // Complete A → B ready
    advanceToInProgress(state, "A");
    rpcCall(state, "completeTask", {{"itemId", "A"}});
    CHECK(state.workflow->queue.readyCount() == 1, "B ready");

    // Execute B (slm)
    auto execB = rpcCall(state, "executeTask", {{"itemId", "B"}});
    CHECK(execB["result"]["workerType"] == "slm", "B=slm");

    // Complete B → C ready
    advanceToInProgress(state, "B");
    rpcCall(state, "completeTask", {{"itemId", "B"}});
    CHECK(state.workflow->queue.readyCount() == 1, "C ready");
    PASS();
}

// 6. Rejection flow with feedback
void test_rejection_with_feedback() {
    TEST(rejection_with_feedback);
    auto state = makeState();
    auto* mod = createSkeletonModule("rej", "python");
    addSkeletonFunction(mod, "transform", {"data"}, "Result");
    setupSkeleton(state, mod);

    rpcCall(state, "createWorkflow", {{"projectName", "rej"}});
    rpcCall(state, "routeAllReady");

    auto items = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = items[0]["id"].get<std::string>();

    // Execute → assign → in-progress → review → reject
    rpcCall(state, "assignTask", {{"itemId", itemId}});
    auto item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_IN_PROGRESS);
    state.workflow->queue.updateItem(itemId, item);
    item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_REVIEW);
    state.workflow->queue.updateItem(itemId, item);

    rpcCall(state, "rejectTask", {{"itemId", itemId}, {"reason", "missing edge cases"}});

    // Item back in ready queue
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    CHECK(ready.size() == 1, "back in queue");
    CHECK(ready[0]["id"] == itemId, "same item");
    PASS();
}

// 7. Review policy: strict → permissive
void test_review_policy_switch() {
    TEST(review_policy_switch);
    ReviewGate gate;

    WorkItem wi;
    wi.id = "t1"; wi.workerType = "deterministic"; wi.reviewRequired = false;
    WorkItemResult result;
    result.confidence = 0.95f;

    // Strict: no rules → require review
    ReviewPolicy strict;
    strict.defaultAction = "require-review";
    auto d1 = gate.shouldAutoApprove(wi, result, strict);
    CHECK(!d1.approved, "strict: not approved");

    // Permissive: default auto-approve
    ReviewPolicy permissive;
    permissive.defaultAction = "auto-approve";
    auto d2 = gate.shouldAutoApprove(wi, result, permissive);
    CHECK(d2.approved, "permissive: approved");
    PASS();
}

// 8. Full lifecycle: 5-function mixed skeleton
void test_full_lifecycle_mixed() {
    TEST(full_lifecycle_mixed);
    auto state = makeState();
    auto* mod = createSkeletonModule("full", "python");
    addSkeletonFunction(mod, "getName", {}, "str");        // template
    addSkeletonFunction(mod, "setValue", {"v"}, "void");   // template
    addSkeletonFunction(mod, "processData", {"x"}, "int"); // slm
    addSkeletonFunction(mod, "validate", {"d"}, "bool");   // slm
    addSkeletonFunction(mod, "isActive", {}, "bool");      // template
    setupSkeleton(state, mod);

    rpcCall(state, "createWorkflow", {{"projectName", "full"}});
    rpcCall(state, "routeAllReady");

    // Count by worker type
    auto items = rpcCall(state, "getReadyTasks")["result"]["items"];
    int templates = 0, slms = 0;
    for (const auto& item : items) {
        std::string wt = item["workerType"].get<std::string>();
        if (wt == "template") templates++;
        if (wt == "slm") slms++;
    }
    CHECK(templates == 3, "3 template items (getName, setValue, isActive), got " +
          std::to_string(templates));
    CHECK(slms == 2, "2 slm items, got " + std::to_string(slms));

    // Execute all template items (auto-approved)
    for (const auto& item : items) {
        if (item["workerType"] == "template") {
            rpcCall(state, "executeTask", {{"itemId", item["id"].get<std::string>()}});
        }
    }

    auto stats = state.workflow->getStats();
    CHECK(stats.total == 5, "5 total");
    PASS();
}

int main() {
    std::cout << "=== Step 331: Phase 12b Integration Tests ===\n";
    try {
        test_skeleton_route_all();
        test_getter_full_lifecycle();
        test_complex_llm_context();
        test_high_ambiguity_human();
        test_dependency_mixed_routing();
        test_rejection_with_feedback();
        test_review_policy_switch();
        test_full_lifecycle_mixed();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
