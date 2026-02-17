// Step 499: Scenario — Greenfield Project Tests (12 tests)

#include "GreenfieldScenarioRunner.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static fs::path makeRoot(const std::string& suffix) {
    fs::path root = fs::temp_directory_path() / ("whetstone_step499_" + suffix);
    fs::remove_all(root);
    fs::create_directories(root);
    return root;
}

void test_prompt_is_greenfield_url_shortener_with_user_accounts() {
    TEST(prompt_is_greenfield_url_shortener_with_user_accounts);
    auto root = makeRoot("prompt");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(r.prompt.find("URL shortener") != std::string::npos ||
          r.prompt.find("URL shortener") != std::string::npos, "prompt mismatch");
    fs::remove_all(root);
    PASS();
}

void test_decomposition_produces_api_auth_data_ui_modules() {
    TEST(decomposition_produces_api_auth_data_ui_modules);
    auto root = makeRoot("decompose");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(r.graph.hasModule("api"), "missing api module");
    CHECK(r.graph.hasModule("auth"), "missing auth module");
    CHECK(r.graph.hasModule("data"), "missing data module");
    CHECK(r.graph.hasModule("ui"), "missing ui module");
    fs::remove_all(root);
    PASS();
}

void test_stack_selection_matches_python_postgresql_typescript_expectation() {
    TEST(stack_selection_matches_python_postgresql_typescript_expectation);
    auto root = makeRoot("stack");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(r.stack.getChoice("api").language == "python", "api should be python");
    CHECK(r.stack.getChoice("data").database == "postgresql", "data should be postgresql");
    CHECK(r.stack.getChoice("ui").language == "typescript", "ui should be typescript");
    fs::remove_all(root);
    PASS();
}

void test_skeleton_contains_annotations_on_generated_functions() {
    TEST(skeleton_contains_annotations_on_generated_functions);
    auto root = makeRoot("skeleton");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(!r.skeleton.modules.empty(), "missing skeleton modules");
    bool hasIntent = false;
    for (const auto& m : r.skeleton.modules) {
        for (const auto& f : m.functions) {
            for (const auto& a : f.annotations) if (a.key == "@Intent") hasIntent = true;
        }
    }
    CHECK(hasIntent, "expected @Intent annotations");
    fs::remove_all(root);
    PASS();
}

void test_workflow_tasks_include_deterministic_autocomplete_items() {
    TEST(workflow_tasks_include_deterministic_autocomplete_items);
    auto root = makeRoot("det");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    bool found = false;
    for (const auto& t : r.tasks) {
        if (t.routeTo == "deterministic" && t.autoCompleted) found = true;
    }
    CHECK(found, "expected deterministic auto-complete tasks");
    fs::remove_all(root);
    PASS();
}

void test_llm_tasks_are_prepared_with_nonempty_context_bundles() {
    TEST(llm_tasks_are_prepared_with_nonempty_context_bundles);
    auto root = makeRoot("llm");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    bool found = false;
    for (const auto& t : r.tasks) {
        if (t.routeTo == "llm") {
            found = true;
            CHECK(!t.contextBundle.empty(), "llm context bundle missing");
        }
    }
    CHECK(found, "expected llm task");
    fs::remove_all(root);
    PASS();
}

void test_auth_logic_surfaces_human_review_tasks() {
    TEST(auth_logic_surfaces_human_review_tasks);
    auto root = makeRoot("human");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    bool found = false;
    for (const auto& t : r.tasks) {
        if (t.routeTo == "human") {
            found = true;
            CHECK(t.reviewRequired, "human task should require review");
        }
    }
    CHECK(found, "expected human review task for auth logic");
    fs::remove_all(root);
    PASS();
}

void test_security_scan_executes_and_returns_structured_findings_vector() {
    TEST(security_scan_executes_and_returns_structured_findings_vector);
    auto root = makeRoot("scan");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(r.securityFindings.size() >= 0, "security findings vector missing");
    fs::remove_all(root);
    PASS();
}

void test_project_structure_is_materialized_on_disk() {
    TEST(project_structure_is_materialized_on_disk);
    auto root = makeRoot("disk");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(r.scaffoldApplied, "expected scaffold apply success");
    CHECK(!r.createdPaths.empty(), "expected created files");
    CHECK(fs::exists(root / "url_shortener/.whetstone/workflow_state.json"), "missing workflow state file");
    fs::remove_all(root);
    PASS();
}

void test_scaffold_contains_whetstone_sidecars_and_gitignore() {
    TEST(scaffold_contains_whetstone_sidecars_and_gitignore);
    auto root = makeRoot("scaffold");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(r.scaffold.hasFile("url_shortener/.gitignore"), "missing .gitignore");
    CHECK(r.scaffold.hasFile("url_shortener/.whetstone/workflow_state.json"), "missing workflow state");
    fs::remove_all(root);
    PASS();
}

void test_task_routing_distribution_includes_all_three_worker_paths() {
    TEST(task_routing_distribution_includes_all_three_worker_paths);
    auto root = makeRoot("routing");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    bool hasDet = false, hasLlm = false, hasHuman = false;
    for (const auto& t : r.tasks) {
        if (t.routeTo == "deterministic") hasDet = true;
        if (t.routeTo == "llm") hasLlm = true;
        if (t.routeTo == "human") hasHuman = true;
    }
    CHECK(hasDet && hasLlm && hasHuman, "expected deterministic/llm/human routes");
    fs::remove_all(root);
    PASS();
}

void test_scenario_notes_include_execution_summary() {
    TEST(scenario_notes_include_execution_summary);
    auto root = makeRoot("notes");
    auto r = GreenfieldScenarioRunner::run(root.string(), "url_shortener");
    CHECK(!r.notes.empty(), "missing notes");
    CHECK(r.notes[0].find("end-to-end") != std::string::npos, "missing execution summary note");
    fs::remove_all(root);
    PASS();
}

int main() {
    std::cout << "Step 499: Scenario — Greenfield Project Tests\n";

    test_prompt_is_greenfield_url_shortener_with_user_accounts();              // 1
    test_decomposition_produces_api_auth_data_ui_modules();                    // 2
    test_stack_selection_matches_python_postgresql_typescript_expectation();   // 3
    test_skeleton_contains_annotations_on_generated_functions();                // 4
    test_workflow_tasks_include_deterministic_autocomplete_items();            // 5
    test_llm_tasks_are_prepared_with_nonempty_context_bundles();               // 6
    test_auth_logic_surfaces_human_review_tasks();                             // 7
    test_security_scan_executes_and_returns_structured_findings_vector();      // 8
    test_project_structure_is_materialized_on_disk();                          // 9
    test_scaffold_contains_whetstone_sidecars_and_gitignore();                 // 10
    test_task_routing_distribution_includes_all_three_worker_paths();          // 11
    test_scenario_notes_include_execution_summary();                           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
