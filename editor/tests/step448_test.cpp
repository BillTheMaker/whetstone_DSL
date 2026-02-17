// Step 448: Phase 20b Integration + Sprint 20 Summary Tests (8 tests)
// Full migration: 3-file C project → Rust with API preservation, test generation,
// partial migration, and FFI boundary management.

#include "MigrationExecutor.h"
#include "MigrationTestGenerator.h"
#include "ModernizationRPC.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<FileInfo> testProject() {
    return {
        {"src/utils.c",
         "int clamp(int x, int lo, int hi){ return x<lo?lo:x>hi?hi:x; }\n"
         "int max(int a, int b){ return a>b?a:b; }\n",
         {"clamp", "max"}, {}},
        {"src/parser.c",
         "typedef struct { int type; char* text; } Token;\n"
         "Token* parse(const char* input){ int m = max(1,2); return 0; }\n"
         "void free_tokens(Token* t){ }\n"
         "int token_count(Token* t){ return clamp(0,0,100); }\n",
         {"parse", "free_tokens", "token_count"}, {"max", "clamp"}},
        {"src/main.c",
         "int main(){ Token* t = parse(\"hello\"); free_tokens(t); return 0; }\n",
         {"main"}, {"parse", "free_tokens"}}
    };
}

void test_full_migration_3_file_c_to_rust() {
    TEST(full_migration_3_file_c_to_rust);
    auto files = testProject();
    auto plan = MigrationPlanGenerator::generate("myproj", files, "c", "rust");
    auto exec = MigrationExecutor::createExecution(plan, files);

    // Execute all phases in order
    int rounds = 0;
    while (exec.countByStatus(MigrationStatus::Pending) > 0 && rounds < 10) {
        auto ready = exec.readyItems();
        if (ready.empty()) break;
        for (const auto& r : ready)
            MigrationExecutor::completeUnit(exec, r.id);
        ++rounds;
    }

    CHECK(exec.countByStatus(MigrationStatus::Completed) == 3,
          "all 3 files should be migrated");
    CHECK(!MigrationExecutor::isPartialMigration(exec),
          "full migration should not be partial");
    PASS();
}

void test_api_boundaries_preserved_across_migration() {
    TEST(api_boundaries_preserved_across_migration);
    auto files = testProject();
    auto plan = MigrationPlanGenerator::generate("myproj", files, "c", "rust");

    // Check API boundary for each unit
    for (const auto& unit : plan.units) {
        const FileInfo* fi = nullptr;
        for (const auto& f : files)
            if (f.filePath == unit.filePath) { fi = &f; break; }
        if (!fi) continue;

        auto api = APIBoundaryPreserver::analyze(fi->source, unit);
        CHECK(api.apiPreserved, "API should be preserved for " + unit.filePath);
        // Every export should be in the public API
        for (const auto& exp : unit.exports)
            CHECK(api.hasFunction(exp), "missing function: " + exp);
    }
    PASS();
}

void test_generated_tests_validate_equivalence() {
    TEST(generated_tests_validate_equivalence);
    auto files = testProject();
    auto plan = MigrationPlanGenerator::generate("myproj", files, "c", "rust");

    int totalTests = 0;
    for (const auto& unit : plan.units) {
        const FileInfo* fi = nullptr;
        for (const auto& f : files)
            if (f.filePath == unit.filePath) { fi = &f; break; }
        if (!fi) continue;

        auto api = APIBoundaryPreserver::analyze(fi->source, unit);
        auto suite = MigrationTestGenerator::generate(api);
        totalTests += (int)suite.tests.size();

        // Each public function should have at least an equivalence test
        for (const auto& exp : unit.exports)
            CHECK(suite.hasTestFor(exp), "missing test for " + exp);

        // All tests should be in Rust
        for (const auto& t : suite.tests)
            CHECK(t.language == "rust", "test language should be rust");
    }
    CHECK(totalTests >= 6, "expected at least 6 total tests across all modules");
    PASS();
}

void test_partial_migration_2_migrated_1_remaining() {
    TEST(partial_migration_2_migrated_1_remaining);
    auto files = testProject();
    auto plan = MigrationPlanGenerator::generate("myproj", files, "c", "rust");
    auto exec = MigrationExecutor::createExecution(plan, files);

    // Complete only utils and parser (leaf + first dependent)
    int completed = 0;
    while (completed < 2) {
        auto ready = exec.readyItems();
        if (ready.empty()) break;
        MigrationExecutor::completeUnit(exec, ready[0].id);
        ++completed;
    }

    CHECK(MigrationExecutor::isPartialMigration(exec),
          "should be partial with 1 remaining");
    CHECK(exec.countByStatus(MigrationStatus::Completed) == 2, "2 should be completed");
    CHECK(exec.countByStatus(MigrationStatus::Pending) >= 1, "at least 1 pending");
    PASS();
}

void test_ffi_annotations_on_unmigrated_boundary() {
    TEST(ffi_annotations_on_unmigrated_boundary);
    auto files = testProject();
    auto plan = MigrationPlanGenerator::generate("myproj", files, "c", "rust");
    auto exec = MigrationExecutor::createExecution(plan, files);

    // Complete utils only
    auto ready = exec.readyItems();
    for (const auto& r : ready) {
        if (r.filePath == "src/utils.c")
            MigrationExecutor::completeUnit(exec, r.id);
    }

    CHECK(MigrationExecutor::isPartialMigration(exec), "should be partial");
    int ffi = exec.countFfiBoundaries();
    CHECK(ffi >= 1, "expected FFI boundary between migrated and unmigrated units");
    PASS();
}

void test_legacy_analysis_integrated_with_migration() {
    TEST(legacy_analysis_integrated_with_migration);
    // Run legacy analysis on one of the project files
    auto files = testProject();
    json params = {{"source", files[1].source}, {"language", "c"}, {"filePath", "src/parser.c"}};
    auto legacy = ModernizationRPCHandler::handleAnalyzeLegacy(params);

    // Parser source is clean — should have low legacy score
    CHECK(legacy.contains("legacyScore"), "expected legacyScore");
    CHECK(legacy.contains("languageVersion"), "expected languageVersion");
    // Now run safety audit
    auto safety = ModernizationRPCHandler::handleGetSafetyReport(params);
    CHECK(safety.contains("overallRisk"), "expected overallRisk");
    PASS();
}

void test_modernization_and_migration_combined() {
    TEST(modernization_and_migration_combined);
    // Legacy code gets both modernization suggestions AND migration plan
    const std::string legacySrc =
        "void process(){ int* p=(int*)malloc(4); gets(buf); free(p); }";
    json params = {{"source", legacySrc}, {"language", "c"},
                   {"filePath", "legacy.c"}, {"targetLanguage", "rust"}};

    auto suggestions = ModernizationRPCHandler::handleSuggestModernization(params);
    CHECK(!suggestions["suggestions"].empty(), "should have modernization suggestions");

    auto workflow = ModernizationRPCHandler::handleCreateWorkflow(params);
    CHECK(workflow["itemCount"].get<int>() >= 1, "should have workflow items");

    // Migration plan for the same file
    FileInfo fi{"legacy.c", legacySrc, {"process"}, {}};
    auto plan = MigrationPlanGenerator::generate("legacy", {fi}, "c", "rust");
    CHECK(plan.units.size() == 1, "should have 1 migration unit");
    CHECK(plan.units[0].targetLanguage == "rust", "target should be rust");
    PASS();
}

void test_sprint_20_complete_pipeline() {
    TEST(sprint_20_complete_pipeline);
    // Sprint 20 delivers: legacy analysis + safety audit + modernization +
    // migration planning + API preservation + test generation + execution
    // Verify all components work together

    auto files = testProject();

    // Phase 20a: Legacy analysis pipeline
    json params = {{"source", files[0].source}, {"language", "c"}, {"filePath", "src/utils.c"}};
    auto legacy = ModernizationRPCHandler::handleAnalyzeLegacy(params);
    CHECK(legacy.contains("legacyScore"), "phase 20a: legacy analysis works");

    auto safety = ModernizationRPCHandler::handleGetSafetyReport(params);
    CHECK(safety.contains("overallRisk"), "phase 20a: safety audit works");

    auto suggest = ModernizationRPCHandler::handleSuggestModernization(params);
    CHECK(suggest.contains("suggestions"), "phase 20a: suggestions work");

    // Phase 20b: Migration pipeline
    auto plan = MigrationPlanGenerator::generate("sprint20", files, "c", "rust");
    CHECK(plan.units.size() == 3, "phase 20b: migration plan works");

    auto api = APIBoundaryPreserver::analyze(files[0].source, plan.units[0]);
    CHECK(api.apiPreserved, "phase 20b: API boundary works");

    auto suite = MigrationTestGenerator::generate(api);
    CHECK(!suite.tests.empty(), "phase 20b: test generation works");

    auto exec = MigrationExecutor::createExecution(plan, files);
    CHECK(exec.workItems.size() == 3, "phase 20b: execution works");

    PASS();
}

int main() {
    std::cout << "Step 448: Phase 20b Integration + Sprint 20 Summary Tests\n";

    test_full_migration_3_file_c_to_rust();             // 1
    test_api_boundaries_preserved_across_migration();    // 2
    test_generated_tests_validate_equivalence();         // 3
    test_partial_migration_2_migrated_1_remaining();     // 4
    test_ffi_annotations_on_unmigrated_boundary();       // 5
    test_legacy_analysis_integrated_with_migration();    // 6
    test_modernization_and_migration_combined();         // 7
    test_sprint_20_complete_pipeline();                  // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
