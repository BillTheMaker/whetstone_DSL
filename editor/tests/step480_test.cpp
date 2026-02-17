// Step 480: Multi-Language Project Orchestration Tests (12 tests)

#include "ArchitectMultiLanguageOrchestrator.h"

#include <iostream>
#include <map>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SkeletonModuleSpec mk(const std::string& name, const std::string& lang,
                             std::initializer_list<std::string> deps = {}) {
    SkeletonModuleSpec m;
    m.moduleName = name;
    m.language = lang;
    m.dependencies.assign(deps.begin(), deps.end());
    return m;
}

static std::map<std::string, int> buildOrderMap(const MultiLanguageWorkflowPlan& p) {
    std::map<std::string, int> out;
    for (const auto& t : p.tasks) out[t.moduleName] = t.buildOrder;
    return out;
}

void test_creates_tasks_for_all_modules_in_single_project_workflow() {
    TEST(creates_tasks_for_all_modules_in_single_project_workflow);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python"));
    sk.modules.push_back(mk("core", "rust"));
    sk.modules.push_back(mk("db", "sql"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.tasks.size() == 3, "task count mismatch");
    CHECK(plan.hasTask("api") && plan.hasTask("core") && plan.hasTask("db"), "missing module tasks");
    PASS();
}

void test_build_order_respects_dependencies() {
    TEST(build_order_respects_dependencies);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("schema", "sql"));
    sk.modules.push_back(mk("core", "rust", {"schema"}));
    sk.modules.push_back(mk("api", "python", {"core"}));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    auto ord = buildOrderMap(plan);
    CHECK(ord["schema"] < ord["core"], "schema should precede core");
    CHECK(ord["core"] < ord["api"], "core should precede api");
    PASS();
}

void test_cross_language_dependency_emits_link_annotation() {
    TEST(cross_language_dependency_emits_link_annotation);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"core"}));
    sk.modules.push_back(mk("core", "rust"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(!plan.links.empty(), "expected cross-language link");
    CHECK(plan.links[0].linkAnnotation.find("@Link(api->core)") != std::string::npos,
          "missing @Link annotation");
    PASS();
}

void test_cross_language_dependency_emits_shim_annotation() {
    TEST(cross_language_dependency_emits_shim_annotation);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("ui", "typescript", {"api"}));
    sk.modules.push_back(mk("api", "python"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(!plan.links.empty(), "expected cross-language link");
    CHECK(plan.links[0].shimAnnotation.find("@Shim(typescript_to_python)") != std::string::npos,
          "missing @Shim annotation");
    PASS();
}

void test_same_language_dependency_does_not_emit_cross_language_link() {
    TEST(same_language_dependency_does_not_emit_cross_language_link);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"data"}));
    sk.modules.push_back(mk("data", "python"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.links.empty(), "expected no cross-language links");
    PASS();
}

void test_ffi_boundaries_are_marked_for_human_review() {
    TEST(ffi_boundaries_are_marked_for_human_review);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"core"}));
    sk.modules.push_back(mk("core", "rust"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.links[0].ffiBoundary, "ffi boundary should be true");
    CHECK(plan.links[0].reviewRequired, "review flag should be true");
    PASS();
}

void test_tasks_touching_cross_language_boundary_require_review() {
    TEST(tasks_touching_cross_language_boundary_require_review);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"core"}));
    sk.modules.push_back(mk("core", "rust"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    auto api = plan.getTask("api");
    auto core = plan.getTask("core");
    CHECK(api.reviewRequired, "api should require review");
    CHECK(core.reviewRequired, "core should require review");
    PASS();
}

void test_cycle_fallback_still_produces_complete_ordering() {
    TEST(cycle_fallback_still_produces_complete_ordering);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("a", "python", {"b"}));
    sk.modules.push_back(mk("b", "rust", {"a"}));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.tasks.size() == 2, "expected both tasks");
    CHECK(plan.tasks[0].buildOrder == 1, "first order mismatch");
    CHECK(plan.tasks[1].buildOrder == 2, "second order mismatch");
    PASS();
}

void test_link_direction_matches_dependency_direction() {
    TEST(link_direction_matches_dependency_direction);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("ui", "typescript", {"api"}));
    sk.modules.push_back(mk("api", "python", {"core"}));
    sk.modules.push_back(mk("core", "rust"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.links.size() == 2, "expected two links");
    CHECK(plan.links[0].fromModule == "ui", "first link should originate from ui");
    CHECK(plan.links[0].toModule == "api", "first link should target api");
    PASS();
}

void test_single_language_project_has_no_cross_language_reviews() {
    TEST(single_language_project_has_no_cross_language_reviews);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python"));
    sk.modules.push_back(mk("data", "python", {"api"}));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.links.empty(), "expected zero cross-language links");
    CHECK(!plan.getTask("api").reviewRequired, "single-language task should not require review");
    PASS();
}

void test_task_carries_build_hints_from_awareness_report() {
    TEST(task_carries_build_hints_from_awareness_report);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"data"}));
    sk.modules.push_back(mk("data", "python"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    auto api = plan.getTask("api");
    CHECK(!api.buildHints.empty(), "expected build hints");
    CHECK(api.buildHints[0].find("dependencies = [") != std::string::npos, "missing manifest hint");
    PASS();
}

void test_notes_describe_single_project_orchestration_contract() {
    TEST(notes_describe_single_project_orchestration_contract);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python"));
    auto awareness = ArchitectBuildAwareness::annotate(sk);
    auto plan = ArchitectMultiLanguageOrchestrator::createWorkflow(sk, awareness);
    CHECK(plan.notes.size() >= 2, "missing notes");
    CHECK(plan.notes[0].find("single project plan") != std::string::npos,
          "missing single-project note");
    PASS();
}

int main() {
    std::cout << "Step 480: Multi-Language Project Orchestration Tests\n";

    test_creates_tasks_for_all_modules_in_single_project_workflow();      // 1
    test_build_order_respects_dependencies();                              // 2
    test_cross_language_dependency_emits_link_annotation();                // 3
    test_cross_language_dependency_emits_shim_annotation();                // 4
    test_same_language_dependency_does_not_emit_cross_language_link();     // 5
    test_ffi_boundaries_are_marked_for_human_review();                    // 6
    test_tasks_touching_cross_language_boundary_require_review();          // 7
    test_cycle_fallback_still_produces_complete_ordering();               // 8
    test_link_direction_matches_dependency_direction();                    // 9
    test_single_language_project_has_no_cross_language_reviews();          // 10
    test_task_carries_build_hints_from_awareness_report();                // 11
    test_notes_describe_single_project_orchestration_contract();          // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
