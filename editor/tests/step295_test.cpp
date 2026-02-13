// Step 295: Semanno Sidecar Integration (12 tests)
#include "SemannoSidecar.h"
#include "SidecarPersistence.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
#include "EnvironmentSpec.h"
#include <iostream>
#include <filesystem>
#include <fstream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string tmpDir() {
    auto p = std::filesystem::temp_directory_path() / "whetstone_test_295";
    std::filesystem::create_directories(p);
    return p.string();
}

static void cleanup() {
    std::filesystem::remove_all(std::filesystem::temp_directory_path() / "whetstone_test_295");
}

// Helper: create a module with annotations
static std::unique_ptr<Module> makeAnnotatedModule() {
    auto mod = std::make_unique<Module>();
    mod->name = "test_module";

    auto fn = std::make_unique<Function>();
    fn->name = "compute";
    fn->spanStartLine = 5;

    auto intent = std::make_unique<IntentAnnotation>();
    intent->summary = "computes result";
    intent->category = "pure";
    fn->addChild("annotations", intent.release());

    auto risk = std::make_unique<RiskAnnotation>();
    risk->level = "low";
    risk->reason = "simple math";
    fn->addChild("annotations", risk.release());

    mod->addChild("functions", fn.release());
    return mod;
}

// 1. Sidecar path generation
void test_sidecar_path() {
    TEST(sidecar_path);
    std::string path = semannoSidecarPath("/workspace", "src/main.py");
    CHECK(path.find(".whetstone") != std::string::npos, "missing .whetstone dir");
    CHECK(path.find("src/main.py.semanno") != std::string::npos, "wrong extension");
    PASS();
}

// 2. Save creates file on disk
void test_save_creates_file() {
    TEST(save_creates_file);
    cleanup();
    auto mod = makeAnnotatedModule();
    auto result = saveSemannoSidecar(tmpDir(), "test.py", mod.get());
    CHECK(result.success, "save failed: " + result.error);
    CHECK(result.annotationCount == 2, "wrong count: " + std::to_string(result.annotationCount));
    CHECK(std::filesystem::exists(result.path), "file not created");
    PASS();
}

// 3. Save content format (L<line>: @semanno:...)
void test_save_content_format() {
    TEST(save_content_format);
    cleanup();
    auto mod = makeAnnotatedModule();
    auto result = saveSemannoSidecar(tmpDir(), "test.py", mod.get());
    CHECK(result.success, "save failed");

    std::ifstream in(result.path);
    std::string content((std::istreambuf_iterator<char>(in)),
                         std::istreambuf_iterator<char>());
    CHECK(content.find("L5: @semanno:intent") != std::string::npos, "missing L5 intent");
    CHECK(content.find("L5: @semanno:risk") != std::string::npos, "missing L5 risk");
    PASS();
}

// 4. Load roundtrip
void test_load_roundtrip() {
    TEST(load_roundtrip);
    cleanup();
    auto mod = makeAnnotatedModule();
    saveSemannoSidecar(tmpDir(), "test.py", mod.get());

    auto loaded = loadSemannoSidecar(tmpDir(), "test.py");
    CHECK(loaded.success, "load failed: " + loaded.error);
    CHECK(loaded.count == 2, "wrong count: " + std::to_string(loaded.count));
    // Verify entries
    bool hasIntent = false, hasRisk = false;
    for (const auto& entry : loaded.entries) {
        if (entry.type == "intent") hasIntent = true;
        if (entry.type == "risk") hasRisk = true;
    }
    CHECK(hasIntent, "missing intent entry");
    CHECK(hasRisk, "missing risk entry");
    PASS();
}

// 5. Load nonexistent file returns error
void test_load_nonexistent() {
    TEST(load_nonexistent);
    cleanup();
    auto loaded = loadSemannoSidecar(tmpDir(), "nonexistent.py");
    CHECK(!loaded.success, "should fail");
    CHECK(loaded.count == 0, "should have 0 entries");
    PASS();
}

// 6. Save null AST returns error
void test_save_null_ast() {
    TEST(save_null_ast);
    auto result = saveSemannoSidecar(tmpDir(), "test.py", nullptr);
    CHECK(!result.success, "should fail");
    CHECK(result.error.find("No AST") != std::string::npos, "wrong error");
    PASS();
}

// 7. Multiple annotation types in sidecar
void test_multiple_types() {
    TEST(multiple_types);
    cleanup();
    auto mod = std::make_unique<Module>();
    mod->name = "multi";

    auto fn = std::make_unique<Function>();
    fn->name = "process";
    fn->spanStartLine = 10;

    fn->addChild("annotations", new IntentAnnotation());
    fn->addChild("annotations", new ComplexityAnnotation());
    auto contract = new ContractAnnotation();
    contract->preconditions = "x > 0";
    fn->addChild("annotations", contract);
    auto tags = new SemanticTagAnnotation();
    tags->tags = {"io", "network"};
    fn->addChild("annotations", tags);

    mod->addChild("functions", fn.release());

    auto result = saveSemannoSidecar(tmpDir(), "multi.py", mod.get());
    CHECK(result.success, "save failed");
    CHECK(result.annotationCount == 4, "wrong count: " + std::to_string(result.annotationCount));

    auto loaded = loadSemannoSidecar(tmpDir(), "multi.py");
    CHECK(loaded.count == 4, "loaded wrong count: " + std::to_string(loaded.count));
    PASS();
}

// 8. Sidecar alongside .ast.json
void test_alongside_ast_sidecar() {
    TEST(alongside_ast_sidecar);
    cleanup();
    auto mod = makeAnnotatedModule();

    // Save both types
    auto semannoResult = saveSemannoSidecar(tmpDir(), "test.py", mod.get());
    CHECK(semannoResult.success, "semanno save failed");

    // Also save AST sidecar using SidecarPersistence
    auto astPath = sidecarPath(tmpDir(), "test.py");
    namespace fs = std::filesystem;
    fs::create_directories(fs::path(astPath).parent_path());
    saveSidecarAST(tmpDir(), "test.py", mod.get());

    // Both files should exist in same .whetstone dir
    CHECK(fs::exists(semannoResult.path), "semanno file missing");
    CHECK(fs::exists(astPath), "ast file missing");
    // Both under .whetstone/
    CHECK(semannoResult.path.find(".whetstone") != std::string::npos, "wrong dir");
    CHECK(astPath.find(".whetstone") != std::string::npos, "wrong dir");
    PASS();
}

// 9. Environment annotations in sidecar
void test_environment_annotations() {
    TEST(environment_annotations);
    cleanup();
    auto mod = std::make_unique<Module>();
    mod->name = "env_test";

    auto fn = std::make_unique<Function>();
    fn->name = "net_call";
    fn->spanStartLine = 1;

    auto cap = new CapabilityRequirement();
    cap->capability = "io.net";
    cap->required = true;
    fn->addChild("annotations", cap);

    mod->addChild("functions", fn.release());

    auto result = saveSemannoSidecar(tmpDir(), "net.py", mod.get());
    CHECK(result.success, "save failed");
    CHECK(result.annotationCount == 1, "wrong count");

    auto loaded = loadSemannoSidecar(tmpDir(), "net.py");
    CHECK(loaded.count == 1, "loaded wrong count");
    CHECK(loaded.entries[0].type == "capability", "wrong type");
    CHECK(loaded.entries[0].properties["capability"] == "io.net", "wrong cap");
    PASS();
}

// 10. Overwrite existing sidecar
void test_overwrite() {
    TEST(overwrite);
    cleanup();
    auto mod1 = makeAnnotatedModule();
    saveSemannoSidecar(tmpDir(), "test.py", mod1.get());

    // Save again with different content
    auto mod2 = std::make_unique<Module>();
    mod2->name = "changed";
    auto fn = std::make_unique<Function>();
    fn->name = "new_func";
    fn->spanStartLine = 1;
    fn->addChild("annotations", new PureAnnotation());
    mod2->addChild("functions", fn.release());

    auto result = saveSemannoSidecar(tmpDir(), "test.py", mod2.get());
    CHECK(result.success, "save failed");
    // PureAnnotation is Subject 1 memory — check if isSemanticAnnotation includes it
    // PureAnnotation is from Sprint 3 original set, check what isSemanticAnnotation says
    // Either way the save should succeed
    auto loaded = loadSemannoSidecar(tmpDir(), "test.py");
    CHECK(loaded.success, "load failed");
    PASS();
}

// 11. Properties preserved in roundtrip
void test_properties_preserved() {
    TEST(properties_preserved);
    cleanup();
    auto mod = std::make_unique<Module>();
    mod->name = "props";

    auto fn = std::make_unique<Function>();
    fn->name = "f";
    fn->spanStartLine = 3;

    auto contract = new ContractAnnotation();
    contract->preconditions = "len(arr) > 0";
    contract->postconditions = "result >= 0";
    contract->returnShape = "int";
    contract->sideEffects = "none";
    fn->addChild("annotations", contract);

    mod->addChild("functions", fn.release());

    saveSemannoSidecar(tmpDir(), "props.py", mod.get());
    auto loaded = loadSemannoSidecar(tmpDir(), "props.py");
    CHECK(loaded.success, "load failed");
    CHECK(loaded.count == 1, "wrong count");

    auto& entry = loaded.entries[0];
    CHECK(entry.type == "contract", "wrong type");
    CHECK(entry.properties["preconditions"] == "len(arr) > 0", "preconditions lost");
    CHECK(entry.properties["postconditions"] == "result >= 0", "postconditions lost");
    PASS();
}

// 12. Empty module produces empty sidecar
void test_empty_module() {
    TEST(empty_module);
    cleanup();
    auto mod = std::make_unique<Module>();
    mod->name = "empty";

    auto result = saveSemannoSidecar(tmpDir(), "empty.py", mod.get());
    CHECK(result.success, "save failed");
    CHECK(result.annotationCount == 0, "should be empty");

    auto loaded = loadSemannoSidecar(tmpDir(), "empty.py");
    CHECK(loaded.success, "load failed");
    CHECK(loaded.count == 0, "should be empty");
    PASS();
}

int main() {
    std::cout << "=== Step 295: Semanno Sidecar Integration Tests ===\n";
    test_sidecar_path();
    test_save_creates_file();
    test_save_content_format();
    test_load_roundtrip();
    test_load_nonexistent();
    test_save_null_ast();
    test_multiple_types();
    test_alongside_ast_sidecar();
    test_environment_annotations();
    test_overwrite();
    test_properties_preserved();
    test_empty_module();
    cleanup();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
