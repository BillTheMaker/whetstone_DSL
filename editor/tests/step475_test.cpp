// Step 475: Architect Review Interface Tests (12 tests)

#include "ArchitectReviewInterface.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static ArchitectReviewState makeState(const std::string& prompt) {
    auto req = ArchitectProblemParser::parse(prompt);
    auto graph = ArchitectModuleDecomposer::decompose(req);
    auto stack = ArchitectTechStackSelector::select(req, graph);
    auto skel = ArchitectSkeletonGenerator::generate(req, graph, stack);
    return ArchitectReviewInterface::initialize(skel, stack, graph);
}

void test_snapshot_contains_module_summaries() {
    TEST(snapshot_contains_module_summaries);
    auto st = makeState("Build a web REST API with auth and PostgreSQL.");
    auto snap = ArchitectReviewInterface::snapshot(st);
    CHECK(!snap.modules.empty(), "expected module summaries");
    CHECK(!snap.dependencyGraph.empty(), "expected dependency graph");
    PASS();
}

void test_snapshot_includes_key_annotations() {
    TEST(snapshot_includes_key_annotations);
    auto st = makeState("Build a REST API with auth.");
    auto snap = ArchitectReviewInterface::snapshot(st);
    bool foundIntent = false;
    for (const auto& m : snap.modules) {
        for (const auto& a : m.keyAnnotations) if (a == "@Intent") foundIntent = true;
    }
    CHECK(foundIntent, "expected key annotation listing");
    PASS();
}

void test_change_language_updates_skeleton_and_stack() {
    TEST(change_language_updates_skeleton_and_stack);
    auto st = makeState("Build a web dashboard.");
    bool ok = ArchitectReviewInterface::changeModuleLanguage(st, "ui", "elm");
    CHECK(ok, "language change should succeed");
    CHECK(st.skeleton.getModule("ui").language == "elm", "skeleton language not updated");
    CHECK(st.stack.getChoice("ui").language == "elm", "stack language not updated");
    PASS();
}

void test_change_language_fails_for_missing_module() {
    TEST(change_language_fails_for_missing_module);
    auto st = makeState("Build an API.");
    bool ok = ArchitectReviewInterface::changeModuleLanguage(st, "missing", "go");
    CHECK(!ok, "language change should fail for missing module");
    PASS();
}

void test_merge_modules_moves_functions_and_removes_secondary() {
    TEST(merge_modules_moves_functions_and_removes_secondary);
    auto st = makeState("Build API with auth.");
    CHECK(st.skeleton.hasModule("api"), "missing api");
    CHECK(st.skeleton.hasModule("auth"), "missing auth");
    auto before = st.skeleton.getModule("api").functions.size();
    bool ok = ArchitectReviewInterface::mergeModules(st, "api", "auth");
    CHECK(ok, "merge should succeed");
    CHECK(!st.skeleton.hasModule("auth"), "secondary module should be removed");
    auto after = st.skeleton.getModule("api").functions.size();
    CHECK(after >= before, "primary should have merged functions");
    PASS();
}

void test_merge_modules_fails_for_invalid_pair() {
    TEST(merge_modules_fails_for_invalid_pair);
    auto st = makeState("Build API with auth.");
    bool ok = ArchitectReviewInterface::mergeModules(st, "api", "missing");
    CHECK(!ok, "merge should fail for missing secondary");
    PASS();
}

void test_add_module_inserts_into_skeleton_stack_graph() {
    TEST(add_module_inserts_into_skeleton_stack_graph);
    auto st = makeState("Build API.");
    bool ok = ArchitectReviewInterface::addModule(st, "billing", "python", {"payment processing"});
    CHECK(ok, "add module should succeed");
    CHECK(st.skeleton.hasModule("billing"), "missing new skeleton module");
    CHECK(st.stack.hasChoiceFor("billing"), "missing new stack choice");
    bool graphHas = false;
    for (const auto& n : st.moduleGraph.nodes) if (n.name == "billing") graphHas = true;
    CHECK(graphHas, "missing new graph node");
    PASS();
}

void test_add_module_fails_when_already_present() {
    TEST(add_module_fails_when_already_present);
    auto st = makeState("Build API.");
    bool ok = ArchitectReviewInterface::addModule(st, "api", "python");
    CHECK(!ok, "adding existing module should fail");
    PASS();
}

void test_adjust_routing_updates_annotations() {
    TEST(adjust_routing_updates_annotations);
    auto st = makeState("Build API with auth.");
    auto api = st.skeleton.getModule("api");
    CHECK(!api.functions.empty(), "missing api functions");
    bool ok = ArchitectReviewInterface::adjustRouting(
        st, "api", api.functions[0].name, "human", "project", "high");
    CHECK(ok, "routing adjustment should succeed");
    auto api2 = st.skeleton.getModule("api");
    std::string autoV, ctxV, compV;
    for (const auto& a : api2.functions[0].annotations) {
        if (a.key == "@Automatability") autoV = a.value;
        if (a.key == "@ContextWidth") ctxV = a.value;
        if (a.key == "@Complexity") compV = a.value;
    }
    CHECK(autoV == "human" && ctxV == "project" && compV == "high",
          "routing annotations not updated");
    PASS();
}

void test_adjust_routing_fails_for_missing_function() {
    TEST(adjust_routing_fails_for_missing_function);
    auto st = makeState("Build API.");
    bool ok = ArchitectReviewInterface::adjustRouting(
        st, "api", "does_not_exist", "llm", "module", "low");
    CHECK(!ok, "routing adjustment should fail for missing function");
    PASS();
}

void test_approve_marks_state_approved() {
    TEST(approve_marks_state_approved);
    auto st = makeState("Build API.");
    CHECK(!st.approved, "expected initial not-approved");
    ArchitectReviewInterface::approve(st);
    CHECK(st.approved, "approve should set approved flag");
    PASS();
}

void test_change_log_records_modifications_and_approval() {
    TEST(change_log_records_modifications_and_approval);
    auto st = makeState("Build API with auth.");
    ArchitectReviewInterface::changeModuleLanguage(st, "api", "go");
    ArchitectReviewInterface::addModule(st, "billing", "python");
    ArchitectReviewInterface::approve(st);
    CHECK(st.changeLog.size() >= 3, "expected change log entries");
    CHECK(st.changeLog.back() == "approved", "expected approval in changelog");
    PASS();
}

int main() {
    std::cout << "Step 475: Architect Review Interface Tests\n";

    test_snapshot_contains_module_summaries();               // 1
    test_snapshot_includes_key_annotations();                // 2
    test_change_language_updates_skeleton_and_stack();       // 3
    test_change_language_fails_for_missing_module();         // 4
    test_merge_modules_moves_functions_and_removes_secondary(); // 5
    test_merge_modules_fails_for_invalid_pair();             // 6
    test_add_module_inserts_into_skeleton_stack_graph();     // 7
    test_add_module_fails_when_already_present();            // 8
    test_adjust_routing_updates_annotations();               // 9
    test_adjust_routing_fails_for_missing_function();        // 10
    test_approve_marks_state_approved();                     // 11
    test_change_log_records_modifications_and_approval();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
