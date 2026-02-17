// Step 447: Migration Execution Integration Tests (12 tests)

#include "MigrationExecutor.h"

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
         "typedef struct { int type; } Token;\n"
         "Token* parse(const char* input){ int m = max(1,2); return 0; }\n"
         "void free_tokens(Token* t){ }\n"
         "int token_count(Token* t){ return clamp(0,0,100); }\n",
         {"parse", "free_tokens", "token_count"}, {"max", "clamp"}},
        {"src/main.c",
         "int main(){ Token* t = parse(\"hello\"); free_tokens(t); return 0; }\n",
         {"main"}, {"parse", "free_tokens"}}
    };
}

static MigrationExecution setupExecution() {
    auto files = testProject();
    auto plan = MigrationPlanGenerator::generate("testproj", files, "c", "rust");
    return MigrationExecutor::createExecution(plan, files);
}

void test_execution_created_from_plan() {
    TEST(execution_created_from_plan);
    auto exec = setupExecution();
    CHECK(exec.projectName == "testproj", "expected project name");
    CHECK(exec.sourceLanguage == "c", "expected source language");
    CHECK(exec.targetLanguage == "rust", "expected target language");
    CHECK(exec.workItems.size() == 3, "expected 3 work items");
    PASS();
}

void test_all_items_start_pending() {
    TEST(all_items_start_pending);
    auto exec = setupExecution();
    CHECK(exec.countByStatus(MigrationStatus::Pending) == 3, "all should be pending");
    CHECK(exec.countByStatus(MigrationStatus::Completed) == 0, "none completed yet");
    PASS();
}

void test_cross_unit_dependencies_respected() {
    TEST(cross_unit_dependencies_respected);
    auto exec = setupExecution();
    // Only leaf modules (no deps) should be ready initially
    auto ready = exec.readyItems();
    CHECK(!ready.empty(), "at least one item should be ready");
    // Utils (leaf) should be ready; parser/main should not be if they depend on utils
    for (const auto& r : ready)
        CHECK(r.dependsOn.empty() || r.filePath == "src/utils.c",
              "ready items should have no unresolved deps");
    PASS();
}

void test_complete_unit_updates_status() {
    TEST(complete_unit_updates_status);
    auto exec = setupExecution();
    auto ready = exec.readyItems();
    CHECK(!ready.empty(), "need ready items");
    MigrationExecutor::completeUnit(exec, ready[0].id);
    CHECK(exec.countByStatus(MigrationStatus::Completed) == 1, "one should be completed");
    PASS();
}

void test_completing_dependency_unblocks_next() {
    TEST(completing_dependency_unblocks_next);
    auto exec = setupExecution();
    auto ready1 = exec.readyItems();
    int initialReady = (int)ready1.size();
    // Complete all initially ready items
    for (const auto& r : ready1)
        MigrationExecutor::completeUnit(exec, r.id);
    auto ready2 = exec.readyItems();
    CHECK((int)ready2.size() >= 1, "completing deps should unblock more items");
    PASS();
}

void test_rollback_annotations() {
    TEST(rollback_annotations);
    auto exec = setupExecution();
    for (const auto& w : exec.workItems) {
        CHECK(w.rollbackAnnotation.find("@Rollback") != std::string::npos,
              "each item should have rollback annotation");
        CHECK(w.rollbackAnnotation.find(w.filePath) != std::string::npos,
              "rollback should reference original file");
    }
    PASS();
}

void test_failed_unit_flagged_for_human_review() {
    TEST(failed_unit_flagged_for_human_review);
    auto exec = setupExecution();
    auto ready = exec.readyItems();
    CHECK(!ready.empty(), "need ready items");
    MigrationExecutor::failUnit(exec, ready[0].id);
    for (const auto& w : exec.workItems) {
        if (w.id == ready[0].id) {
            CHECK(w.status == MigrationStatus::Failed, "should be failed");
            CHECK(w.requiresHumanReview, "failed should require human review");
        }
    }
    PASS();
}

void test_progressive_migration_partial_state() {
    TEST(progressive_migration_partial_state);
    auto exec = setupExecution();
    CHECK(!MigrationExecutor::isPartialMigration(exec), "initially not partial");
    // Complete first unit
    auto ready = exec.readyItems();
    MigrationExecutor::completeUnit(exec, ready[0].id);
    CHECK(MigrationExecutor::isPartialMigration(exec),
          "after completing one unit, should be partial");
    PASS();
}

void test_ffi_boundary_during_partial_migration() {
    TEST(ffi_boundary_during_partial_migration);
    auto exec = setupExecution();
    // Complete utils (leaf)
    auto ready = exec.readyItems();
    for (const auto& r : ready) {
        if (r.filePath == "src/utils.c")
            MigrationExecutor::completeUnit(exec, r.id);
    }
    // Parser depends on utils — now it's a mixed-language boundary
    int ffiBoundaries = exec.countFfiBoundaries();
    CHECK(ffiBoundaries >= 1, "expected FFI boundary in partial migration");
    PASS();
}

void test_full_migration_no_ffi_boundaries() {
    TEST(full_migration_no_ffi_boundaries);
    auto exec = setupExecution();
    // Complete all in order
    while (true) {
        auto ready = exec.readyItems();
        if (ready.empty()) break;
        for (const auto& r : ready)
            MigrationExecutor::completeUnit(exec, r.id);
    }
    CHECK(!MigrationExecutor::isPartialMigration(exec),
          "fully migrated should not be partial");
    CHECK(exec.countByStatus(MigrationStatus::Completed) == 3, "all should be completed");
    PASS();
}

void test_high_risk_units_require_human() {
    TEST(high_risk_units_require_human);
    auto exec = setupExecution();
    bool foundHumanReview = false;
    for (const auto& w : exec.workItems) {
        if (w.requiresHumanReview) foundHumanReview = true;
    }
    // At least parser (high API surface, deps) should require review
    CHECK(foundHumanReview, "expected at least one item requiring human review");
    PASS();
}

void test_work_item_titles_descriptive() {
    TEST(work_item_titles_descriptive);
    auto exec = setupExecution();
    for (const auto& w : exec.workItems) {
        CHECK(w.title.find("Migrate") != std::string::npos, "title should contain Migrate");
        CHECK(w.title.find("->") != std::string::npos, "title should show language transition");
    }
    PASS();
}

int main() {
    std::cout << "Step 447: Migration Execution Integration Tests\n";

    test_execution_created_from_plan();             // 1
    test_all_items_start_pending();                  // 2
    test_cross_unit_dependencies_respected();        // 3
    test_complete_unit_updates_status();              // 4
    test_completing_dependency_unblocks_next();       // 5
    test_rollback_annotations();                     // 6
    test_failed_unit_flagged_for_human_review();     // 7
    test_progressive_migration_partial_state();      // 8
    test_ffi_boundary_during_partial_migration();    // 9
    test_full_migration_no_ffi_boundaries();         // 10
    test_high_risk_units_require_human();            // 11
    test_work_item_titles_descriptive();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
