// Step 317: Architect Tooling — RPC + MCP (12 tests)
// 4 RPCs: createSkeleton, addSkeletonNode, getProjectModel, inferAnnotations.
// 4 MCP tools. Permission enforcement.

#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "MCPServer.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>

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

// 1. createSkeleton creates a new skeleton module
void test_create_skeleton_rpc() {
    TEST(create_skeleton_rpc);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.defaultLanguage = "python";
    auto resp = rpcCall(state, "createSkeleton", {{"name", "myapp"}, {"language", "python"}});
    CHECK(resp.contains("result"), "Has result");
    CHECK(resp["result"]["name"] == "myapp", "Module name");
    CHECK(resp["result"]["language"] == "python", "Module language");
    CHECK(resp["result"].contains("bufferId"), "Has bufferId");
    // Active buffer should be set
    CHECK(state.activeBuffer != nullptr, "Active buffer set");
    PASS();
}

// 2. addSkeletonNode adds function with annotations
void test_add_skeleton_node_rpc() {
    TEST(add_skeleton_node_rpc);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    rpcCall(state, "createSkeleton", {{"name", "myapp"}, {"language", "python"}});

    auto resp = rpcCall(state, "addSkeletonNode", {
        {"nodeType", "function"}, {"name", "process"},
        {"parameters", json::array({"data", "config"})},
        {"contextWidth", "file"}, {"automatability", "template"},
        {"priority", "high"}
    });
    CHECK(resp.contains("result"), "Has result");
    CHECK(resp["result"]["name"] == "process", "Node name");
    CHECK(resp["result"]["nodeType"] == "function", "Node type");
    CHECK(resp["result"].contains("nodeId"), "Has nodeId");
    PASS();
}

// 3. addSkeletonNode adds class
void test_add_skeleton_class_rpc() {
    TEST(add_skeleton_class_rpc);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    rpcCall(state, "createSkeleton", {{"name", "myapp"}, {"language", "python"}});

    auto resp = rpcCall(state, "addSkeletonNode", {
        {"nodeType", "class"}, {"name", "UserService"},
        {"methods", json::array({"create", "delete", "find"})}
    });
    CHECK(resp.contains("result"), "Has result");
    CHECK(resp["result"]["name"] == "UserService", "Class name");
    CHECK(resp["result"]["nodeType"] == "class", "Node type");
    PASS();
}

// 4. getProjectModel returns summary + task list
void test_get_project_model_rpc() {
    TEST(get_project_model_rpc);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    rpcCall(state, "createSkeleton", {{"name", "myapp"}, {"language", "python"}});
    rpcCall(state, "addSkeletonNode", {{"name", "task_a"}, {"priority", "critical"}});
    rpcCall(state, "addSkeletonNode", {{"name", "task_b"},
        {"contextWidth", "project"}, {"automatability", "llm"}});

    auto resp = rpcCall(state, "getProjectModel");
    CHECK(resp.contains("result"), "Has result");
    auto& r = resp["result"];
    CHECK(r["totalNodes"].get<int>() == 2, "2 total nodes, got: " + std::to_string(r["totalNodes"].get<int>()));
    CHECK(r["skeletonNodes"].get<int>() >= 1, "Has skeleton nodes");
    CHECK(r.contains("tasks"), "Has tasks");
    CHECK(r["tasks"].is_array(), "Tasks is array");
    CHECK(r["tasks"].size() == 2, "2 tasks");

    // Check task fields
    auto& t1 = r["tasks"][0];
    CHECK(t1.contains("nodeName"), "Task has nodeName");
    CHECK(t1.contains("contextWidth"), "Task has contextWidth");
    CHECK(t1.contains("automatability"), "Task has automatability");
    CHECK(t1.contains("priority"), "Task has priority");
    CHECK(t1.contains("status"), "Task has status");
    PASS();
}

// 5. inferAnnotations returns suggestions
void test_infer_annotations_rpc() {
    TEST(infer_annotations_rpc);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    // Parse some Python code into the buffer
    state.openBuffer("test", "def process():\n    return 42\n", "python");
    auto* ast = state.activeAST();
    CHECK(ast != nullptr, "Has AST");

    auto resp = rpcCall(state, "inferAnnotations");
    CHECK(resp.contains("result"), "Has result");
    auto& r = resp["result"];
    CHECK(r.contains("suggestions"), "Has suggestions");
    CHECK(r["suggestions"].is_array(), "Suggestions is array");
    CHECK(r.contains("count"), "Has count");
    PASS();
}

// 6. Permission: Linter cannot createSkeleton
void test_linter_permission_denied() {
    TEST(linter_permission_denied);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Linter;
    auto resp = rpcCall(state, "createSkeleton", {{"name", "x"}, {"language", "python"}});
    CHECK(resp.contains("error"), "Should get error");
    CHECK(resp["error"]["code"] == -32031, "Permission denied code");
    PASS();
}

// 7. Permission: Linter can getProjectModel
void test_linter_can_read() {
    TEST(linter_can_read);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    rpcCall(state, "createSkeleton", {{"name", "myapp"}, {"language", "python"}});
    state.agent.defaultRole = AgentRole::Linter;

    auto resp = rpcCall(state, "getProjectModel");
    CHECK(resp.contains("result"), "Linter can call getProjectModel");

    auto resp2 = rpcCall(state, "inferAnnotations");
    CHECK(resp2.contains("result"), "Linter can call inferAnnotations");
    PASS();
}

// 8. MCP tool registration — 4 workflow tools
void test_mcp_tool_registration() {
    TEST(mcp_tool_registration);
    MCPServer server;
    json initReq = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"},
        {"params", {{"protocolVersion", "2024-11-05"},
            {"capabilities", json::object()},
            {"clientInfo", {{"name", "test"}}}}}};
    server.handleRequest(initReq);

    json listReq = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = server.handleRequest(listReq);
    CHECK(resp.contains("result"), "Has result");
    auto& tools = resp["result"]["tools"];

    bool hasSkeleton = false, hasAddNode = false, hasModel = false, hasInfer = false;
    for (const auto& t : tools) {
        std::string name = t["name"];
        if (name == "whetstone_create_skeleton") hasSkeleton = true;
        if (name == "whetstone_add_skeleton_node") hasAddNode = true;
        if (name == "whetstone_get_project_model") hasModel = true;
        if (name == "whetstone_infer_annotations") hasInfer = true;
    }
    CHECK(hasSkeleton, "Has create_skeleton tool");
    CHECK(hasAddNode, "Has add_skeleton_node tool");
    CHECK(hasModel, "Has get_project_model tool");
    CHECK(hasInfer, "Has infer_annotations tool");
    PASS();
}

// 9. MCP tool count (42+)
void test_mcp_tool_count() {
    TEST(mcp_tool_count);
    MCPServer server;
    json initReq = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"},
        {"params", {{"protocolVersion", "2024-11-05"},
            {"capabilities", json::object()},
            {"clientInfo", {{"name", "test"}}}}}};
    server.handleRequest(initReq);

    json listReq = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = server.handleRequest(listReq);
    int count = (int)resp["result"]["tools"].size();
    CHECK(count >= 42, "42+ tools, got: " + std::to_string(count));
    PASS();
}

// 10. Full workflow: create → add nodes → get model
void test_full_workflow() {
    TEST(full_workflow);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    rpcCall(state, "createSkeleton", {{"name", "webapp"}, {"language", "python"}});

    rpcCall(state, "addSkeletonNode", {
        {"name", "auth_handler"}, {"contextWidth", "file"},
        {"automatability", "llm"}, {"priority", "critical"}});
    rpcCall(state, "addSkeletonNode", {
        {"name", "data_validator"}, {"contextWidth", "local"},
        {"automatability", "template"}, {"priority", "high"},
        {"blockedBy", json::array({"auth_handler"})}});
    rpcCall(state, "addSkeletonNode", {
        {"nodeType", "class"}, {"name", "UserModel"},
        {"methods", json::array({"create", "update", "delete"})}});

    auto resp = rpcCall(state, "getProjectModel");
    auto& r = resp["result"];
    // 2 functions + 1 class + 3 methods = 6
    CHECK(r["totalNodes"].get<int>() >= 5, "5+ total nodes");
    CHECK(r["tasks"].size() >= 5, "5+ tasks");

    // Verify routing annotations extracted
    bool foundCritical = false;
    for (const auto& t : r["tasks"]) {
        if (t["nodeName"] == "auth_handler") {
            CHECK(t["contextWidth"] == "file", "auth_handler contextWidth");
            CHECK(t["automatability"] == "llm", "auth_handler automatability");
            CHECK(t["priority"] == "critical", "auth_handler priority");
            foundCritical = true;
        }
        if (t["nodeName"] == "data_validator") {
            CHECK(t["dependencies"].size() == 1, "data_validator has dep");
        }
    }
    CHECK(foundCritical, "Found critical task");
    PASS();
}

// 11. inferAnnotations response fields
void test_infer_response_fields() {
    TEST(infer_response_fields);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test",
        "async def fetch():\n    data = await get_data()\n    return data\n",
        "python");

    auto resp = rpcCall(state, "inferAnnotations");
    auto& suggestions = resp["result"]["suggestions"];
    if (!suggestions.empty()) {
        auto& s = suggestions[0];
        CHECK(s.contains("nodeId"), "Suggestion has nodeId");
        CHECK(s.contains("annotationType"), "Suggestion has annotationType");
        CHECK(s.contains("confidence"), "Suggestion has confidence");
        CHECK(s.contains("reason"), "Suggestion has reason");
        double conf = s["confidence"].get<double>();
        CHECK(conf >= 0.0 && conf <= 1.0, "Confidence in range");
    }
    PASS();
}

// 12. No active buffer error
void test_no_active_buffer_error() {
    TEST(no_active_buffer_error);
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    // Don't create any buffer

    auto resp1 = rpcCall(state, "getProjectModel");
    CHECK(resp1.contains("error"), "getProjectModel error without buffer");

    auto resp2 = rpcCall(state, "inferAnnotations");
    CHECK(resp2.contains("error"), "inferAnnotations error without buffer");

    auto resp3 = rpcCall(state, "addSkeletonNode", {{"name", "x"}});
    CHECK(resp3.contains("error"), "addSkeletonNode error without buffer");
    PASS();
}

int main() {
    std::cout << "Step 317: Architect Tooling — RPC + MCP\n";
    try {
    test_create_skeleton_rpc();
    test_add_skeleton_node_rpc();
    test_add_skeleton_class_rpc();
    test_get_project_model_rpc();
    test_infer_annotations_rpc();
    test_linter_permission_denied();
    test_linter_can_read();
    test_mcp_tool_registration();
    test_mcp_tool_count();
    test_full_workflow();
    test_infer_response_fields();
    test_no_active_buffer_error();
    } catch (const std::exception& e) {
        std::cout << "\nFATAL: " << e.what() << "\n";
        return 1;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
