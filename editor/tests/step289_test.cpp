// Step 289: Phase 10e Environment Layer Integration Tests (8 tests)
//
// End-to-end integration tests exercising the full environment layer:
// EnvironmentSpec, capability validation, annotation compatibility,
// lowering hints, host boundary nodes, and MCP tool counts.
#include <cassert>
#include <iostream>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/HostBoundary.h"
#include "ast/Serialization.h"
#include "EnvironmentSpec.h"
#include "ASTUtils.h"
#include "CompactAST.h"
#include "HeadlessEditorState.h"
#include "MCPServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

// Test 1: Full workflow — setEnvironment → add annotations → validate → getLoweringHints
static void test_full_env_workflow() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("app.py",
        "def fetch_data():\n    pass\n"
        "def process():\n    pass\n", "python");

    // 1. Set environment
    json setEnv = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "posix_server"}, {"scheduler", "threads"},
                    {"memory", "manual"}, {"exceptions", "unwind"},
                    {"capabilities", json::array({"io.fs", "io.net", "threads"})}}}};
    json envResp = state.processAgentRequest(setEnv, "s1");
    check(envResp["result"]["success"] == true, "environment set");

    // 2. Add @Exec(async) annotation to fetch_data
    Module* ast = state.activeAST();
    auto fns = ast->getChildren("functions");
    check(fns.size() == 2, "2 functions parsed");
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fns[0]->addChild("annotations", ea);

    // 3. Validate environment
    json valReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "validateEnvironment"}};
    json valResp = state.processAgentRequest(valReq, "s1");
    check(valResp["result"]["count"].get<int>() == 0,
          "0 diagnostics (threads supports async)");

    // 4. Get lowering hints for fetch_data
    json hintReq = {{"jsonrpc", "2.0"}, {"id", 3},
        {"method", "getLoweringHints"},
        {"params", {{"nodeId", fns[0]->id}}}};
    json hintResp = state.processAgentRequest(hintReq, "s1");
    check(hintResp["result"]["count"].get<int>() >= 3,
          "multiple hints (std_async + explicit_delete + try_catch)");
}

// Test 2: Incompatible env detects issues and changing env clears them
static void test_env_switch_clears_issues() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("ui.py",
        "def render():\n    pass\n", "python");

    // Set single_thread env
    json setEnv1 = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "embedded"}, {"scheduler", "single_thread"},
                    {"memory", "manual"}}}};
    state.processAgentRequest(setEnv1, "s1");

    // Add @Parallel (incompatible with single_thread)
    Module* ast = state.activeAST();
    auto fns = ast->getChildren("functions");
    auto* pa = new ParallelAnnotation(); pa->kind = "data";
    fns[0]->addChild("annotations", pa);

    // Validate — should fail
    json valReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "validateEnvironment"}};
    json val1 = state.processAgentRequest(valReq, "s1");
    check(val1["result"]["count"].get<int>() >= 1,
          "at least 1 diagnostic in single_thread");

    // Switch to threads env
    json setEnv2 = {{"jsonrpc", "2.0"}, {"id", 3},
        {"method", "setEnvironment"},
        {"params", {{"envId", "server"}, {"scheduler", "threads"},
                    {"memory", "manual"}}}};
    state.processAgentRequest(setEnv2, "s1");

    // Validate again — should pass
    json val2 = state.processAgentRequest(valReq, "s1");
    check(val2["result"]["count"].get<int>() == 0,
          "0 diagnostics after switching to threads");
}

// Test 3: Host boundary nodes in a function body survive AST roundtrip
static void test_host_boundary_roundtrip() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "server";
    auto* fn = new Function(); fn->id = "f1"; fn->name = "handler";
    auto* hc = new HostCall("io.net", "accept");
    hc->id = "hc1";
    fn->addChild("body", hc);
    auto* st = new ScheduleTask();
    st->id = "st1"; st->queue = "thread";
    fn->addChild("body", st);
    auto* ml = new ModuleLoad();
    ml->id = "ml1"; ml->moduleName = "crypto"; ml->mode = "dynamic";
    fn->addChild("body", ml);
    mod->addChild("functions", fn);

    // Full AST roundtrip
    json j = toJson(mod);
    ASTNode* restored = fromJson(j);
    auto rFns = restored->getChildren("functions");
    check(rFns.size() == 1, "1 function restored");
    auto body = rFns[0]->getChildren("body");
    check(body.size() == 3, "3 body nodes restored");
    check(body[0]->conceptType == "HostCall", "body[0] is HostCall");
    check(body[1]->conceptType == "ScheduleTask", "body[1] is ScheduleTask");
    check(body[2]->conceptType == "ModuleLoad", "body[2] is ModuleLoad");
    delete mod;
    delete restored;
}

// Test 4: CapabilityRequirement + EnvironmentSpec validation end-to-end
static void test_capability_validation_e2e() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("net.py",
        "def serve():\n    pass\n", "python");

    // Set env without io.net
    json setEnv = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "no_net"}, {"scheduler", "threads"},
                    {"capabilities", json::array({"io.fs"})}}}};
    state.processAgentRequest(setEnv, "s1");

    // Add CapabilityRequirement for io.net
    Module* ast = state.activeAST();
    auto fns = ast->getChildren("functions");
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net"; cr->required = true;
    fns[0]->addChild("annotations", cr);

    // Validate
    json valReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "validateEnvironment"}};
    json resp = state.processAgentRequest(valReq, "s1");
    bool foundE0501 = false;
    for (const auto& d : resp["result"]["diagnostics"]) {
        if (d["message"].get<std::string>().find("E0501") != std::string::npos)
            foundE0501 = true;
    }
    check(foundE0501, "E0501 for unmet io.net capability");
}

// Test 5: Lowering hints change when environment changes
static void test_lowering_hints_change_with_env() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("async.py",
        "def fetch():\n    pass\n", "python");

    Module* ast = state.activeAST();
    auto fns = ast->getChildren("functions");
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fns[0]->addChild("annotations", ea);

    // Set event_loop env
    json setEnv1 = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "browser"}, {"scheduler", "event_loop"},
                    {"memory", "tracing_gc"}}}};
    state.processAgentRequest(setEnv1, "s1");

    json hintReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "getLoweringHints"},
        {"params", {{"nodeId", fns[0]->id}}}};
    json hints1 = state.processAgentRequest(hintReq, "s1");

    bool hasCallback = false;
    for (const auto& h : hints1["result"]["hints"])
        if (h["pattern"] == "callback") hasCallback = true;
    check(hasCallback, "event_loop env → callback pattern");

    // Switch to threads env
    json setEnv2 = {{"jsonrpc", "2.0"}, {"id", 3},
        {"method", "setEnvironment"},
        {"params", {{"envId", "server"}, {"scheduler", "threads"},
                    {"memory", "manual"}}}};
    state.processAgentRequest(setEnv2, "s1");

    json hints2 = state.processAgentRequest(hintReq, "s1");
    bool hasStdAsync = false;
    for (const auto& h : hints2["result"]["hints"])
        if (h["pattern"] == "std_async") hasStdAsync = true;
    check(hasStdAsync, "threads env → std_async pattern");
}

// Test 6: Batch query with environment methods
static void test_batch_env_queries() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("batch.py",
        "def work():\n    pass\n", "python");

    // Set env
    json setEnv = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "jvm"}, {"scheduler", "threads"},
                    {"memory", "tracing_gc"}, {"exceptions", "unwind"}}}};
    state.processAgentRequest(setEnv, "s1");

    // Batch query: getEnvironment + validateEnvironment + getLoweringHints
    json batchReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "batchQuery"},
        {"params", {{"queries", json::array({
            {{"method", "getEnvironment"}},
            {{"method", "validateEnvironment"}},
            {{"method", "getLoweringHints"}, {"params", json::object()}}
        })}}}};
    json resp = state.processAgentRequest(batchReq, "s1");
    check(resp.contains("result"), "batchQuery returns result");
    auto results = resp["result"]["results"];
    check(results.is_array(), "results is array");
    check(results.size() == 3, "3 sub-query results");

    // Check getEnvironment result
    check(results[0]["result"]["envId"] == "jvm",
          "batch[0] getEnvironment returns jvm");
    // Check validateEnvironment result
    check(results[1]["result"].contains("diagnostics"),
          "batch[1] validateEnvironment has diagnostics");
    // Check getLoweringHints result
    check(results[2]["result"].contains("hints"),
          "batch[2] getLoweringHints has hints");
}

// Test 7: MCP tool count includes environment tools
static void test_mcp_tool_count() {
    MCPServer mcp;
    const auto& tools = mcp.getTools();
    // Previous: 34 tools from Sprint 9 + sidecar + annotation
    // + 4 new environment tools = 38
    int envToolCount = 0;
    for (const auto& t : tools) {
        if (t.name.find("environment") != std::string::npos ||
            t.name.find("lowering") != std::string::npos)
            envToolCount++;
    }
    check(envToolCount == 4, "4 environment MCP tools registered");
    check((int)tools.size() >= 38, "at least 38 total MCP tools");
}

// Test 8: EnvironmentSpec compact AST integration
static void test_env_compact_ast() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "compact_test";
    auto* env = new EnvironmentSpec();
    env->id = "env1"; env->envId = "posix";
    env->scheduler = "threads"; env->memory = "raii";
    mod->addChild("environment", env);

    auto* fn = new Function(); fn->id = "f1"; fn->name = "worker";
    auto* hc = new HostCall("io.fs", "readFile");
    hc->id = "hc1";
    fn->addChild("body", hc);
    mod->addChild("functions", fn);

    // Compact AST should include the host boundary node
    json compact = toJsonCompactTree(mod);
    check(compact.is_array(), "compact tree is array");
    check(compact.size() >= 3, "at least 3 nodes (mod + fn + hostcall)");

    // Find the HostCall in compact output
    bool foundHostCall = false;
    for (const auto& node : compact) {
        if (node.value("type", "") == "HostCall") {
            foundHostCall = true;
            check(node.value("name", "") == "readFile",
                  "compact HostCall name is readFile");
        }
    }
    check(foundHostCall, "HostCall found in compact AST");
    delete mod;
}

int main() {
    std::cout << "=== Step 289: Phase 10e Integration Tests ===\n";
    test_full_env_workflow();
    test_env_switch_clears_issues();
    test_host_boundary_roundtrip();
    test_capability_validation_e2e();
    test_lowering_hints_change_with_env();
    test_batch_env_queries();
    test_mcp_tool_count();
    test_env_compact_ast();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
