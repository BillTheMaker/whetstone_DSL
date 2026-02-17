// Step 476: Phase 23a Integration Tests (8 tests)

#include "ArchitectReviewInterface.h"

#include <iostream>
#include <vector>

struct WorkflowTask {
    std::string moduleName;
    std::string functionName;
};

static std::vector<WorkflowTask> createWorkflow(const SkeletonProjectSpec& sk) {
    std::vector<WorkflowTask> tasks;
    for (const auto& m : sk.modules) {
        for (const auto& f : m.functions) {
            tasks.push_back({m.moduleName, f.name});
        }
    }
    return tasks;
}

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ArchitectReviewState runFlow(const std::string& prompt) {
    auto req = ArchitectProblemParser::parse(prompt);
    auto graph = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Layered);
    auto stack = ArchitectTechStackSelector::select(req, graph);
    auto sk = ArchitectSkeletonGenerator::generate(req, graph, stack);
    return ArchitectReviewInterface::initialize(sk, stack, graph);
}

void test_full_flow_extracts_structured_requirements() {
    TEST(full_flow_extracts_structured_requirements);
    auto req = ArchitectProblemParser::parse(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    CHECK(!req.functionalRequirements.empty(), "missing functional requirements");
    CHECK(!req.integrationRequirements.empty(), "missing integration requirements");
    CHECK(!req.platformConstraints.empty(), "missing platform requirements");
    PASS();
}

void test_full_flow_decomposes_expected_modules() {
    TEST(full_flow_decomposes_expected_modules);
    auto req = ArchitectProblemParser::parse(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    auto g = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Layered);
    CHECK(g.hasModule("api"), "missing api module");
    CHECK(g.hasModule("auth"), "missing auth module");
    CHECK(g.hasModule("data"), "missing data module");
    CHECK(g.hasModule("ui"), "missing ui module");
    PASS();
}

void test_full_flow_stack_selects_frontend_and_database() {
    TEST(full_flow_stack_selects_frontend_and_database);
    auto req = ArchitectProblemParser::parse(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    auto g = ArchitectModuleDecomposer::decompose(req, DecompositionStrategy::Layered);
    auto stack = ArchitectTechStackSelector::select(req, g);
    auto ui = stack.getChoice("ui");
    auto data = stack.getChoice("data");
    CHECK(ui.language == "typescript", "expected typescript frontend");
    CHECK(ui.framework == "react", "expected react framework");
    CHECK(data.database == "postgresql", "expected postgresql");
    PASS();
}

void test_full_flow_generates_skeleton_modules_and_annotations() {
    TEST(full_flow_generates_skeleton_modules_and_annotations);
    auto st = runFlow(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    CHECK(st.skeleton.hasModule("api"), "missing api skeleton module");
    auto api = st.skeleton.getModule("api");
    CHECK(!api.functions.empty(), "missing api functions");
    bool hasIntent = false, hasContract = false;
    for (const auto& a : api.functions[0].annotations) {
        if (a.key == "@Intent") hasIntent = true;
        if (a.key == "@Contract") hasContract = true;
    }
    CHECK(hasIntent && hasContract, "missing key annotations");
    PASS();
}

void test_architect_review_can_modify_and_approve() {
    TEST(architect_review_can_modify_and_approve);
    auto st = runFlow(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    bool changed = ArchitectReviewInterface::changeModuleLanguage(st, "api", "go");
    CHECK(changed, "expected language change");
    ArchitectReviewInterface::approve(st);
    CHECK(st.approved, "expected approved state");
    PASS();
}

void test_workflow_created_after_approval() {
    TEST(workflow_created_after_approval);
    auto st = runFlow(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    ArchitectReviewInterface::approve(st);
    auto tasks = createWorkflow(st.skeleton);
    CHECK(st.approved, "expected approved state");
    CHECK(!tasks.empty(), "expected workflow tasks");
    PASS();
}

void test_dependency_graph_available_in_review_snapshot() {
    TEST(dependency_graph_available_in_review_snapshot);
    auto st = runFlow(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    auto snap = ArchitectReviewInterface::snapshot(st);
    CHECK(!snap.dependencyGraph.empty(), "expected dependency graph");
    CHECK(!snap.modules.empty(), "expected module summaries");
    PASS();
}

void test_end_to_end_flow_keeps_review_structured_for_mcp() {
    TEST(end_to_end_flow_keeps_review_structured_for_mcp);
    auto st = runFlow(
        "Build a REST API for a bookstore with user auth, PostgreSQL, and a React web frontend.");
    auto snap = ArchitectReviewInterface::snapshot(st);
    bool hasStackRationale = !snap.stackRationales.empty();
    bool hasModuleSummary = !snap.modules.empty();
    CHECK(hasStackRationale, "expected stack rationale entries");
    CHECK(hasModuleSummary, "expected module summary entries");
    PASS();
}

int main() {
    std::cout << "Step 476: Phase 23a Integration Tests\n";

    test_full_flow_extracts_structured_requirements();          // 1
    test_full_flow_decomposes_expected_modules();               // 2
    test_full_flow_stack_selects_frontend_and_database();       // 3
    test_full_flow_generates_skeleton_modules_and_annotations();// 4
    test_architect_review_can_modify_and_approve();             // 5
    test_workflow_created_after_approval();                     // 6
    test_dependency_graph_available_in_review_snapshot();       // 7
    test_end_to_end_flow_keeps_review_structured_for_mcp();     // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
