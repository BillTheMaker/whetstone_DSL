// Step 500: Scenario - Legacy Modernization Tests (12 tests)

#include "LegacyModernizationScenarioRunner.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static LegacyModernizationScenarioResult runNow() {
    return LegacyModernizationScenarioRunner::run();
}

void test_legacy_ingestion_detects_c_idioms_and_legacy_score() {
    TEST(legacy_ingestion_detects_c_idioms_and_legacy_score);
    auto r = runNow();
    CHECK(r.legacyReport.language == "c", "expected c language");
    CHECK(r.legacyReport.legacyScore > 0, "expected non-zero legacy score");
    CHECK(r.legacyReport.hasPattern("manual-memory-management"), "missing manual memory pattern");
    PASS();
}

void test_safety_audit_flags_memory_and_buffer_risks() {
    TEST(safety_audit_flags_memory_and_buffer_risks);
    auto r = runNow();
    CHECK(r.safetyReport.overallRisk >= 2, "expected elevated safety risk");
    CHECK(r.safetyReport.hasCategory("buffer-overflow"), "missing buffer-overflow finding");
    CHECK(r.safetyReport.hasCategory("use-after-free"), "missing use-after-free finding");
    PASS();
}

void test_modernization_suggestions_include_rust_target_and_actions() {
    TEST(modernization_suggestions_include_rust_target_and_actions);
    auto r = runNow();
    CHECK(r.modernizationReport.sourceLanguage == "c", "expected c source");
    CHECK(r.modernizationReport.targetLanguage == "rust", "expected rust target");
    CHECK(!r.modernizationReport.suggestions.empty(), "expected modernization suggestions");
    PASS();
}

void test_security_annotations_strengthened_for_rust_ownership_model() {
    TEST(security_annotations_strengthened_for_rust_ownership_model);
    auto r = runNow();
    CHECK(r.securityOwnershipStrengthened, "expected rust ownership strengthening");
    CHECK(r.modernizationReport.hasSuggestionTo("Rust ownership + borrow checker"),
          "missing rust ownership suggestion");
    PASS();
}

void test_workflow_contains_deterministic_and_llm_routing_paths() {
    TEST(workflow_contains_deterministic_and_llm_routing_paths);
    auto r = runNow();
    CHECK(r.modernizationWorkflow.countByRouting(WorkItemRouting::Deterministic) > 0,
          "expected deterministic work items");
    CHECK(r.modernizationWorkflow.countByRouting(WorkItemRouting::LLM) > 0,
          "expected llm work items");
    PASS();
}

void test_workflow_prioritizes_quick_wins_before_harder_refactors() {
    TEST(workflow_prioritizes_quick_wins_before_harder_refactors);
    auto r = runNow();
    auto ordered = r.modernizationWorkflow.itemsInOrder();
    CHECK(!ordered.empty(), "expected ordered workflow items");
    CHECK(ordered.front().effort == ModernizeEffort::QuickWin,
          "first workflow item should be quick win");
    PASS();
}

void test_migration_plan_targets_rust_and_has_units() {
    TEST(migration_plan_targets_rust_and_has_units);
    auto r = runNow();
    CHECK(r.migrationPlan.sourceLanguage == "c", "expected c source migration plan");
    CHECK(r.migrationPlan.targetLanguage == "rust", "expected rust migration plan target");
    CHECK(r.migrationPlan.units.size() >= 2, "expected multiple migration units");
    PASS();
}

void test_migration_plan_has_mixed_routing_deterministic_and_llm() {
    TEST(migration_plan_has_mixed_routing_deterministic_and_llm);
    auto r = runNow();
    CHECK(r.migrationPlan.countByRouting(MigrationRouting::Deterministic) > 0,
          "expected deterministic migration unit");
    CHECK(r.migrationPlan.countByRouting(MigrationRouting::LLM) > 0,
          "expected llm migration unit");
    PASS();
}

void test_api_boundary_preservation_is_verified() {
    TEST(api_boundary_preservation_is_verified);
    auto r = runNow();
    CHECK(r.apiBoundaryOriginal.hasFunction("clamp_small"), "original api function missing");
    CHECK(r.apiBoundaryMigrated.hasFunction("clamp_small"), "migrated api function missing");
    CHECK(r.apiBoundaryPreserved, "api boundary preservation should verify");
    PASS();
}

void test_ffi_annotations_are_available_for_rust_boundary() {
    TEST(ffi_annotations_are_available_for_rust_boundary);
    auto r = runNow();
    CHECK(!r.apiBoundaryOriginal.ffiBoundaries.empty(), "expected ffi boundaries");
    CHECK(r.apiBoundaryOriginal.ffiBoundaries[0].annotation.find("FFI") != std::string::npos,
          "missing ffi annotation");
    PASS();
}

void test_migration_tests_are_generated_from_preserved_api_surface() {
    TEST(migration_tests_are_generated_from_preserved_api_surface);
    auto r = runNow();
    CHECK(!r.migrationTests.tests.empty(), "expected generated migration tests");
    CHECK(r.migrationTests.targetLanguage == "rust", "expected rust migration tests");
    CHECK(r.migrationTests.hasTestFor("clamp_small"), "missing tests for preserved api");
    PASS();
}

void test_scenario_notes_capture_end_to_end_modernization_flow() {
    TEST(scenario_notes_capture_end_to_end_modernization_flow);
    auto r = runNow();
    CHECK(r.notes.size() >= 2, "expected scenario notes");
    CHECK(r.notes[0].find("Legacy C modernization scenario executed") != std::string::npos,
          "missing execution note");
    PASS();
}

int main() {
    std::cout << "Step 500: Scenario - Legacy Modernization Tests\n";

    test_legacy_ingestion_detects_c_idioms_and_legacy_score();              // 1
    test_safety_audit_flags_memory_and_buffer_risks();                       // 2
    test_modernization_suggestions_include_rust_target_and_actions();        // 3
    test_security_annotations_strengthened_for_rust_ownership_model();       // 4
    test_workflow_contains_deterministic_and_llm_routing_paths();            // 5
    test_workflow_prioritizes_quick_wins_before_harder_refactors();          // 6
    test_migration_plan_targets_rust_and_has_units();                        // 7
    test_migration_plan_has_mixed_routing_deterministic_and_llm();           // 8
    test_api_boundary_preservation_is_verified();                             // 9
    test_ffi_annotations_are_available_for_rust_boundary();                  // 10
    test_migration_tests_are_generated_from_preserved_api_surface();         // 11
    test_scenario_notes_capture_end_to_end_modernization_flow();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
