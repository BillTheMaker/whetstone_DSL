// Step 478: Scaffold File Generation Tests (12 tests)

#include "ArchitectScaffoldGenerator.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

namespace fs = std::filesystem;

static bool endsWith(const std::string& v, const std::string& suffix) {
    return v.size() >= suffix.size() &&
           v.compare(v.size() - suffix.size(), suffix.size(), suffix) == 0;
}

static const ScaffoldFileSpec* findBySuffix(const ScaffoldPlan& plan,
                                            const std::string& suffix) {
    for (const auto& f : plan.files) {
        if (endsWith(f.relativePath, suffix)) return &f;
    }
    return nullptr;
}

static SkeletonFunctionSpec fn(const std::string& name) {
    SkeletonFunctionSpec f;
    f.name = name;
    f.signature = "void " + name + "()";
    f.annotations = {
        {"@Intent", "intent " + name},
        {"@Complexity", "medium"},
        {"@ContextWidth", "module"},
        {"@Automatability", "llm"},
        {"@Contract", "pre: x; post: y"}
    };
    return f;
}

static SkeletonModuleSpec mod(const std::string& name, const std::string& lang,
                              std::initializer_list<SkeletonFunctionSpec> fns = {}) {
    SkeletonModuleSpec m;
    m.moduleName = name;
    m.language = lang;
    m.functions.assign(fns.begin(), fns.end());
    return m;
}

static SkeletonProjectSpec mixedSkeleton() {
    SkeletonProjectSpec sk;
    sk.modules.push_back(mod("api", "typescript", {fn("handleRequest")}));
    sk.modules.push_back(mod("workers", "go", {fn("dispatch")}));
    sk.modules.push_back(mod("schema", "sql", {fn("migrate")}));
    return sk;
}

void test_generates_source_files_with_semanno_annotations() {
    TEST(generates_source_files_with_semanno_annotations);
    ScaffoldInput in{"/tmp", "demo", mixedSkeleton(), true};
    auto plan = ArchitectScaffoldGenerator::buildPlan(in);
    auto* src = findBySuffix(plan, "demo/src/api/api.ts");
    CHECK(src != nullptr, "missing api scaffold file");
    CHECK(src->content.find("@Intent") != std::string::npos, "missing @Intent annotation");
    CHECK(src->content.find("handleRequest") != std::string::npos, "missing function stub");
    PASS();
}

void test_generates_package_json_for_node_stack() {
    TEST(generates_package_json_for_node_stack);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mod("api", "typescript", {fn("main")}));
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", sk, true});
    CHECK(plan.hasFile("demo/package.json"), "missing package.json");
    PASS();
}

void test_generates_pyproject_for_python_stack() {
    TEST(generates_pyproject_for_python_stack);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mod("jobs", "python", {fn("runJobs")}));
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", sk, true});
    CHECK(plan.hasFile("demo/pyproject.toml"), "missing pyproject.toml");
    PASS();
}

void test_generates_cargo_for_rust_stack() {
    TEST(generates_cargo_for_rust_stack);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mod("core", "rust", {fn("compute")}));
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", sk, true});
    CHECK(plan.hasFile("demo/Cargo.toml"), "missing Cargo.toml");
    PASS();
}

void test_generates_cmake_for_cpp_stack() {
    TEST(generates_cmake_for_cpp_stack);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mod("engine", "cpp", {fn("solve")}));
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", sk, true});
    CHECK(plan.hasFile("demo/CMakeLists.txt"), "missing CMakeLists.txt");
    PASS();
}

void test_directory_structure_follows_language_conventions() {
    TEST(directory_structure_follows_language_conventions);
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", mixedSkeleton(), true});
    CHECK(findBySuffix(plan, "demo/src/api/api.ts") != nullptr, "missing ts convention path");
    CHECK(findBySuffix(plan, "demo/internal/workers/workers.go") != nullptr,
          "missing go convention path");
    CHECK(findBySuffix(plan, "demo/db/schema.sql") != nullptr, "missing sql convention path");
    PASS();
}

void test_gitignore_matches_selected_stack() {
    TEST(gitignore_matches_selected_stack);
    SkeletonProjectSpec sk;
    sk.modules.push_back(mod("api", "typescript", {fn("a")}));
    sk.modules.push_back(mod("jobs", "python", {fn("b")}));
    sk.modules.push_back(mod("core", "rust", {fn("c")}));
    sk.modules.push_back(mod("engine", "cpp", {fn("d")}));
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", sk, true});
    auto* gi = findBySuffix(plan, "demo/.gitignore");
    CHECK(gi != nullptr, "missing .gitignore");
    CHECK(gi->content.find("node_modules/") != std::string::npos, "missing node ignore");
    CHECK(gi->content.find("__pycache__/") != std::string::npos, "missing python ignore");
    CHECK(gi->content.find("target/") != std::string::npos, "missing rust ignore");
    CHECK(gi->content.find("build/") != std::string::npos, "missing cpp ignore");
    PASS();
}

void test_whetstone_state_and_sidecars_are_generated() {
    TEST(whetstone_state_and_sidecars_are_generated);
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", mixedSkeleton(), true});
    CHECK(plan.hasFile("demo/.whetstone/workflow_state.json"), "missing workflow state file");
    CHECK(plan.hasFile("demo/.whetstone/sidecars/api.ast.json"), "missing api sidecar");
    CHECK(plan.hasFile("demo/.whetstone/sidecars/workers.ast.json"), "missing workers sidecar");
    PASS();
}

void test_operations_are_filecreate_and_filewrite_pairs() {
    TEST(operations_are_filecreate_and_filewrite_pairs);
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", mixedSkeleton(), true});
    CHECK(plan.operations.size() == plan.files.size() * 2, "unexpected operation count");
    int creates = 0, writes = 0;
    for (const auto& op : plan.operations) {
        if (op.method == "fileCreate") ++creates;
        if (op.method == "fileWrite") ++writes;
    }
    CHECK(creates == (int)plan.files.size(), "fileCreate count mismatch");
    CHECK(writes == (int)plan.files.size(), "fileWrite count mismatch");
    PASS();
}

void test_apply_plan_writes_files_to_disk() {
    TEST(apply_plan_writes_files_to_disk);
    fs::path root = fs::temp_directory_path() / "whetstone_step478_apply";
    fs::remove_all(root);
    fs::create_directories(root);

    auto plan = ArchitectScaffoldGenerator::buildPlan({root.string(), "demo", mixedSkeleton(), true});
    std::string err;
    bool ok = ArchitectScaffoldGenerator::applyPlan(plan, &err);
    CHECK(ok, ("apply failed: " + err).c_str());
    CHECK(fs::exists(root / "demo/src/api/api.ts"), "missing written source file");
    CHECK(fs::exists(root / "demo/.whetstone/workflow_state.json"), "missing written workflow state");

    std::ifstream in(root / "demo/src/api/api.ts");
    std::string body((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    CHECK(body.find("@Intent") != std::string::npos, "written file missing annotation");
    fs::remove_all(root);
    PASS();
}

void test_apply_plan_is_idempotent_when_files_exist() {
    TEST(apply_plan_is_idempotent_when_files_exist);
    fs::path root = fs::temp_directory_path() / "whetstone_step478_idempotent";
    fs::remove_all(root);
    fs::create_directories(root);

    auto plan = ArchitectScaffoldGenerator::buildPlan({root.string(), "demo", mixedSkeleton(), true});
    std::string err;
    CHECK(ArchitectScaffoldGenerator::applyPlan(plan, &err), ("first apply failed: " + err).c_str());
    err.clear();
    CHECK(ArchitectScaffoldGenerator::applyPlan(plan, &err), ("second apply failed: " + err).c_str());
    fs::remove_all(root);
    PASS();
}

void test_empty_skeleton_still_generates_basics() {
    TEST(empty_skeleton_still_generates_basics);
    SkeletonProjectSpec sk;
    auto plan = ArchitectScaffoldGenerator::buildPlan({"/tmp", "demo", sk, true});
    CHECK(plan.hasFile("demo/.gitignore"), "missing .gitignore");
    CHECK(plan.hasFile("demo/.whetstone/workflow_state.json"), "missing workflow state");
    CHECK(!plan.operations.empty(), "expected non-empty file operations");
    PASS();
}

int main() {
    std::cout << "Step 478: Scaffold File Generation Tests\n";

    test_generates_source_files_with_semanno_annotations();      // 1
    test_generates_package_json_for_node_stack();                // 2
    test_generates_pyproject_for_python_stack();                 // 3
    test_generates_cargo_for_rust_stack();                       // 4
    test_generates_cmake_for_cpp_stack();                        // 5
    test_directory_structure_follows_language_conventions();     // 6
    test_gitignore_matches_selected_stack();                     // 7
    test_whetstone_state_and_sidecars_are_generated();           // 8
    test_operations_are_filecreate_and_filewrite_pairs();        // 9
    test_apply_plan_writes_files_to_disk();                      // 10
    test_apply_plan_is_idempotent_when_files_exist();            // 11
    test_empty_skeleton_still_generates_basics();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
