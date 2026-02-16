// Step 329: Routing RPC + MCP Tools (12 tests)
// Tests routeTask, routeAllReady, executeTask, getRoutingExplanation,
// Linter restrictions, MCP registration.

#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "MCPServer.h"
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
                    const json& params = json::object(),
                    const std::string& session = "test-session") {
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method}};
    if (!params.empty()) req["params"] = params;
    return handleHeadlessAgentRequest(state, req, session);
}

// Setup: state with skeleton workflow
static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.setAgentRole("test-session", AgentRole::Generator);

    auto* mod = createSkeletonModule("proj", "python");
    addSkeletonFunction(mod, "getName", {}, "str");     // getter → deterministic
    addSkeletonFunction(mod, "processData", {"x"}, "int"); // local → slm
    addSkeletonFunction(mod, "orchestrate", {"ctx"}, "void"); // will set to project

    state.openBuffer("test", "# skeleton", "python");
    state.activeBuffer->sync.setAST(std::unique_ptr<Module>(mod));
    rpcCall(state, "createWorkflow", {{"projectName", "proj"}});

    return state;
}

// 1. routeTask returns valid decision
void test_route_task() {
    TEST(route_task);
    auto state = makeState();
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    auto resp = rpcCall(state, "routeTask", {{"itemId", itemId}});
    CHECK(resp.contains("result"), "has result");
    auto decision = resp["result"]["decision"];
    CHECK(decision.contains("workerType"), "has workerType");
    CHECK(decision.contains("contextWidth"), "has contextWidth");
    CHECK(decision.contains("contextBudgetTokens"), "has budget");
    CHECK(decision.contains("reasoning"), "has reasoning");
    PASS();
}

// 2. routeAllReady batch processes queue
void test_route_all_ready() {
    TEST(route_all_ready);
    auto state = makeState();

    auto resp = rpcCall(state, "routeAllReady");
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["routed"].get<int>() == 3, "3 items routed");
    CHECK(resp["result"]["decisions"].size() == 3, "3 decisions");
    PASS();
}

// 3. executeTask runs deterministic worker — produces code
void test_execute_deterministic() {
    TEST(execute_deterministic);
    auto state = makeState();

    // Route all first to set worker types
    rpcCall(state, "routeAllReady");

    // Find the getter item (should be deterministic)
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string getterId;
    for (const auto& item : ready) {
        if (item["nodeName"] == "getName") {
            getterId = item["id"].get<std::string>();
            break;
        }
    }
    CHECK(!getterId.empty(), "found getter item");

    auto resp = rpcCall(state, "executeTask", {{"itemId", getterId}});
    CHECK(resp.contains("result"), "has result: " + resp.dump().substr(0, 200));
    CHECK(resp["result"]["workerType"] == "template", "template worker");
    CHECK(!resp["result"]["result"]["generatedCode"].get<std::string>().empty(),
          "produced code");
    PASS();
}

// 4. Agent items marked as prepared, not executed
void test_agent_prepared() {
    TEST(agent_prepared);
    auto state = makeState();
    rpcCall(state, "routeAllReady");

    // Find an SLM-routed item
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string slmId;
    for (const auto& item : ready) {
        if (item["workerType"] == "slm") {
            slmId = item["id"].get<std::string>();
            break;
        }
    }
    CHECK(!slmId.empty(), "found slm item");

    auto resp = rpcCall(state, "executeTask", {{"itemId", slmId}});
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["result"]["confidence"].get<float>() == 0.0f,
          "deferred confidence");
    CHECK(resp["result"]["result"]["reasoning"].get<std::string>().find("MCP") !=
          std::string::npos, "mentions MCP");
    PASS();
}

// 5. Human items marked for review
void test_human_marked() {
    TEST(human_marked);
    auto state = makeState();

    // Manually set an item to human worker type
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();
    auto item = *state.workflow->queue.getItem(itemId);
    item.workerType = "human";
    state.workflow->queue.updateItem(itemId, item);

    auto resp = rpcCall(state, "executeTask", {{"itemId", itemId}});
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"]["workerType"] == "human", "human worker");
    CHECK(resp["result"]["result"]["reasoning"].get<std::string>().find("human") !=
          std::string::npos, "mentions human");
    PASS();
}

// 6. Routing explanation includes annotation references
void test_routing_explanation() {
    TEST(routing_explanation);
    auto state = makeState();
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    auto resp = rpcCall(state, "getRoutingExplanation", {{"itemId", itemId}});
    CHECK(resp.contains("result"), "has result");
    CHECK(resp["result"].contains("reasoning"), "has reasoning");
    CHECK(resp["result"].contains("annotationsUsed"), "has annotationsUsed");
    CHECK(resp["result"].contains("rulesApplied"), "has rulesApplied");
    CHECK(resp["result"].contains("decision"), "has decision");
    PASS();
}

// 7. Linter can read explanation but not route
void test_linter_restrictions() {
    TEST(linter_restrictions);
    auto state = makeState();
    state.setAgentRole("lint", AgentRole::Linter);

    auto ready = rpcCall(state, "getReadyTasks", {}, "lint")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    // Read: should work
    auto resp1 = rpcCall(state, "getRoutingExplanation",
                         {{"itemId", itemId}}, "lint");
    CHECK(resp1.contains("result"), "Linter can read explanation");

    // Mutate: should fail
    auto resp2 = rpcCall(state, "routeTask", {{"itemId", itemId}}, "lint");
    CHECK(resp2.contains("error"), "Linter cannot route");

    auto resp3 = rpcCall(state, "executeTask", {{"itemId", itemId}}, "lint");
    CHECK(resp3.contains("error"), "Linter cannot execute");
    PASS();
}

// 8. MCP tool registration (4 new routing tools)
void test_mcp_registration() {
    TEST(mcp_registration);
    MCPServer server;
    const auto& tools = server.getTools();

    int routingCount = 0;
    for (const auto& t : tools) {
        if (t.name == "whetstone_route_task" ||
            t.name == "whetstone_route_all_ready" ||
            t.name == "whetstone_execute_task" ||
            t.name == "whetstone_get_routing_explanation") {
            routingCount++;
        }
    }
    CHECK(routingCount == 4, "4 routing tools, got " + std::to_string(routingCount));
    CHECK((int)tools.size() >= 54, "54+ total tools, got " + std::to_string(tools.size()));
    PASS();
}

// 9. routeTask updates item workerType
void test_route_updates_item() {
    TEST(route_updates_item);
    auto state = makeState();

    // Find getter item
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string getterId;
    for (const auto& item : ready) {
        if (item["nodeName"] == "getName") {
            getterId = item["id"].get<std::string>();
            break;
        }
    }
    CHECK(!getterId.empty(), "found getter");

    rpcCall(state, "routeTask", {{"itemId", getterId}});

    auto itemResp = rpcCall(state, "getWorkItem", {{"itemId", getterId}});
    CHECK(itemResp["result"]["workerType"] == "template",
          "workerType updated to template");
    PASS();
}

// 10. executeTask with no worker returns error
void test_execute_no_worker() {
    TEST(execute_no_worker);
    auto state = makeState();

    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string itemId = ready[0]["id"].get<std::string>();

    // Set to unknown worker type
    auto item = *state.workflow->queue.getItem(itemId);
    item.workerType = "quantum";
    state.workflow->queue.updateItem(itemId, item);

    auto resp = rpcCall(state, "executeTask", {{"itemId", itemId}});
    CHECK(resp.contains("error"), "error for unknown worker");
    PASS();
}

// 11. routeAllReady returns decisions with itemIds
void test_route_all_has_item_ids() {
    TEST(route_all_has_item_ids);
    auto state = makeState();
    auto resp = rpcCall(state, "routeAllReady");

    for (const auto& d : resp["result"]["decisions"]) {
        CHECK(d.contains("itemId"), "decision has itemId");
        CHECK(d.contains("workerType"), "decision has workerType");
    }
    PASS();
}

// 12. executeTask autoApproved for deterministic with high confidence
void test_auto_approved() {
    TEST(auto_approved);
    auto state = makeState();
    rpcCall(state, "routeAllReady");

    // Find deterministic (getter) item
    auto ready = rpcCall(state, "getReadyTasks")["result"]["items"];
    std::string getterId;
    for (const auto& item : ready) {
        if (item["workerType"] == "template") {
            getterId = item["id"].get<std::string>();
            break;
        }
    }
    CHECK(!getterId.empty(), "found template item");

    auto resp = rpcCall(state, "executeTask", {{"itemId", getterId}});
    CHECK(resp.contains("result"), "has result");
    // Template/deterministic with high confidence → autoApproved
    CHECK(resp["result"]["autoApproved"].get<bool>(), "auto approved");
    PASS();
}

int main() {
    std::cout << "=== Step 329: Routing RPC + MCP Tools ===\n";
    try {
        test_route_task();
        test_route_all_ready();
        test_execute_deterministic();
        test_agent_prepared();
        test_human_marked();
        test_routing_explanation();
        test_linter_restrictions();
        test_mcp_registration();
        test_route_updates_item();
        test_execute_no_worker();
        test_route_all_has_item_ids();
        test_auto_approved();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
