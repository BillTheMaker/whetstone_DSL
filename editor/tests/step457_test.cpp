// Step 457: Translation Report Tests (12 tests)

#include "IntentTranslator.h"
#include "TranslationReport.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static IntentTranslationResult makeSampleTranslations() {
    std::vector<FunctionIntent> funcs = {
        {"sort_items", "sort list ascending", "items.sort()"},
        {"find_user", "find element by key", "users.get(key)"},
        {"mystery", "", "x = a + b * c"}
    };
    return IntentTranslator::translate(funcs, "python", "rust");
}

void test_report_contains_all_functions() {
    TEST(report_contains_all_functions);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    CHECK(report.functions.size() == 3, "expected 3 function reports");
    CHECK(report.hasFunction("sort_items"), "missing sort_items");
    CHECK(report.hasFunction("find_user"), "missing find_user");
    CHECK(report.hasFunction("mystery"), "missing mystery");
    PASS();
}

void test_source_and_target_languages_propagated() {
    TEST(source_and_target_languages_propagated);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    CHECK(report.sourceLanguage == "python", "wrong source language");
    CHECK(report.targetLanguage == "rust", "wrong target language");
    PASS();
}

void test_idiomatic_and_literal_counts() {
    TEST(idiomatic_and_literal_counts);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    CHECK(report.summary.totalFunctions == 3, "wrong total");
    CHECK(report.summary.idiomaticCount == 2, "expected 2 idiomatic");
    CHECK(report.summary.literalCount == 1, "expected 1 literal");
    PASS();
}

void test_review_flag_count_includes_no_intent() {
    TEST(review_flag_count_includes_no_intent);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    CHECK(report.summary.flaggedForReviewCount == 1, "expected one review-required function");
    CHECK(report.getFunction("mystery").reviewRequired, "mystery should require review");
    PASS();
}

void test_construct_classification_detects_sort() {
    TEST(construct_classification_detects_sort);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    auto fn = report.getFunction("sort_items");
    CHECK(fn.targetConstruct == "sort", "expected sort construct");
    PASS();
}

void test_annotation_delta_includes_confidence_and_idiomatic() {
    TEST(annotation_delta_includes_confidence_and_idiomatic);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    auto fn = report.getFunction("sort_items");
    CHECK(!fn.annotations.added.empty(), "expected added annotations");
    bool hasConfidence = false;
    bool hasIdiomatic = false;
    for (const auto& a : fn.annotations.added) {
        if (a.find("@Confidence(") != std::string::npos) hasConfidence = true;
        if (a.find("@Idiomatic(") != std::string::npos) hasIdiomatic = true;
    }
    CHECK(hasConfidence, "expected confidence annotation");
    CHECK(hasIdiomatic, "expected idiomatic annotation");
    PASS();
}

void test_rationale_mentions_idiomatic_when_applicable() {
    TEST(rationale_mentions_idiomatic_when_applicable);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    auto fn = report.getFunction("find_user");
    CHECK(fn.rationale.find("idiomatic") != std::string::npos, "expected idiomatic rationale");
    PASS();
}

void test_safety_delta_c_to_rust_gained() {
    TEST(safety_delta_c_to_rust_gained);
    std::vector<FunctionIntent> funcs = {{"sum_values", "sum list", "total += x"}};
    auto tr = IntentTranslator::translate(funcs, "c", "rust");
    auto report = TranslationReportGenerator::generate(tr);
    auto fn = report.getFunction("sum_values");
    CHECK(fn.safetyDelta == "gained", "expected gained safety delta for c->rust");
    PASS();
}

void test_safety_delta_rust_to_c_lost() {
    TEST(safety_delta_rust_to_c_lost);
    std::vector<FunctionIntent> funcs = {{"sum_values", "sum list", "values.iter().fold(0, |a, x| a + x)"}};
    auto tr = IntentTranslator::translate(funcs, "rust", "c");
    auto report = TranslationReportGenerator::generate(tr);
    auto fn = report.getFunction("sum_values");
    CHECK(fn.safetyDelta == "lost", "expected lost safety delta for rust->c");
    PASS();
}

void test_percentages_are_computed() {
    TEST(percentages_are_computed);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    CHECK(report.summary.idiomaticPercent() > 66.0f && report.summary.idiomaticPercent() < 67.0f,
          "expected idiomatic percent ~66.67");
    CHECK(report.summary.literalPercent() > 33.0f && report.summary.literalPercent() < 34.0f,
          "expected literal percent ~33.33");
    PASS();
}

void test_get_function_returns_empty_for_missing_name() {
    TEST(get_function_returns_empty_for_missing_name);
    auto tr = makeSampleTranslations();
    auto report = TranslationReportGenerator::generate(tr);
    auto fn = report.getFunction("does_not_exist");
    CHECK(fn.functionName.empty(), "missing function should return empty report entry");
    PASS();
}

void test_has_intent_override_affects_confidence_path() {
    TEST(has_intent_override_affects_confidence_path);
    std::vector<FunctionIntent> funcs = {{"custom", "", "custom_transform(data)"}};
    auto tr = IntentTranslator::translate(funcs, "python", "java");
    auto reportNoIntent = TranslationReportGenerator::generate(tr, {false});
    auto reportWithIntent = TranslationReportGenerator::generate(tr, {true});
    auto a = reportNoIntent.getFunction("custom");
    auto b = reportWithIntent.getFunction("custom");
    CHECK(a.confidence != b.confidence, "intent override should affect confidence class");
    PASS();
}

int main() {
    std::cout << "Step 457: Translation Report Tests\n";

    test_report_contains_all_functions();                  // 1
    test_source_and_target_languages_propagated();        // 2
    test_idiomatic_and_literal_counts();                  // 3
    test_review_flag_count_includes_no_intent();          // 4
    test_construct_classification_detects_sort();         // 5
    test_annotation_delta_includes_confidence_and_idiomatic(); // 6
    test_rationale_mentions_idiomatic_when_applicable();  // 7
    test_safety_delta_c_to_rust_gained();                 // 8
    test_safety_delta_rust_to_c_lost();                   // 9
    test_percentages_are_computed();                      // 10
    test_get_function_returns_empty_for_missing_name();   // 11
    test_has_intent_override_affects_confidence_path();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
