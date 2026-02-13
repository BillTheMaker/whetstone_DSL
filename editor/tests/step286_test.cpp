// Step 286: Environment-Aware Pipeline Hooks (12 tests)
//
// Tests that validateEnvAnnotations detects incompatible annotations,
// that the RPC validateEnvironment method returns combined diagnostics,
// and that MCP environment tools are properly registered.
#include <cassert>
#include <iostream>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "EnvironmentSpec.h"
#include "HeadlessEditorState.h"
#include "MCPServer.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

// Helper: build a module with env + annotated function
static Module* makeEnvModule(const std::string& scheduler,
                              const std::string& memory) {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "envtest";
    auto* env = new EnvironmentSpec();
    env->envId = "test_env";
    env->scheduler = scheduler;
    env->memory = memory;
    mod->addChild("environment", env);
    return mod;
}

// Test 1: @Parallel in single_thread → E0502
static void test_parallel_single_thread() {
    auto* mod = makeEnvModule("single_thread", "manual");
    auto* fn = new Function(); fn->id = "f1"; fn->name = "doWork";
    auto* pa = new ParallelAnnotation();
    pa->kind = "data";
    fn->addChild("annotations", pa);
    mod->addChild("functions", fn);

    auto* env = static_cast<EnvironmentSpec*>(mod->getChildren("environment")[0]);
    auto diags = validateEnvAnnotations(mod, env);
    check(diags.size() == 1, "1 diagnostic for @Parallel in single_thread");
    check(diags[0].message.find("E0502") != std::string::npos, "E0502 error code");
    check(diags[0].severity == "error", "severity is error");
    delete mod;
}

// Test 2: @ThreadModel in single_thread → E0503
static void test_threadmodel_single_thread() {
    auto* mod = makeEnvModule("single_thread", "manual");
    auto* fn = new Function(); fn->id = "f1"; fn->name = "spawn";
    auto* tm = new ThreadModelAnnotation();
    tm->model = "os";
    fn->addChild("annotations", tm);
    mod->addChild("functions", fn);

    auto* env = static_cast<EnvironmentSpec*>(mod->getChildren("environment")[0]);
    auto diags = validateEnvAnnotations(mod, env);
    check(diags.size() == 1, "1 diagnostic for @ThreadModel in single_thread");
    check(diags[0].message.find("E0503") != std::string::npos, "E0503 error code");
    delete mod;
}

// Test 3: @Atomic in single_thread → E0504 warning
static void test_atomic_single_thread() {
    auto* mod = makeEnvModule("single_thread", "manual");
    auto* fn = new Function(); fn->id = "f1"; fn->name = "inc";
    auto* at = new AtomicAnnotation();
    at->consistency = "seq_cst";
    fn->addChild("annotations", at);
    mod->addChild("functions", fn);

    auto* env = static_cast<EnvironmentSpec*>(mod->getChildren("environment")[0]);
    auto diags = validateEnvAnnotations(mod, env);
    check(diags.size() == 1, "1 diagnostic for @Atomic in single_thread");
    check(diags[0].severity == "warning", "severity is warning (not error)");
    check(diags[0].message.find("E0504") != std::string::npos, "E0504 code");
    delete mod;
}

// Test 4: @Exec(async) in single_thread → E0505
static void test_exec_async_single_thread() {
    auto* mod = makeEnvModule("single_thread", "manual");
    auto* fn = new Function(); fn->id = "f1"; fn->name = "fetch";
    auto* ea = new ExecAnnotation();
    ea->mode = "async";
    fn->addChild("annotations", ea);
    mod->addChild("functions", fn);

    auto* env = static_cast<EnvironmentSpec*>(mod->getChildren("environment")[0]);
    auto diags = validateEnvAnnotations(mod, env);
    check(diags.size() == 1, "1 diagnostic for @Exec(async) in single_thread");
    check(diags[0].message.find("E0505") != std::string::npos, "E0505 code");
    delete mod;
}

// Test 5: No diagnostics when scheduler is compatible
static void test_compatible_scheduler() {
    auto* mod = makeEnvModule("threads", "manual");
    auto* fn = new Function(); fn->id = "f1"; fn->name = "doWork";
    auto* pa = new ParallelAnnotation();
    pa->kind = "data";
    fn->addChild("annotations", pa);
    auto* ea = new ExecAnnotation();
    ea->mode = "async";
    fn->addChild("annotations", ea);
    mod->addChild("functions", fn);

    auto* env = static_cast<EnvironmentSpec*>(mod->getChildren("environment")[0]);
    auto diags = validateEnvAnnotations(mod, env);
    check(diags.empty(), "no diagnostics with threads scheduler");
    delete mod;
}

// Test 6: Multiple incompatible annotations → multiple diagnostics
static void test_multiple_incompatible() {
    auto* mod = makeEnvModule("single_thread", "manual");
    auto* fn = new Function(); fn->id = "f1"; fn->name = "multi";
    auto* pa = new ParallelAnnotation(); pa->kind = "task";
    fn->addChild("annotations", pa);
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fn->addChild("annotations", ea);
    auto* at = new AtomicAnnotation(); at->consistency = "relaxed";
    fn->addChild("annotations", at);
    mod->addChild("functions", fn);

    auto* env = static_cast<EnvironmentSpec*>(mod->getChildren("environment")[0]);
    auto diags = validateEnvAnnotations(mod, env);
    check(diags.size() == 3, "3 diagnostics for 3 incompatible annotations");
    delete mod;
}

// Test 7: validateEnvironment RPC combines cap + annotation diagnostics
static void test_rpc_validate_environment() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test.py",
        "def worker():\n    pass\n", "python");

    // Set environment
    json setReq = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "single_thread_env"},
                    {"scheduler", "single_thread"},
                    {"memory", "manual"},
                    {"capabilities", json::array({"io.fs"})}}}};
    state.processAgentRequest(setReq, "s1");

    // Add @Parallel annotation via mutation
    Module* ast = state.activeAST();
    auto fns = ast->getChildren("functions");
    if (!fns.empty()) {
        auto* pa = new ParallelAnnotation(); pa->kind = "data";
        fns[0]->addChild("annotations", pa);
    }

    // Validate
    json valReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "validateEnvironment"}};
    json resp = state.processAgentRequest(valReq, "s1");
    check(resp.contains("result"), "validateEnvironment returns result");
    auto diagsArr = resp["result"]["diagnostics"];
    check(diagsArr.is_array(), "diagnostics is array");
    check(resp["result"]["count"].get<int>() >= 1,
          "at least 1 diagnostic");
    // Should have E0502 for @Parallel in single_thread
    bool foundE0502 = false;
    for (const auto& d : diagsArr) {
        if (d["message"].get<std::string>().find("E0502") != std::string::npos)
            foundE0502 = true;
    }
    check(foundE0502, "E0502 found in validateEnvironment response");
}

// Test 8: setEnvironment + getEnvironment RPC roundtrip
static void test_rpc_set_get_environment() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test.py", "x = 1\n", "python");

    json setReq = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "browser"}, {"scheduler", "event_loop"},
                    {"memory", "tracing_gc"}, {"exceptions", "untyped"}}}};
    json setResp = state.processAgentRequest(setReq, "s1");
    check(setResp["result"]["success"] == true, "setEnvironment success");
    check(setResp["result"]["envId"] == "browser", "envId in response");

    json getReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "getEnvironment"}};
    json getResp = state.processAgentRequest(getReq, "s1");
    check(getResp["result"]["hasEnvironment"] == true, "hasEnvironment true");
    check(getResp["result"]["envId"] == "browser", "retrieved envId");
    check(getResp["result"]["scheduler"] == "event_loop", "scheduler matches");
}

// Test 9: getEnvironment with no env set
static void test_rpc_get_no_environment() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test.py", "x = 1\n", "python");

    json req = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "getEnvironment"}};
    json resp = state.processAgentRequest(req, "s1");
    check(resp["result"]["hasEnvironment"] == false, "hasEnvironment false when no env");
}

// Test 10: Linter role can read but not write environment
static void test_linter_permission() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Linter;
    state.openBuffer("test.py", "x = 1\n", "python");

    // Linter cannot setEnvironment
    json setReq = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "test"}}}};
    json setResp = state.processAgentRequest(setReq, "s1");
    check(setResp.contains("error"), "Linter denied setEnvironment");

    // Linter can getEnvironment
    json getReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "getEnvironment"}};
    json getResp = state.processAgentRequest(getReq, "s1");
    check(!getResp.contains("error") || getResp.contains("result"),
          "Linter can getEnvironment");
}

// Test 11: MCP environment tools registered
static void test_mcp_env_tools() {
    MCPServer mcp;
    const auto& tools = mcp.getTools();
    bool foundSetEnv = false, foundGetEnv = false,
         foundValidate = false, foundLowering = false;
    for (const auto& t : tools) {
        if (t.name == "whetstone_set_environment") foundSetEnv = true;
        if (t.name == "whetstone_get_environment") foundGetEnv = true;
        if (t.name == "whetstone_validate_environment") foundValidate = true;
        if (t.name == "whetstone_get_lowering_hints") foundLowering = true;
    }
    check(foundSetEnv, "whetstone_set_environment registered");
    check(foundGetEnv, "whetstone_get_environment registered");
    check(foundValidate, "whetstone_validate_environment registered");
    check(foundLowering, "whetstone_get_lowering_hints registered");
}

// Test 12: setEnvironment replaces existing env
static void test_replace_environment() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test.py", "x = 1\n", "python");

    // Set first env
    json set1 = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "posix"}, {"scheduler", "threads"}}}};
    state.processAgentRequest(set1, "s1");

    // Replace with second env
    json set2 = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "setEnvironment"},
        {"params", {{"envId", "browser"}, {"scheduler", "event_loop"}}}};
    state.processAgentRequest(set2, "s1");

    // Verify only the new one exists
    json getReq = {{"jsonrpc", "2.0"}, {"id", 3},
        {"method", "getEnvironment"}};
    json resp = state.processAgentRequest(getReq, "s1");
    check(resp["result"]["envId"] == "browser", "env replaced to browser");
    check(resp["result"]["scheduler"] == "event_loop",
          "scheduler replaced to event_loop");

    // Verify only 1 environment child
    Module* ast = state.activeAST();
    check(ast->getChildren("environment").size() == 1,
          "only 1 EnvironmentSpec after replacement");
}

int main() {
    std::cout << "=== Step 286: Environment-Aware Pipeline Hooks ===\n";
    test_parallel_single_thread();
    test_threadmodel_single_thread();
    test_atomic_single_thread();
    test_exec_async_single_thread();
    test_compatible_scheduler();
    test_multiple_incompatible();
    test_rpc_validate_environment();
    test_rpc_set_get_environment();
    test_rpc_get_no_environment();
    test_linter_permission();
    test_mcp_env_tools();
    test_replace_environment();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
