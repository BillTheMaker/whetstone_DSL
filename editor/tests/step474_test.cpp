// Step 474: Skeleton Generation from Requirements Tests (12 tests)

#include "ArchitectSkeletonGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... " << std::flush; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static StructuredRequirements reqFrom(const std::string& s) {
    return ArchitectProblemParser::parse(s);
}

static SkeletonProjectSpec makeSkeleton(const std::string& prompt) {
    auto req = reqFrom(prompt);
    auto graph = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Layered);
    auto stack = ArchitectTechStackSelector::select(req, graph);
    return ArchitectSkeletonGenerator::generate(req, graph, stack);
}

void test_generates_modules_from_graph_and_stack() {
    TEST(generates_modules_from_graph_and_stack);
    auto sk = makeSkeleton("Build a web REST API with auth and PostgreSQL.");
    CHECK(sk.hasModule("api"), "missing api module");
    CHECK(sk.hasModule("auth"), "missing auth module");
    CHECK(sk.hasModule("data"), "missing data module");
    PASS();
}

void test_module_languages_match_stack_choices() {
    TEST(module_languages_match_stack_choices);
    auto sk = makeSkeleton("Build a web REST API with auth and PostgreSQL.");
    auto ui = sk.getModule("ui");
    auto api = sk.getModule("api");
    CHECK(ui.language == "typescript", "expected typescript ui");
    CHECK(!api.language.empty(), "expected backend language");
    PASS();
}

void test_generates_skeleton_functions_per_module() {
    TEST(generates_skeleton_functions_per_module);
    auto sk = makeSkeleton("Build a REST API with auth.");
    auto api = sk.getModule("api");
    auto auth = sk.getModule("auth");
    CHECK(api.functions.size() >= 1, "expected api functions");
    CHECK(auth.functions.size() >= 1, "expected auth functions");
    PASS();
}

void test_adds_intent_annotations() {
    TEST(adds_intent_annotations);
    auto sk = makeSkeleton("Build a REST API with auth.");
    CHECK(sk.hasModule("api"), "missing api module");
    auto api = sk.getModule("api");
    CHECK(!api.functions.empty(), "missing api functions");
    bool foundIntent = false;
    for (const auto& a : api.functions[0].annotations) {
        if (a.key == "@Intent" && !a.value.empty()) foundIntent = true;
    }
    CHECK(foundIntent, "missing @Intent annotation");
    PASS();
}

void test_adds_routing_annotations_complexity_context_automatability() {
    TEST(adds_routing_annotations_complexity_context_automatability);
    auto sk = makeSkeleton("Build a secure low-latency REST API with auth.");
    CHECK(sk.hasModule("api"), "missing api module");
    auto api = sk.getModule("api");
    CHECK(!api.functions.empty(), "missing api functions");
    bool hasComplexity = false, hasContext = false, hasAutomatability = false;
    for (const auto& a : api.functions[0].annotations) {
        if (a.key == "@Complexity") hasComplexity = true;
        if (a.key == "@ContextWidth") hasContext = true;
        if (a.key == "@Automatability") hasAutomatability = true;
    }
    CHECK(hasComplexity && hasContext && hasAutomatability, "missing routing annotations");
    PASS();
}

void test_adds_contract_annotations() {
    TEST(adds_contract_annotations);
    auto sk = makeSkeleton("Build a secure REST API with auth.");
    CHECK(sk.hasModule("auth"), "missing auth module");
    auto auth = sk.getModule("auth");
    CHECK(!auth.functions.empty(), "missing auth functions");
    bool hasContract = false;
    for (const auto& a : auth.functions[0].annotations) {
        if (a.key == "@Contract" && a.value.find("auth") != std::string::npos) hasContract = true;
    }
    CHECK(hasContract, "missing @Contract annotation");
    PASS();
}

void test_adds_dependency_annotations_between_modules() {
    TEST(adds_dependency_annotations_between_modules);
    auto sk = makeSkeleton("Build a web REST API with auth and PostgreSQL.");
    auto api = sk.getModule("api");
    bool hasAuthDep = false;
    for (const auto& d : api.dependencies) {
        if (d == "auth") hasAuthDep = true;
    }
    CHECK(hasAuthDep, "expected api->auth dependency");
    PASS();
}

void test_generates_multifile_multi_module_spec() {
    TEST(generates_multifile_multi_module_spec);
    auto sk = makeSkeleton("Build a web dashboard API with auth, search, reporting, and PostgreSQL.");
    CHECK(sk.modules.size() >= 4, "expected multi-module skeleton");
    PASS();
}

void test_fallback_module_generated_when_stack_sparse() {
    TEST(fallback_module_generated_when_stack_sparse);
    StructuredRequirements req;
    ModuleGraph g;
    g.nodes.push_back({"core", {"core behavior"}, 3, false});
    TechStackDecision d;
    auto sk = ArchitectSkeletonGenerator::generate(req, g, d);
    CHECK(sk.hasModule("core"), "expected fallback module generation");
    PASS();
}

void test_context_width_varies_by_module_role() {
    TEST(context_width_varies_by_module_role);
    auto sk = makeSkeleton("Build a REST API with auth and Stripe integration.");
    CHECK(sk.hasModule("api"), "missing api module");
    CHECK(sk.hasModule("integrations"), "missing integrations module");
    auto api = sk.getModule("api");
    auto integ = sk.getModule("integrations");
    CHECK(!api.functions.empty(), "missing api functions");
    CHECK(!integ.functions.empty(), "missing integrations functions");
    std::string apiCtx, integCtx;
    for (const auto& a : api.functions[0].annotations) if (a.key == "@ContextWidth") apiCtx = a.value;
    for (const auto& a : integ.functions[0].annotations) if (a.key == "@ContextWidth") integCtx = a.value;
    CHECK(apiCtx == "project", "expected project context for api");
    CHECK(integCtx == "module", "expected module context for integrations");
    PASS();
}

void test_automatability_human_for_auth_module() {
    TEST(automatability_human_for_auth_module);
    auto sk = makeSkeleton("Build a secure API with auth.");
    CHECK(sk.hasModule("auth"), "missing auth module");
    auto auth = sk.getModule("auth");
    CHECK(!auth.functions.empty(), "missing auth functions");
    std::string autoValue;
    for (const auto& a : auth.functions[0].annotations) if (a.key == "@Automatability") autoValue = a.value;
    CHECK(autoValue == "human", "expected human automatability for auth");
    PASS();
}

void test_contract_enriched_for_secure_requirement() {
    TEST(contract_enriched_for_secure_requirement);
    auto sk = makeSkeleton("Build a secure API with auth and postgres.");
    CHECK(sk.hasModule("api"), "missing api module");
    auto api = sk.getModule("api");
    CHECK(!api.functions.empty(), "missing api functions");
    std::string contract;
    for (const auto& a : api.functions[0].annotations) if (a.key == "@Contract") contract = a.value;
    CHECK(contract.find("sec:") != std::string::npos, "expected security contract enrichment");
    PASS();
}

int main() {
    std::cout << "Step 474: Skeleton Generation from Requirements Tests\n";

    test_generates_modules_from_graph_and_stack();      // 1
    test_module_languages_match_stack_choices();        // 2
    test_generates_skeleton_functions_per_module();     // 3
    test_adds_intent_annotations();                     // 4
    test_adds_routing_annotations_complexity_context_automatability(); // 5
    test_adds_contract_annotations();                   // 6
    test_adds_dependency_annotations_between_modules(); // 7
    test_generates_multifile_multi_module_spec();       // 8
    test_fallback_module_generated_when_stack_sparse(); // 9
    test_context_width_varies_by_module_role();         // 10
    test_automatability_human_for_auth_module();        // 11
    test_contract_enriched_for_secure_requirement();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
