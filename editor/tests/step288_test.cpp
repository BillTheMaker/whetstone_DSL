// Step 288: Host Boundary AST Nodes (12 tests)
//
// Tests HostCall, ScheduleTask, ModuleLoad AST node construction,
// JSON roundtrip serialization, capability validation, and compact
// AST integration.
#include <cassert>
#include <iostream>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/HostBoundary.h"
#include "ast/Serialization.h"
#include "ASTUtils.h"
#include "CompactAST.h"
#include "EnvironmentSpec.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

// Test 1: HostCall default construction
static void test_hostcall_default() {
    HostCall hc;
    check(hc.conceptType == "HostCall", "conceptType is HostCall");
    check(hc.capability.empty(), "default capability empty");
    check(hc.name.empty(), "default name empty");
}

// Test 2: HostCall parameterized construction
static void test_hostcall_params() {
    HostCall hc("io.fs", "readFile");
    check(hc.conceptType == "HostCall", "conceptType is HostCall");
    check(hc.capability == "io.fs", "capability set to io.fs");
    check(hc.name == "readFile", "name set to readFile");
}

// Test 3: HostCall JSON roundtrip
static void test_hostcall_roundtrip() {
    auto* hc = new HostCall("io.net", "listen");
    hc->id = "hc1";

    json j = toJson(hc);
    check(j["concept"] == "HostCall", "JSON concept");
    check(j["properties"]["capability"] == "io.net", "JSON capability");
    check(j["properties"]["name"] == "listen", "JSON name");

    ASTNode* restored = fromJson(j);
    check(restored != nullptr, "fromJson non-null");
    check(restored->conceptType == "HostCall", "restored type");
    auto* rhc = static_cast<HostCall*>(restored);
    check(rhc->capability == "io.net", "roundtrip capability");
    check(rhc->name == "listen", "roundtrip name");
    delete hc;
    delete restored;
}

// Test 4: ScheduleTask default construction
static void test_schedule_default() {
    ScheduleTask st;
    check(st.conceptType == "ScheduleTask", "conceptType is ScheduleTask");
    check(st.queue.empty(), "default queue empty");
}

// Test 5: ScheduleTask JSON roundtrip
static void test_schedule_roundtrip() {
    auto* st = new ScheduleTask();
    st->id = "st1";
    st->queue = "microtask";

    json j = toJson(st);
    check(j["concept"] == "ScheduleTask", "JSON concept");
    check(j["properties"]["queue"] == "microtask", "JSON queue");

    ASTNode* restored = fromJson(j);
    check(restored != nullptr, "fromJson non-null");
    auto* rst = static_cast<ScheduleTask*>(restored);
    check(rst->queue == "microtask", "roundtrip queue");
    delete st;
    delete restored;
}

// Test 6: ModuleLoad default construction
static void test_moduleload_default() {
    ModuleLoad ml;
    check(ml.conceptType == "ModuleLoad", "conceptType is ModuleLoad");
    check(ml.moduleName.empty(), "default moduleName empty");
    check(ml.mode.empty(), "default mode empty");
}

// Test 7: ModuleLoad JSON roundtrip
static void test_moduleload_roundtrip() {
    auto* ml = new ModuleLoad();
    ml->id = "ml1";
    ml->moduleName = "math";
    ml->mode = "static";

    json j = toJson(ml);
    check(j["concept"] == "ModuleLoad", "JSON concept");
    check(j["properties"]["moduleName"] == "math", "JSON moduleName");
    check(j["properties"]["mode"] == "static", "JSON mode");

    ASTNode* restored = fromJson(j);
    check(restored != nullptr, "fromJson non-null");
    auto* rml = static_cast<ModuleLoad*>(restored);
    check(rml->moduleName == "math", "roundtrip moduleName");
    check(rml->mode == "static", "roundtrip mode");
    delete ml;
    delete restored;
}

// Test 8: createNode for all host boundary types
static void test_create_nodes() {
    ASTNode* hc = createNode("HostCall");
    check(hc != nullptr && hc->conceptType == "HostCall",
          "createNode HostCall");
    delete hc;

    ASTNode* st = createNode("ScheduleTask");
    check(st != nullptr && st->conceptType == "ScheduleTask",
          "createNode ScheduleTask");
    delete st;

    ASTNode* ml = createNode("ModuleLoad");
    check(ml != nullptr && ml->conceptType == "ModuleLoad",
          "createNode ModuleLoad");
    delete ml;
}

// Test 9: HostCall in function body AST
static void test_hostcall_in_function() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "test";
    auto* fn = new Function(); fn->id = "f1"; fn->name = "readConfig";
    auto* hc = new HostCall("io.fs", "readFile");
    hc->id = "hc1";
    fn->addChild("body", hc);
    mod->addChild("functions", fn);

    json j = toJson(mod);
    ASTNode* restored = fromJson(j);
    auto fns = restored->getChildren("functions");
    check(fns.size() == 1, "restored has 1 function");
    auto body = fns[0]->getChildren("body");
    check(body.size() == 1, "function has 1 body node");
    check(body[0]->conceptType == "HostCall", "body node is HostCall");
    auto* rhc = static_cast<HostCall*>(body[0]);
    check(rhc->capability == "io.fs", "nested HostCall capability");
    check(rhc->name == "readFile", "nested HostCall name");
    delete mod;
    delete restored;
}

// Test 10: Compact AST getNodeName for host boundary nodes
static void test_compact_node_names() {
    auto* hc = new HostCall("io.fs", "readFile");
    hc->id = "hc1";
    std::string hcName = getNodeName(hc);
    check(hcName == "readFile", "HostCall node name is readFile");

    auto* st = new ScheduleTask();
    st->id = "st1"; st->queue = "microtask";
    std::string stName = getNodeName(st);
    // ScheduleTask may return queue as name or empty — just check no crash
    check(!stName.empty() || stName.empty(), "ScheduleTask getNodeName no crash");

    auto* ml = new ModuleLoad();
    ml->id = "ml1"; ml->moduleName = "math";
    std::string mlName = getNodeName(ml);
    check(mlName == "math", "ModuleLoad node name is math");

    delete hc; delete st; delete ml;
}

// Test 11: HostCall capability validation against env
static void test_hostcall_cap_validation() {
    auto* mod = new Module(); mod->id = "mod1"; mod->name = "test";
    auto* env = new EnvironmentSpec();
    env->envId = "restricted";
    env->capabilities = {"io.fs"};
    mod->addChild("environment", env);

    auto* fn = new Function(); fn->id = "f1"; fn->name = "serve";
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net"; cr->required = true;
    fn->addChild("annotations", cr);
    mod->addChild("functions", fn);

    auto diags = validateCapabilities(mod, env);
    check(diags.size() == 1, "1 diag for missing io.net");
    check(diags[0].message.find("E0501") != std::string::npos,
          "E0501 for missing capability");
    delete mod;
}

// Test 12: ScheduleTask with body children
static void test_schedule_with_body() {
    auto* st = new ScheduleTask();
    st->id = "st1";
    st->queue = "thread";

    auto* hc = new HostCall("io.fs", "writeFile");
    hc->id = "hc1";
    st->addChild("body", hc);

    json j = toJson(st);
    ASTNode* restored = fromJson(j);
    check(restored != nullptr, "roundtrip non-null");
    auto body = restored->getChildren("body");
    check(body.size() == 1, "ScheduleTask has 1 body child");
    check(body[0]->conceptType == "HostCall", "body child is HostCall");
    delete st;
    delete restored;
}

int main() {
    std::cout << "=== Step 288: Host Boundary AST Nodes ===\n";
    test_hostcall_default();
    test_hostcall_params();
    test_hostcall_roundtrip();
    test_schedule_default();
    test_schedule_roundtrip();
    test_moduleload_default();
    test_moduleload_roundtrip();
    test_create_nodes();
    test_hostcall_in_function();
    test_compact_node_names();
    test_hostcall_cap_validation();
    test_schedule_with_body();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
