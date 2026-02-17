// Step 479: Dependency + Build System Awareness Tests (12 tests)

#include "ArchitectBuildAwareness.h"

#include <iostream>
#include <set>
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

void test_python_dependency_annotations_include_imports_and_pip_requirements() {
    TEST(python_dependency_annotations_include_imports_and_pip_requirements);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"data"}));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto api = report.getModule("api");
    CHECK(!api.importHints.empty(), "expected import hints");
    CHECK(api.importHints[0].find("from data import *") != std::string::npos, "missing data import");
    CHECK(api.manifestSnippet.find("fastapi") != std::string::npos, "missing fastapi hint");
    PASS();
}

void test_rust_dependency_annotations_include_cargo_format() {
    TEST(rust_dependency_annotations_include_cargo_format);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("core", "rust", {"util"}));
    sk.modules.push_back(mk("util", "rust"));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto core = report.getModule("core");
    CHECK(core.manifestSnippet.find("[dependencies]") != std::string::npos, "missing dependencies section");
    CHECK(core.manifestSnippet.find("util = { path = \"../util\" }") != std::string::npos,
          "missing local crate path dep");
    PASS();
}

void test_node_dependency_annotations_include_package_json_style() {
    TEST(node_dependency_annotations_include_package_json_style);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "typescript", {"auth"}));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto api = report.getModule("api");
    CHECK(api.manifestSnippet.find("\"dependencies\"") != std::string::npos, "missing package deps object");
    CHECK(api.manifestSnippet.find("express") != std::string::npos, "missing express hint");
    PASS();
}

void test_cpp_dependency_annotations_include_cmake_target_hints() {
    TEST(cpp_dependency_annotations_include_cmake_target_hints);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("engine", "cpp", {"math"}));
    sk.modules.push_back(mk("math", "cpp"));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto engine = report.getModule("engine");
    CHECK(engine.importHints[0].find("#include \"math.h\"") != std::string::npos, "missing include hint");
    CHECK(engine.manifestSnippet.find("target_link_libraries(engine INTERFACE math)") != std::string::npos,
          "missing cmake link line");
    PASS();
}

void test_sql_dependency_annotations_include_migration_ordering() {
    TEST(sql_dependency_annotations_include_migration_ordering);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("schema", "sql"));
    sk.modules.push_back(mk("seed", "sql", {"schema"}));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto schema = report.getModule("schema");
    auto seed = report.getModule("seed");
    CHECK(schema.migrationOrder > 0, "schema missing migration order");
    CHECK(seed.migrationOrder > schema.migrationOrder, "seed should come after schema");
    PASS();
}

void test_module_dependencies_are_preserved_in_report() {
    TEST(module_dependencies_are_preserved_in_report);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python", {"auth", "data"}));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto api = report.getModule("api");
    CHECK(api.moduleDependencies.size() == 2, "dependency count mismatch");
    CHECK(api.moduleDependencies[0] == "auth", "missing auth dep");
    CHECK(api.moduleDependencies[1] == "data", "missing data dep");
    PASS();
}

void test_module_without_dependencies_still_has_manifest_snippet() {
    TEST(module_without_dependencies_still_has_manifest_snippet);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("core", "rust"));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto core = report.getModule("core");
    CHECK(core.manifestSnippet.find("[dependencies]") != std::string::npos, "missing manifest header");
    PASS();
}

void test_mixed_language_report_contains_all_modules() {
    TEST(mixed_language_report_contains_all_modules);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python"));
    sk.modules.push_back(mk("core", "rust"));
    sk.modules.push_back(mk("ui", "typescript"));
    sk.modules.push_back(mk("engine", "cpp"));
    sk.modules.push_back(mk("schema", "sql"));
    auto report = ArchitectBuildAwareness::annotate(sk);
    CHECK(report.modules.size() == 5, "expected five modules");
    PASS();
}

void test_node_dependencies_emit_import_hints_for_each_module_dep() {
    TEST(node_dependencies_emit_import_hints_for_each_module_dep);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("ui", "typescript", {"api", "state"}));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto ui = report.getModule("ui");
    CHECK(ui.importHints.size() == 2, "import hint count mismatch");
    CHECK(ui.importHints[0].find("../api/api") != std::string::npos, "missing api import path");
    PASS();
}

void test_unknown_language_falls_back_to_generic_hint() {
    TEST(unknown_language_falls_back_to_generic_hint);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("docs", "markdown"));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto docs = report.getModule("docs");
    CHECK(docs.manifestSnippet.find("build-hints unavailable") != std::string::npos,
          "missing generic fallback hint");
    PASS();
}

void test_sql_cycle_fallback_is_deterministic() {
    TEST(sql_cycle_fallback_is_deterministic);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("a", "sql", {"b"}));
    sk.modules.push_back(mk("b", "sql", {"a"}));
    auto report = ArchitectBuildAwareness::annotate(sk);
    auto a = report.getModule("a");
    auto b = report.getModule("b");
    CHECK(a.migrationOrder > 0 && b.migrationOrder > 0, "expected migration orders");
    CHECK(a.migrationOrder != b.migrationOrder, "orders should remain unique");
    PASS();
}

void test_global_notes_explain_annotation_scope() {
    TEST(global_notes_explain_annotation_scope);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mk("api", "python"));
    auto report = ArchitectBuildAwareness::annotate(sk);
    CHECK(report.globalNotes.size() >= 2, "missing report notes");
    CHECK(report.globalNotes[1].find("annotation-level") != std::string::npos,
          "missing scope disclaimer");
    PASS();
}

int main() {
    std::cout << "Step 479: Dependency + Build System Awareness Tests\n";

    test_python_dependency_annotations_include_imports_and_pip_requirements(); // 1
    test_rust_dependency_annotations_include_cargo_format();                   // 2
    test_node_dependency_annotations_include_package_json_style();             // 3
    test_cpp_dependency_annotations_include_cmake_target_hints();              // 4
    test_sql_dependency_annotations_include_migration_ordering();              // 5
    test_module_dependencies_are_preserved_in_report();                        // 6
    test_module_without_dependencies_still_has_manifest_snippet();             // 7
    test_mixed_language_report_contains_all_modules();                         // 8
    test_node_dependencies_emit_import_hints_for_each_module_dep();            // 9
    test_unknown_language_falls_back_to_generic_hint();                        // 10
    test_sql_cycle_fallback_is_deterministic();                                // 11
    test_global_notes_explain_annotation_scope();                              // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
