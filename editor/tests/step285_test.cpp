// Step 285: Capability Vocabulary & Validation (12 tests)
#include <cassert>
#include <iostream>
#include <string>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Serialization.h"
#include "EnvironmentSpec.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

static int passed = 0, failed = 0;
static void check(bool c, const std::string& n) {
    if (c) { std::cout << "  PASS: " << n << "\n"; ++passed; }
    else   { std::cout << "  FAIL: " << n << "\n"; ++failed; }
}

// Test 1: knownCapabilities returns expected set
static void test_known_capabilities() {
    const auto& caps = knownCapabilities();
    check(caps.count("io.fs") > 0, "io.fs is known");
    check(caps.count("io.net") > 0, "io.net is known");
    check(caps.count("threads") > 0, "threads is known");
    check(caps.count("event_loop") > 0, "event_loop is known");
    check(caps.count("gc") > 0, "gc is known");
    check(caps.count("ui.dom") > 0, "ui.dom is known");
    check(caps.size() >= 17, "at least 17 capabilities");
}

// Test 2: isKnownCapability checks
static void test_is_known_capability() {
    check(isKnownCapability("io.fs"), "io.fs recognized");
    check(isKnownCapability("threads"), "threads recognized");
    check(!isKnownCapability("quantum_computing"), "quantum_computing not known");
    check(!isKnownCapability(""), "empty string not known");
}

// Test 3: CapabilityRequirement construction
static void test_cap_requirement() {
    CapabilityRequirement cr;
    check(cr.conceptType == "CapabilityRequirement", "conceptType");
    check(cr.required == true, "default required is true");
    check(cr.capability.empty(), "default capability empty");

    cr.capability = "io.fs";
    cr.required = true;
    check(cr.capability == "io.fs", "capability set");
}

// Test 4: CapabilityRequirement createNode
static void test_cap_requirement_create_node() {
    ASTNode* node = createNode("CapabilityRequirement");
    check(node != nullptr, "createNode non-null");
    check(node->conceptType == "CapabilityRequirement", "correct type");
    delete node;
}

// Test 5: CapabilityRequirement JSON roundtrip
static void test_cap_requirement_roundtrip() {
    auto* cr = new CapabilityRequirement();
    cr->id = "cr1";
    cr->capability = "io.net";
    cr->required = true;

    json j = toJson(cr);
    check(j["concept"] == "CapabilityRequirement", "JSON concept");
    check(j["properties"]["capability"] == "io.net", "JSON capability");
    check(j["properties"]["required"] == true, "JSON required");

    ASTNode* restored = fromJson(j);
    check(restored != nullptr, "fromJson non-null");
    auto* rcr = static_cast<CapabilityRequirement*>(restored);
    check(rcr->capability == "io.net", "roundtrip capability");
    check(rcr->required == true, "roundtrip required");
    delete cr;
    delete restored;
}

// Test 6: validateCapabilities — all met
static void test_validate_all_met() {
    auto* mod = new Module(); mod->id = "m1"; mod->name = "test";
    auto* fn = new Function(); fn->id = "f1"; fn->name = "readFile";
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.fs"; cr->required = true;
    fn->addChild("annotations", cr);
    mod->addChild("functions", fn);

    EnvironmentSpec env;
    env.envId = "posix";
    env.capabilities = {"io.fs", "io.net", "threads"};

    auto diags = validateCapabilities(mod, &env);
    check(diags.empty(), "no diagnostics when all caps met");
    delete mod;
}

// Test 7: validateCapabilities — unmet requirement
static void test_validate_unmet() {
    auto* mod = new Module(); mod->id = "m1"; mod->name = "test";
    auto* fn = new Function(); fn->id = "f1"; fn->name = "useGPU";
    auto* cr = new CapabilityRequirement();
    cr->capability = "gpu.compute"; cr->required = true;
    fn->addChild("annotations", cr);
    mod->addChild("functions", fn);

    EnvironmentSpec env;
    env.envId = "basic";
    env.capabilities = {"io.fs"};

    auto diags = validateCapabilities(mod, &env);
    check(diags.size() == 1, "1 diagnostic for unmet cap");
    check(diags[0].severity == "error", "severity is error");
    check(diags[0].message.find("E0501") != std::string::npos, "E0501 code");
    check(diags[0].capability == "gpu.compute", "diagnostic has capability");
    delete mod;
}

// Test 8: validateCapabilities — multiple unmet
static void test_validate_multiple_unmet() {
    auto* mod = new Module(); mod->id = "m1"; mod->name = "test";
    auto* fn1 = new Function(); fn1->id = "f1"; fn1->name = "serve";
    auto* cr1 = new CapabilityRequirement(); cr1->capability = "io.net";
    fn1->addChild("annotations", cr1);
    auto* fn2 = new Function(); fn2->id = "f2"; fn2->name = "reflectTypes";
    auto* cr2 = new CapabilityRequirement(); cr2->capability = "reflection";
    fn2->addChild("annotations", cr2);
    mod->addChild("functions", fn1);
    mod->addChild("functions", fn2);

    EnvironmentSpec env;
    env.envId = "minimal";
    env.capabilities = {};

    auto diags = validateCapabilities(mod, &env);
    check(diags.size() == 2, "2 diagnostics for 2 unmet caps");
    delete mod;
}

// Test 9: validateCapabilities — non-required is OK
static void test_validate_optional() {
    auto* mod = new Module(); mod->id = "m1"; mod->name = "test";
    auto* fn = new Function(); fn->id = "f1"; fn->name = "maybeNet";
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net"; cr->required = false;
    fn->addChild("annotations", cr);
    mod->addChild("functions", fn);

    EnvironmentSpec env;
    env.envId = "basic";
    env.capabilities = {};

    auto diags = validateCapabilities(mod, &env);
    check(diags.empty(), "optional requirement → no diagnostic");
    delete mod;
}

// Test 10: validateCapabilities null safety
static void test_validate_null() {
    auto diags1 = validateCapabilities(nullptr, nullptr);
    check(diags1.empty(), "null root → empty");

    auto* mod = new Module(); mod->id = "m1";
    auto diags2 = validateCapabilities(mod, nullptr);
    check(diags2.empty(), "null env → empty");
    delete mod;
}

// Test 11: CapabilityRequirement in isSemanticAnnotation
static void test_cap_is_semantic() {
    check(isSemanticAnnotation("CapabilityRequirement"),
          "CapabilityRequirement is semantic annotation");
}

// Test 12: CapabilityRequirement in compact AST semantic summary
static void test_cap_compact_summary() {
    auto* fn = new Function(); fn->id = "f1"; fn->name = "netOp";
    auto* cr = new CapabilityRequirement();
    cr->capability = "io.net"; cr->required = true;
    fn->addChild("annotations", cr);

    json sem = extractSemanticSummary(fn);
    check(sem.contains("capabilityReq"), "semantic has capabilityReq");
    check(sem["capabilityReq"]["capability"] == "io.net", "capabilityReq.capability");
    check(sem["capabilityReq"]["required"] == true, "capabilityReq.required");
    delete fn;
}

int main() {
    std::cout << "=== Step 285: Capability Vocabulary & Validation ===\n";
    test_known_capabilities();
    test_is_known_capability();
    test_cap_requirement();
    test_cap_requirement_create_node();
    test_cap_requirement_roundtrip();
    test_validate_all_met();
    test_validate_unmet();
    test_validate_multiple_unmet();
    test_validate_optional();
    test_validate_null();
    test_cap_is_semantic();
    test_cap_compact_summary();
    std::cout << "\nResults: " << passed << "/" << (passed+failed) << "\n";
    return failed > 0 ? 1 : 0;
}
