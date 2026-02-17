// Step 481: Phase 23b Integration + Sprint Summary Tests (8 tests)

#include "ArchitectProjectPipeline.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static int sourceFileCount(const ScaffoldPlan& plan) {
    int count = 0;
    for (const auto& f : plan.files) {
        if (f.relativePath.find("/src/") != std::string::npos ||
            f.relativePath.find("/internal/") != std::string::npos ||
            f.relativePath.find("/db/") != std::string::npos) {
            ++count;
        }
    }
    return count;
}

void test_rest_api_template_to_scaffold_to_workflow() {
    TEST(rest_api_template_to_scaffold_to_workflow);
    TemplateInput in{"bookstore", "Book", {"title", "author"}, "jwt"};
    auto result = ArchitectProjectPipeline::fromTemplate(
        ArchitectureTemplateKind::RestApi, in, "/tmp");
    CHECK(!result.templated.skeleton.modules.empty(), "missing skeleton modules");
    CHECK(sourceFileCount(result.scaffold) >= 5, "expected scaffold source files");
    CHECK(!result.workflow.tasks.empty(), "missing workflow tasks");
    PASS();
}

void test_multi_language_project_python_rust_sql_orchestrates_as_single_plan() {
    TEST(multi_language_project_python_rust_sql_orchestrates_as_single_plan);
    SkeletonProjectSpec sk;
    sk.modules.push_back({"service", "python", "", "", {"core"}, {}});
    sk.modules.push_back({"core", "rust", "", "", {"schema"}, {}});
    sk.modules.push_back({"schema", "sql", "", "", {}, {}});
    auto result = ArchitectProjectPipeline::fromSkeleton(sk, "polyglot", "/tmp");
    CHECK(result.workflow.tasks.size() == 3, "expected 3 tasks");
    CHECK(result.workflow.links.size() >= 2, "expected cross-language links");
    CHECK(result.workflow.notes[0].find("single project plan") != std::string::npos,
          "missing single-plan note");
    PASS();
}

void test_architect_language_change_mid_planning_replans_scaffold() {
    TEST(architect_language_change_mid_planning_replans_scaffold);
    TemplateInput in{"uiapp", "Widget", {"id"}, "none"};
    auto base = ArchitectProjectPipeline::fromTemplate(
        ArchitectureTemplateKind::FullStack, in, "/tmp");
    auto changed = ArchitectProjectPipeline::overrideModuleLanguage(
        base.templated.skeleton, "backend", "typescript");
    auto replanned = ArchitectProjectPipeline::fromSkeleton(changed, "uiapp", "/tmp");
    bool hasBackendTs = replanned.scaffold.hasFile("uiapp/src/backend/backend.ts");
    CHECK(hasBackendTs, "expected regenerated backend ts file after language change");
    PASS();
}

void test_all_scaffolded_source_files_contain_semanno_annotations() {
    TEST(all_scaffolded_source_files_contain_semanno_annotations);
    TemplateInput in{"authsvc", "Session", {"token"}, "jwt"};
    auto result = ArchitectProjectPipeline::fromTemplate(
        ArchitectureTemplateKind::RestApi, in, "/tmp");
    int checked = 0;
    for (const auto& f : result.scaffold.files) {
        if (f.relativePath.find("/src/") == std::string::npos &&
            f.relativePath.find("/internal/") == std::string::npos &&
            f.relativePath.find("/db/") == std::string::npos) {
            continue;
        }
        CHECK(f.content.find("@Intent") != std::string::npos, "missing @Intent in scaffolded source");
        ++checked;
    }
    CHECK(checked > 0, "no source files checked");
    PASS();
}

void test_scaffold_apply_materializes_files_and_whetstone_state() {
    TEST(scaffold_apply_materializes_files_and_whetstone_state);
    fs::path root = fs::temp_directory_path() / "whetstone_step481_apply";
    fs::remove_all(root);
    fs::create_directories(root);

    TemplateInput in{"phase23b_apply", "Order", {"id"}, "none"};
    auto result = ArchitectProjectPipeline::fromTemplate(
        ArchitectureTemplateKind::CliTool, in, root.string());
    std::string err;
    CHECK(ArchitectScaffoldGenerator::applyPlan(result.scaffold, &err), ("apply failed: " + err).c_str());
    CHECK(fs::exists(root / "phase23b_apply/.whetstone/workflow_state.json"), "missing workflow state");
    CHECK(fs::exists(root / "phase23b_apply/.gitignore"), "missing .gitignore");
    fs::remove_all(root);
    PASS();
}

void test_cross_language_links_emit_shim_and_review_flags() {
    TEST(cross_language_links_emit_shim_and_review_flags);
    SkeletonProjectSpec sk;
    sk.modules.push_back({"frontend", "typescript", "", "", {"backend"}, {}});
    sk.modules.push_back({"backend", "python", "", "", {"data"}, {}});
    sk.modules.push_back({"data", "sql", "", "", {}, {}});
    auto result = ArchitectProjectPipeline::fromSkeleton(sk, "links", "/tmp");
    CHECK(!result.workflow.links.empty(), "expected links");
    CHECK(result.workflow.links[0].shimAnnotation.find("@Shim(") != std::string::npos, "missing shim");
    CHECK(result.workflow.links[0].reviewRequired, "expected reviewRequired");
    PASS();
}

void test_sidecar_files_exist_for_all_modules_in_scaffold_plan() {
    TEST(sidecar_files_exist_for_all_modules_in_scaffold_plan);
    TemplateInput in{"sidecars", "Entry", {"id"}, "none"};
    auto result = ArchitectProjectPipeline::fromTemplate(
        ArchitectureTemplateKind::Library, in, "/tmp");
    int sidecars = 0;
    for (const auto& m : result.templated.skeleton.modules) {
        const std::string path = "sidecars/.whetstone/sidecars/" + m.moduleName + ".ast.json";
        if (result.scaffold.hasFile(path)) ++sidecars;
    }
    CHECK(sidecars == (int)result.templated.skeleton.modules.size(), "missing sidecar entries");
    PASS();
}

void test_phase23b_pipeline_outputs_remain_structured_for_mcp_consumers() {
    TEST(phase23b_pipeline_outputs_remain_structured_for_mcp_consumers);
    TemplateInput in{"mcp_shape", "Doc", {"id"}, "none"};
    auto result = ArchitectProjectPipeline::fromTemplate(
        ArchitectureTemplateKind::Microservice, in, "/tmp");
    CHECK(!result.scaffold.operations.empty(), "missing operation plan");
    CHECK(result.scaffold.operations[0].method == "fileCreate", "expected fileCreate op");
    CHECK(!result.awareness.modules.empty(), "missing awareness modules");
    CHECK(!result.workflow.tasks.empty(), "missing workflow tasks");
    PASS();
}

int main() {
    std::cout << "Step 481: Phase 23b Integration + Sprint Summary Tests\n";

    test_rest_api_template_to_scaffold_to_workflow();                           // 1
    test_multi_language_project_python_rust_sql_orchestrates_as_single_plan();  // 2
    test_architect_language_change_mid_planning_replans_scaffold();             // 3
    test_all_scaffolded_source_files_contain_semanno_annotations();             // 4
    test_scaffold_apply_materializes_files_and_whetstone_state();               // 5
    test_cross_language_links_emit_shim_and_review_flags();                     // 6
    test_sidecar_files_exist_for_all_modules_in_scaffold_plan();                // 7
    test_phase23b_pipeline_outputs_remain_structured_for_mcp_consumers();       // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
