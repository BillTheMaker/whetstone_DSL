// Step 501: Scenario - Cross-Language Port Tests (12 tests)

#include "CrossLanguagePortScenarioRunner.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static CrossLanguagePortScenarioResult runNow() {
    return CrossLanguagePortScenarioRunner::run();
}

void test_source_and_target_languages_are_python_to_rust() {
    TEST(source_and_target_languages_are_python_to_rust);
    auto r = runNow();
    CHECK(r.sourceLanguage == "python", "expected python source");
    CHECK(r.targetLanguage == "rust", "expected rust target");
    PASS();
}

void test_pipeline_source_contains_asyncio_and_loop_constructs() {
    TEST(pipeline_source_contains_asyncio_and_loop_constructs);
    auto r = runNow();
    CHECK(r.pipelineSource.find("async") != std::string::npos, "expected async pipeline source");
    CHECK(r.pipelineSource.find("for ") != std::string::npos, "expected loop in pipeline source");
    PASS();
}

void test_function_extraction_and_port_result_population() {
    TEST(function_extraction_and_port_result_population);
    auto r = runNow();
    CHECK(r.functions.size() == 3, "expected three functions in scenario");
    CHECK(r.intentTranslations.translations.size() == 3, "expected three translations");
    PASS();
}

void test_hot_loop_identification_tags_training_loop_function() {
    TEST(hot_loop_identification_tags_training_loop_function);
    auto r = runNow();
    bool foundHot = false;
    for (const auto& f : r.functions) {
        if (f.name == "train_epoch") {
            foundHot = true;
            CHECK(f.hotLoop, "train_epoch should be tagged hot loop");
            CHECK(!f.annotations.empty(), "expected hot loop annotation");
        }
    }
    CHECK(foundHot, "missing train_epoch function");
    PASS();
}

void test_semantic_transpilation_produces_idiomatic_rust_constructs() {
    TEST(semantic_transpilation_produces_idiomatic_rust_constructs);
    auto r = runNow();
    CHECK(r.intentTranslations.countIdiomatic() >= 2, "expected idiomatic rust translations");
    CHECK(r.intentTranslations.getTranslation("train_epoch").targetCode.find("fold") != std::string::npos,
          "train_epoch should map to reduce/fold pattern");
    PASS();
}

void test_memory_model_translation_reflects_gc_to_borrowchecked_shift() {
    TEST(memory_model_translation_reflects_gc_to_borrowchecked_shift);
    auto r = runNow();
    CHECK(r.memoryAnalysis.sourceModel == OwnershipModel::GC, "python source should be GC model");
    CHECK(r.memoryAnalysis.targetModel == OwnershipModel::BorrowChecked,
          "rust target should be borrow-checked model");
    PASS();
}

void test_concurrency_translation_maps_asyncio_to_tokio_pattern() {
    TEST(concurrency_translation_maps_asyncio_to_tokio_pattern);
    auto r = runNow();
    CHECK(r.concurrencyAnalysis.hasPattern("async/await"), "expected async/await translation");
    bool hasTokio = false;
    for (const auto& t : r.concurrencyAnalysis.translations) {
        if (t.pattern == "async/await" &&
            (t.annotation.find("tokio") != std::string::npos ||
             t.note.find("tokio") != std::string::npos)) {
            hasTokio = true;
        }
    }
    CHECK(hasTokio, "expected tokio runtime annotation or note");
    PASS();
}

void test_confidence_scoring_is_generated_for_every_translation() {
    TEST(confidence_scoring_is_generated_for_every_translation);
    auto r = runNow();
    for (const auto& f : r.functions) {
        CHECK(f.confidence.score > 0.0f, "missing confidence score");
        CHECK(!f.confidence.annotation.empty(), "missing confidence annotation");
    }
    CHECK(r.averageConfidence() > 0.0f, "expected positive average confidence");
    PASS();
}

void test_low_confidence_translation_is_flagged_for_human_review() {
    TEST(low_confidence_translation_is_flagged_for_human_review);
    auto r = runNow();
    bool flagged = false;
    for (const auto& f : r.functions) {
        if (f.name == "async_fetch_batch") {
            flagged = true;
            CHECK(f.confidence.reviewRequired, "async function should require review");
        }
    }
    CHECK(flagged, "missing async_fetch_batch function");
    CHECK(!r.reviewRequiredFunctions.empty(), "expected review-required function list");
    PASS();
}

void test_equivalence_assertions_are_generated_per_function() {
    TEST(equivalence_assertions_are_generated_per_function);
    auto r = runNow();
    for (const auto& f : r.functions) {
        CHECK(f.equivalence.assertionCount() > 0, "missing equivalence assertions");
        CHECK(!f.equivalence.annotation.empty(), "missing equivalence annotation");
    }
    PASS();
}

void test_review_queue_matches_low_confidence_functions() {
    TEST(review_queue_matches_low_confidence_functions);
    auto r = runNow();
    int lowConfidence = 0;
    for (const auto& f : r.functions) {
        if (f.confidence.reviewRequired) ++lowConfidence;
    }
    CHECK(static_cast<int>(r.reviewRequiredFunctions.size()) == lowConfidence,
          "review queue size mismatch");
    PASS();
}

void test_scenario_notes_include_cross_language_execution_summary() {
    TEST(scenario_notes_include_cross_language_execution_summary);
    auto r = runNow();
    CHECK(r.notes.size() >= 2, "expected scenario notes");
    CHECK(r.notes[0].find("Python->Rust") != std::string::npos, "missing scenario summary note");
    PASS();
}

int main() {
    std::cout << "Step 501: Scenario - Cross-Language Port Tests\n";

    test_source_and_target_languages_are_python_to_rust();                  // 1
    test_pipeline_source_contains_asyncio_and_loop_constructs();            // 2
    test_function_extraction_and_port_result_population();                  // 3
    test_hot_loop_identification_tags_training_loop_function();             // 4
    test_semantic_transpilation_produces_idiomatic_rust_constructs();       // 5
    test_memory_model_translation_reflects_gc_to_borrowchecked_shift();     // 6
    test_concurrency_translation_maps_asyncio_to_tokio_pattern();           // 7
    test_confidence_scoring_is_generated_for_every_translation();           // 8
    test_low_confidence_translation_is_flagged_for_human_review();          // 9
    test_equivalence_assertions_are_generated_per_function();               // 10
    test_review_queue_matches_low_confidence_functions();                   // 11
    test_scenario_notes_include_cross_language_execution_summary();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
