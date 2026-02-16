// Step 324: Workflow RPC + MCP Tools (12 tests)
// Tests createWorkflow, getReadyTasks, assignTask, completeTask,
// rejectTask, getWorkItem, Linter role restrictions, MCP tool registration,
// saveWorkflow round-trip, getWorkflowState.

#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "MCPServer.h"
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
                    const json& params = json::object(),
                    const std::string& session = "test-session") {
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}};
    if (!params.empty()) req["params"] = params;
    return handleHeadlessAgentRequest(state, req, session);
}

static HeadlessEditorState makeStateWithSkeleton() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    // Create a skeleton module with functions
    auto* mod = createSkeletonModule("testmod", "python");
    addSkeletonFunction(mod, "compute", {"x", "y"}, "int");
    addSkeletonFunction(mod, "validate", {"data"}, "bool");
    addSkeletonFunction(mod, "transform", {"input"}, "string");

    // Open buffer with some source and replace AST
    state.openBuffer("test", "# skeleton", "python");
    // Set the skeleton AST directly
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
    return state;
}

// 1. createWorkflow from skeleton module
void test_create_workflow() {
    TEST(create_workflow);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);

    auto resp = rpcCall(state, "createWorkflow", {{"projectName", "myproj"}});
    CHECK(resp.contains("result"), "has result: " + resp.dump());
    auto r = resp["result"];
    CHECK(r["itemCount"].get<int>() == 3, "3 items created");
    CHECK(r.contains("phase"), "has phase");
    CHECK(r.contains("stats"), "has stats");
    PASS();
}

// 2. getReadyTasks ordering
void test_get_ready_tasks() {
    TEST(get_ready_tasks);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    auto resp = rpcCall(state, "getReadyTasks");
    CHECK(resp.contains("result"), "has result");
    auto r = resp["result"];
    CHECK(r["count"].get<int>() == 3, "3 ready tasks");
    CHECK(r["items"].is_array(), "items is array");
    CHECK(r["items"].size() == 3, "3 items in array");
    PASS();
}

// 3. assignTask transitions
void test_assign_task() {
    TEST(assign_task);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    // Get a ready task
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    auto resp = rpcCall(state, "assignTask", {{"itemId", itemId}, {"assignee", "worker-1"}});
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["success"].get<bool>(), "success");
    CHECK(resp["result"]["item"]["status"] == "assigned", "status=assigned");
    CHECK(resp["result"]["item"]["assignee"] == "worker-1", "assignee set");
    PASS();
}

// 4. completeTask triggers dependency cascade
void test_complete_task() {
    TEST(complete_task);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    // Get and assign a task
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();
    rpcCall(state, "assignTask", {{"itemId", itemId}});

    // Transition to in-progress manually (via queue update)
    auto item = state.workflow->queue.getItem(itemId);
    WorkItem updated = *item;
    transitionWorkItem(updated, WI_IN_PROGRESS);
    state.workflow->queue.updateItem(itemId, updated);

    // Complete with result
    auto resp = rpcCall(state, "completeTask", {
        {"itemId", itemId},
        {"result", {{"generatedCode", "return x + y"}, {"confidence", 0.95}, {"reasoning", "simple"}}}
    });
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["success"].get<bool>(), "success");

    // Verify the item is complete
    auto itemResp = rpcCall(state, "getWorkItem", {{"itemId", itemId}});
    CHECK(itemResp["result"]["status"] == "complete", "item complete");
    CHECK(itemResp["result"]["result"]["generatedCode"] == "return x + y", "code attached");
    PASS();
}

// 5. rejectTask re-enqueues
void test_reject_task() {
    TEST(reject_task);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    // assign → in-progress → review
    rpcCall(state, "assignTask", {{"itemId", itemId}});
    auto item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_IN_PROGRESS);
    state.workflow->queue.updateItem(itemId, item);
    item = *state.workflow->queue.getItem(itemId);
    transitionWorkItem(item, WI_REVIEW);
    state.workflow->queue.updateItem(itemId, item);

    auto resp = rpcCall(state, "rejectTask", {{"itemId", itemId}, {"reason", "needs tests"}});
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["success"].get<bool>(), "success");

    // Item should be back to ready
    auto itemResp = rpcCall(state, "getWorkItem", {{"itemId", itemId}});
    CHECK(itemResp["result"]["status"] == "ready", "back to ready");
    PASS();
}

// 6. getWorkItem returns full details
void test_get_work_item() {
    TEST(get_work_item);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    auto resp = rpcCall(state, "getWorkItem", {{"itemId", itemId}});
    CHECK(resp.contains("result"), "has result");
    auto r = resp["result"];
    CHECK(r.contains("id"), "has id");
    CHECK(r.contains("nodeId"), "has nodeId");
    CHECK(r.contains("nodeName"), "has nodeName");
    CHECK(r.contains("nodeType"), "has nodeType");
    CHECK(r.contains("priority"), "has priority");
    CHECK(r.contains("status"), "has status");
    CHECK(r.contains("result"), "has result field");
    PASS();
}

// 7. Linter role can read but not mutate
void test_linter_restrictions() {
    TEST(linter_restrictions);
    auto state = makeStateWithSkeleton();
    // Create workflow as Generator first
    state.setAgentRole("gen-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}}, "gen-session");

    // Now try as Linter
    state.setAgentRole("lint-session", AgentRole::Linter);

    // Read operations should work
    auto resp1 = rpcCall(state, "getWorkflowState", {}, "lint-session");
    CHECK(resp1.contains("result"), "Linter can getWorkflowState");

    auto resp2 = rpcCall(state, "getReadyTasks", {}, "lint-session");
    CHECK(resp2.contains("result"), "Linter can getReadyTasks");

    // Mutation should fail
    auto ready = resp2["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    auto resp3 = rpcCall(state, "assignTask", {{"itemId", itemId}}, "lint-session");
    CHECK(resp3.contains("error"), "Linter cannot assignTask");

    auto resp4 = rpcCall(state, "createWorkflow", {{"projectName", "x"}}, "lint-session");
    CHECK(resp4.contains("error"), "Linter cannot createWorkflow");
    PASS();
}

// 8. MCP tool registration (8 new tools, 50+ total)
void test_mcp_registration() {
    TEST(mcp_registration);
    MCPServer server;

    const auto& tools = server.getTools();
    // Count workflow execution tools
    int workflowExecCount = 0;
    for (const auto& t : tools) {
        std::string name = t.name;
        if (name == "whetstone_create_workflow" ||
            name == "whetstone_get_workflow_state" ||
            name == "whetstone_get_ready_tasks" ||
            name == "whetstone_get_work_item" ||
            name == "whetstone_assign_task" ||
            name == "whetstone_complete_task" ||
            name == "whetstone_reject_task" ||
            name == "whetstone_save_workflow") {
            workflowExecCount++;
        }
    }
    CHECK(workflowExecCount == 8, "8 workflow execution tools, got " +
          std::to_string(workflowExecCount));
    CHECK((int)tools.size() >= 50, "50+ total tools, got " +
          std::to_string(tools.size()));
    PASS();
}

// 9. saveWorkflow round-trip via RPC
void test_save_workflow_rpc() {
    TEST(save_workflow_rpc);
    std::string testDir = "/tmp/whetstone_step324_test_" + std::to_string(getpid());
    fs::create_directories(testDir);

    auto state = makeStateWithSkeleton();
    state.workspaceRoot = testDir;
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "savetest"}});

    auto resp = rpcCall(state, "saveWorkflow");
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["success"].get<bool>(), "save success");
    CHECK(resp["result"]["bytesWritten"].get<int>() > 0, "bytes written");

    // Verify file exists
    std::string path = testDir + "/.whetstone/savetest.workflow.json";
    CHECK(fs::exists(path), "sidecar file exists");

    fs::remove_all(testDir);
    PASS();
}

// 10. getWorkflowState returns stats
void test_get_workflow_state() {
    TEST(get_workflow_state);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    auto resp = rpcCall(state, "getWorkflowState");
    CHECK(resp.contains("result"), "has result");
    auto r = resp["result"];
    CHECK(r.contains("phase"), "has phase");
    CHECK(r.contains("stats"), "has stats");
    CHECK(r.contains("readyCount"), "has readyCount");
    CHECK(r.contains("blockedCount"), "has blockedCount");
    CHECK(r["readyCount"].get<int>() == 3, "3 ready");
    PASS();
}

// 11. No workflow returns error
void test_no_workflow_error() {
    TEST(no_workflow_error);
    HeadlessEditorState state;
    auto resp = rpcCall(state, "getWorkflowState");
    CHECK(resp.contains("error"), "error when no workflow");
    PASS();
}

// 12. getWorkItem nonexistent returns error
void test_get_nonexistent_item() {
    TEST(get_nonexistent_item);
    auto state = makeStateWithSkeleton();
    state.setAgentRole("test-session", AgentRole::Generator);
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    auto resp = rpcCall(state, "getWorkItem", {{"itemId", "nonexistent"}});
    CHECK(resp.contains("error"), "error for nonexistent item");
    PASS();
}

int main() {
    std::cout << "=== Step 324: Workflow RPC + MCP Tools ===\n";
    try {
        test_create_workflow();
        test_get_ready_tasks();
        test_assign_task();
        test_complete_task();
        test_reject_task();
        test_get_work_item();
        test_linter_restrictions();
        test_mcp_registration();
        test_save_workflow_rpc();
        test_get_workflow_state();
        test_no_workflow_error();
        test_get_nonexistent_item();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
