// Step 287: Environment-Aware Lowering Decisions (12 tests)
//
// Tests getLoweringHints — environment-aware code generation patterns
// based on scheduler, memory, and exception models. Also tests
// the getLoweringHints RPC method through HeadlessEditorState.
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
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

// Test 1: @Exec(async) + event_loop → callback pattern
static void test_async_event_loop() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "fetch";
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fn->addChild("annotations", ea);

    EnvironmentSpec env;
    env.scheduler = "event_loop";
    env.memory = "tracing_gc";
    env.exceptions = "untyped";

    auto hints = getLoweringHints(fn, &env);
    bool foundCallback = false;
    for (const auto& h : hints)
        if (h.pattern == "callback") foundCallback = true;
    check(foundCallback, "async + event_loop → callback pattern");
    delete fn;
}

// Test 2: @Exec(async) + threads → std_async pattern
static void test_async_threads() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "compute";
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fn->addChild("annotations", ea);

    EnvironmentSpec env;
    env.scheduler = "threads";
    env.memory = "manual";
    env.exceptions = "unwind";

    auto hints = getLoweringHints(fn, &env);
    bool foundStdAsync = false;
    for (const auto& h : hints)
        if (h.pattern == "std_async") foundStdAsync = true;
    check(foundStdAsync, "async + threads → std_async pattern");
    delete fn;
}

// Test 3: @Exec(async) + coroutines → coroutine pattern
static void test_async_coroutines() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "stream";
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fn->addChild("annotations", ea);

    EnvironmentSpec env;
    env.scheduler = "coroutines";
    env.memory = "raii";
    env.exceptions = "typed";

    auto hints = getLoweringHints(fn, &env);
    bool foundCoroutine = false;
    for (const auto& h : hints)
        if (h.pattern == "coroutine") foundCoroutine = true;
    check(foundCoroutine, "async + coroutines → coroutine pattern");
    delete fn;
}

// Test 4: tracing_gc → suppress_dealloc
static void test_gc_suppress_dealloc() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "alloc";

    EnvironmentSpec env;
    env.scheduler = "event_loop";
    env.memory = "tracing_gc";

    auto hints = getLoweringHints(fn, &env);
    bool found = false;
    for (const auto& h : hints)
        if (h.pattern == "suppress_dealloc") found = true;
    check(found, "tracing_gc → suppress_dealloc");
    delete fn;
}

// Test 5: manual memory → explicit_delete
static void test_manual_explicit_delete() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "alloc";

    EnvironmentSpec env;
    env.scheduler = "threads";
    env.memory = "manual";

    auto hints = getLoweringHints(fn, &env);
    bool found = false;
    for (const auto& h : hints)
        if (h.pattern == "explicit_delete") found = true;
    check(found, "manual → explicit_delete");
    delete fn;
}

// Test 6: RAII → raii_cleanup
static void test_raii_cleanup() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "resource";

    EnvironmentSpec env;
    env.scheduler = "threads";
    env.memory = "raii";

    auto hints = getLoweringHints(fn, &env);
    bool found = false;
    for (const auto& h : hints)
        if (h.pattern == "raii_cleanup") found = true;
    check(found, "raii → raii_cleanup");
    delete fn;
}

// Test 7: unwind exceptions → try_catch
static void test_unwind_try_catch() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "parse";

    EnvironmentSpec env;
    env.scheduler = "threads";
    env.memory = "manual";
    env.exceptions = "unwind";

    auto hints = getLoweringHints(fn, &env);
    bool found = false;
    for (const auto& h : hints)
        if (h.pattern == "try_catch") found = true;
    check(found, "unwind → try_catch");
    delete fn;
}

// Test 8: typed exceptions → expected
static void test_typed_expected() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "safe";

    EnvironmentSpec env;
    env.scheduler = "threads";
    env.memory = "raii";
    env.exceptions = "typed";

    auto hints = getLoweringHints(fn, &env);
    bool found = false;
    for (const auto& h : hints)
        if (h.pattern == "expected") found = true;
    check(found, "typed → expected (std::expected)");
    delete fn;
}

// Test 9: Combined hints — async + manual + unwind
static void test_combined_hints() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "combo";
    auto* ea = new ExecAnnotation(); ea->mode = "async";
    fn->addChild("annotations", ea);

    EnvironmentSpec env;
    env.scheduler = "threads";
    env.memory = "manual";
    env.exceptions = "unwind";

    auto hints = getLoweringHints(fn, &env);
    // Should get: std_async + explicit_delete + try_catch
    check(hints.size() >= 3, "at least 3 hints for combined env");

    bool hasAsync = false, hasDelete = false, hasTryCatch = false;
    for (const auto& h : hints) {
        if (h.pattern == "std_async") hasAsync = true;
        if (h.pattern == "explicit_delete") hasDelete = true;
        if (h.pattern == "try_catch") hasTryCatch = true;
    }
    check(hasAsync && hasDelete && hasTryCatch,
          "all 3 patterns present in combined hints");
    delete fn;
}

// Test 10: getLoweringHints RPC method
static void test_rpc_lowering_hints() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test.py",
        "def worker():\n    pass\n", "python");

    // Set environment
    json setReq = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "posix"}, {"scheduler", "threads"},
                    {"memory", "manual"}, {"exceptions", "unwind"}}}};
    state.processAgentRequest(setReq, "s1");

    // Add @Exec(async) to worker function
    Module* ast = state.activeAST();
    auto fns = ast->getChildren("functions");
    if (!fns.empty()) {
        auto* ea = new ExecAnnotation(); ea->mode = "async";
        fns[0]->addChild("annotations", ea);
    }

    // Get lowering hints for the function
    std::string fnId = fns.empty() ? "" : fns[0]->id;
    json hintReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "getLoweringHints"},
        {"params", {{"nodeId", fnId}}}};
    json resp = state.processAgentRequest(hintReq, "s1");
    check(resp.contains("result"), "getLoweringHints returns result");
    check(resp["result"]["hints"].is_array(), "hints is array");
    check(resp["result"]["count"].get<int>() >= 1, "at least 1 hint");

    // Should include std_async for threads+async
    bool foundStdAsync = false;
    for (const auto& h : resp["result"]["hints"]) {
        if (h["pattern"] == "std_async") foundStdAsync = true;
    }
    check(foundStdAsync, "RPC hints include std_async");
}

// Test 11: getLoweringHints with no nodeId → module-level hints
static void test_rpc_lowering_module_level() {
    HeadlessEditorState state;
    state.agent.defaultRole = AgentRole::Refactor;
    state.openBuffer("test.py", "x = 1\n", "python");

    json setReq = {{"jsonrpc", "2.0"}, {"id", 1},
        {"method", "setEnvironment"},
        {"params", {{"envId", "browser"}, {"scheduler", "event_loop"},
                    {"memory", "tracing_gc"}, {"exceptions", "untyped"}}}};
    state.processAgentRequest(setReq, "s1");

    // No nodeId → module-level
    json hintReq = {{"jsonrpc", "2.0"}, {"id", 2},
        {"method", "getLoweringHints"},
        {"params", json::object()}};
    json resp = state.processAgentRequest(hintReq, "s1");
    check(resp.contains("result"), "module-level hints returns result");
    // Should get suppress_dealloc from tracing_gc
    bool foundSuppressDealloc = false;
    for (const auto& h : resp["result"]["hints"]) {
        if (h["pattern"] == "suppress_dealloc") foundSuppressDealloc = true;
    }
    check(foundSuppressDealloc, "module-level includes suppress_dealloc");
}

// Test 12: null safety
static void test_null_safety() {
    auto hints1 = getLoweringHints(nullptr, nullptr);
    check(hints1.empty(), "null node + null env → empty");

    auto* fn = new Function(); fn->id = "f1";
    auto hints2 = getLoweringHints(fn, nullptr);
    check(hints2.empty(), "null env → empty");

    EnvironmentSpec env;
    auto hints3 = getLoweringHints(nullptr, &env);
    check(hints3.empty(), "null node → empty");
    delete fn;
}

int main() {
    std::cout << "=== Step 287: Environment-Aware Lowering Decisions ===\n";
    test_async_event_loop();
    test_async_threads();
    test_async_coroutines();
    test_gc_suppress_dealloc();
    test_manual_explicit_delete();
    test_raii_cleanup();
    test_unwind_try_catch();
    test_typed_expected();
    test_combined_hints();
    test_rpc_lowering_hints();
    test_rpc_lowering_module_level();
    test_null_safety();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
