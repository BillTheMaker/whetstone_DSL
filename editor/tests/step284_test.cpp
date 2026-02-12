// Step 284: EnvironmentSpec Schema & AST Node (12 tests)
#include <cassert>
#include <iostream>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Serialization.h"
#include "EnvironmentSpec.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

// Test 1: EnvironmentSpec default construction
static void test_env_defaults() {
    EnvironmentSpec env;
    check(env.conceptType == "EnvironmentSpec", "conceptType is EnvironmentSpec");
    check(env.envId.empty(), "envId default empty");
    check(env.capabilities.empty(), "capabilities default empty");
    check(env.scheduler.empty(), "scheduler default empty");
    check(env.memory.empty(), "memory default empty");
    check(env.bindingTimes.is_object(), "bindingTimes default is json object");
}

// Test 2: EnvironmentSpec field assignment
static void test_env_fields() {
    EnvironmentSpec env;
    env.envId = "posix_process";
    env.envVersion = "4.18";
    env.capabilities = {"io.fs", "io.net", "threads"};
    env.constraints = {"no_jit"};
    env.scheduler = "threads";
    env.memory = "manual";
    env.exceptions = "unwind";
    env.ffi = "c_abi";
    env.bindingTimes = {{"types", "compile"}, {"dispatch", "runtime"}};

    check(env.envId == "posix_process", "envId set");
    check(env.capabilities.size() == 3, "3 capabilities");
    check(env.scheduler == "threads", "scheduler set");
    check(env.memory == "manual", "memory set");
    check(env.exceptions == "unwind", "exceptions set");
    check(env.ffi == "c_abi", "ffi set");
}

// Test 3: EnvironmentSpec JSON roundtrip via envSpecToJson/envSpecFromJson
static void test_env_json_roundtrip() {
    EnvironmentSpec env1;
    env1.envId = "browser";
    env1.envVersion = "es2022";
    env1.capabilities = {"ui.dom", "io.net", "event_loop"};
    env1.scheduler = "event_loop";
    env1.memory = "tracing_gc";
    env1.exceptions = "untyped";
    env1.ffi = "none";
    env1.bindingTimes = {{"modules", "load"}};

    json j = envSpecToJson(&env1);
    check(j["envId"] == "browser", "JSON envId");
    check(j["scheduler"] == "event_loop", "JSON scheduler");
    check(j["capabilities"].size() == 3, "JSON capabilities count");

    EnvironmentSpec env2;
    envSpecFromJson(&env2, j);
    check(env2.envId == "browser", "roundtrip envId");
    check(env2.scheduler == "event_loop", "roundtrip scheduler");
    check(env2.memory == "tracing_gc", "roundtrip memory");
    check(env2.capabilities.size() == 3, "roundtrip capabilities count");
}

// Test 4: EnvironmentSpec attachment to Module
static void test_env_attach_to_module() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "test";
    auto* env = new EnvironmentSpec();
    env->envId = "jvm";
    env->scheduler = "threads";
    mod->addChild("environment", env);

    auto envChildren = mod->getChildren("environment");
    check(envChildren.size() == 1, "module has 1 environment child");
    check(envChildren[0]->conceptType == "EnvironmentSpec", "child is EnvironmentSpec");
    auto* retrieved = static_cast<EnvironmentSpec*>(envChildren[0]);
    check(retrieved->envId == "jvm", "retrieved envId matches");
    delete mod;
}

// Test 5: EnvironmentSpec createNode
static void test_env_create_node() {
    ASTNode* node = createNode("EnvironmentSpec");
    check(node != nullptr, "createNode returns non-null");
    check(node->conceptType == "EnvironmentSpec", "created node type");
    delete node;
}

// Test 6: EnvironmentSpec via AST JSON roundtrip (toJson/fromJson)
static void test_env_ast_roundtrip() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "envtest";
    auto* env = new EnvironmentSpec();
    env->id = "env1";
    env->envId = "python_cpython";
    env->scheduler = "event_loop";
    env->memory = "tracing_gc";
    mod->addChild("environment", env);

    json j = toJson(mod);
    check(j["children"].contains("environment"), "JSON has environment children");
    check(j["children"]["environment"].size() == 1, "1 environment in JSON");

    ASTNode* restored = fromJson(j);
    check(restored != nullptr, "fromJson returns non-null");
    auto envKids = restored->getChildren("environment");
    check(envKids.size() == 1, "restored has 1 environment");
    check(envKids[0]->conceptType == "EnvironmentSpec", "restored type correct");
    delete mod;
    delete restored;
}

// Test 7: envSpecToJson omits empty fields
static void test_env_json_sparse() {
    EnvironmentSpec env;
    env.envId = "minimal";
    json j = envSpecToJson(&env);
    check(j.contains("envId"), "has envId");
    check(!j.contains("scheduler"), "no scheduler when empty");
    check(!j.contains("memory"), "no memory when empty");
    check(!j.contains("capabilities"), "no capabilities when empty");
}

// Test 8: envSpecFromJson handles missing keys gracefully
static void test_env_json_partial() {
    json j = {{"envId", "partial"}, {"scheduler", "fibers"}};
    EnvironmentSpec env;
    envSpecFromJson(&env, j);
    check(env.envId == "partial", "partial envId");
    check(env.scheduler == "fibers", "partial scheduler");
    check(env.memory.empty(), "missing memory stays empty");
    check(env.capabilities.empty(), "missing capabilities stays empty");
}

// Test 9: Multiple EnvironmentSpec values
static void test_env_values() {
    std::vector<std::string> schedulers = {"event_loop", "threads", "fibers", "coroutines", "single_thread"};
    std::vector<std::string> memories = {"manual", "raii", "refcount", "tracing_gc", "region"};
    int ok = 0;
    for (const auto& s : schedulers) {
        EnvironmentSpec env;
        env.scheduler = s;
        json j = envSpecToJson(&env);
        if (j["scheduler"] == s) ++ok;
    }
    for (const auto& m : memories) {
        EnvironmentSpec env;
        env.memory = m;
        json j = envSpecToJson(&env);
        if (j["memory"] == m) ++ok;
    }
    check(ok == 10, "all scheduler/memory values roundtrip");
}

// Test 10: Binding times JSON object
static void test_binding_times() {
    EnvironmentSpec env;
    env.envId = "bt_test";
    env.bindingTimes = {{"types", "compile"}, {"dispatch", "runtime"}, {"generics", "link"}};
    json j = envSpecToJson(&env);
    check(j["bindingTimes"].size() == 3, "3 binding times");
    check(j["bindingTimes"]["types"] == "compile", "types binding time");

    EnvironmentSpec env2;
    envSpecFromJson(&env2, j);
    check(env2.bindingTimes["dispatch"] == "runtime", "roundtrip dispatch binding");
}

// Test 11: envSpecToJson null safety
static void test_null_safety() {
    json j = envSpecToJson(nullptr);
    check(j.is_null(), "null env → null json");
    EnvironmentSpec env;
    envSpecFromJson(nullptr, {{"envId", "x"}});
    envSpecFromJson(&env, json());
    check(env.envId.empty(), "empty json → no change");
}

// Test 12: EnvironmentSpec properties via propertiesToJson
static void test_env_properties() {
    // EnvironmentSpec uses its own envSpecToJson, but propertiesToJson
    // should at least not crash (returns empty for unknown types handled by the EnvironmentSpec's own serialization)
    EnvironmentSpec env;
    env.envId = "test";
    // propertiesToJson won't have a specific case for EnvironmentSpec (uses envSpecToJson instead)
    // but it shouldn't crash
    json props = propertiesToJson(&env);
    // EnvironmentSpec is not in propertiesToJson dispatch, so props will be empty — that's OK
    // The test verifies no crash and that envSpecToJson works as the canonical path
    json envJ = envSpecToJson(&env);
    check(envJ["envId"] == "test", "envSpecToJson canonical path works");
}

int main() {
    std::cout << "=== Step 284: EnvironmentSpec Schema & AST Node ===\n";
    test_env_defaults();
    test_env_fields();
    test_env_json_roundtrip();
    test_env_attach_to_module();
    test_env_create_node();
    test_env_ast_roundtrip();
    test_env_json_sparse();
    test_env_json_partial();
    test_env_values();
    test_binding_times();
    test_null_safety();
    test_env_properties();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
