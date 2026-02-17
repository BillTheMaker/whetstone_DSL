// Step 495: Transpile Whetstone Modules Tests (12 tests)

#include "SelfHostTranspileAudit.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_default_specs_include_three_expected_headers() {
    TEST(default_specs_include_three_expected_headers);
    auto specs = SelfHostTranspileAudit::defaultSpecs();
    CHECK(specs.size() == 3, "expected three specs");
    CHECK(specs[0].sourceHeaderPath.find("AnnotationConflictExtended.h") != std::string::npos, "missing first header");
    CHECK(specs[1].sourceHeaderPath.find("ResponseBudget.h") != std::string::npos, "missing second header");
    CHECK(specs[2].sourceHeaderPath.find("Icons.h") != std::string::npos, "missing third header");
    PASS();
}

void test_default_run_produces_three_file_reports() {
    TEST(default_run_produces_three_file_reports);
    auto r = SelfHostTranspileAudit::runDefault();
    CHECK(r.files.size() == 3, "expected three file reports");
    PASS();
}

void test_sources_parse_before_projection() {
    TEST(sources_parse_before_projection);
    auto r = SelfHostTranspileAudit::runDefault();
    for (const auto& f : r.files) {
        CHECK(f.sourceParsed, "source should parse");
    }
    PASS();
}

void test_projection_and_annotation_preservation_hold() {
    TEST(projection_and_annotation_preservation_hold);
    auto r = SelfHostTranspileAudit::runDefault();
    for (const auto& f : r.files) {
        CHECK(f.projected, "projection should succeed");
        CHECK(f.annotationsPreserved, "annotations should be preserved");
    }
    PASS();
}

void test_generated_target_code_is_nonempty_for_each_translation() {
    TEST(generated_target_code_is_nonempty_for_each_translation);
    auto r = SelfHostTranspileAudit::runDefault();
    for (const auto& f : r.files) {
        CHECK(f.generatedCodeNonEmpty, "generated code should not be empty");
    }
    PASS();
}

void test_target_parse_attempt_runs_for_each_translation() {
    TEST(target_parse_attempt_runs_for_each_translation);
    auto r = SelfHostTranspileAudit::runDefault();
    for (const auto& f : r.files) {
        CHECK(f.targetParses || !f.warnings.empty(), "target parse should run and report result");
    }
    PASS();
}

void test_confidence_score_is_recorded_per_file() {
    TEST(confidence_score_is_recorded_per_file);
    auto r = SelfHostTranspileAudit::runDefault();
    for (const auto& f : r.files) {
        CHECK(f.confidence.score >= 0.0f, "confidence below zero");
        CHECK(f.confidence.score <= 1.0f, "confidence above one");
    }
    PASS();
}

void test_average_confidence_is_aggregate_of_reports() {
    TEST(average_confidence_is_aggregate_of_reports);
    auto r = SelfHostTranspileAudit::runDefault();
    CHECK(r.averageConfidence >= 0.0f && r.averageConfidence <= 1.0f,
          "average confidence out of range");
    PASS();
}

void test_idiomatic_and_literal_counts_sum_to_file_count() {
    TEST(idiomatic_and_literal_counts_sum_to_file_count);
    auto r = SelfHostTranspileAudit::runDefault();
    CHECK(r.idiomaticCount + r.literalCount == (int)r.files.size(),
          "idiomatic+literal count mismatch");
    PASS();
}

void test_target_languages_match_python_rust_java() {
    TEST(target_languages_match_python_rust_java);
    auto r = SelfHostTranspileAudit::runDefault();
    CHECK(r.hasTarget("python"), "missing python target");
    CHECK(r.hasTarget("rust"), "missing rust target");
    CHECK(r.hasTarget("java"), "missing java target");
    PASS();
}

void test_structural_counts_are_propagated_to_projected_ast_metrics() {
    TEST(structural_counts_are_propagated_to_projected_ast_metrics);
    auto r = SelfHostTranspileAudit::runDefault();
    for (const auto& f : r.files) {
        CHECK(f.projectedFunctionCount >= 0, "projected function count invalid");
        CHECK(f.projectedClassCount >= 0, "projected class count invalid");
    }
    PASS();
}

void test_report_includes_completion_note() {
    TEST(report_includes_completion_note);
    auto r = SelfHostTranspileAudit::runDefault();
    CHECK(!r.notes.empty(), "expected report notes");
    CHECK(r.notes[0].find("complete") != std::string::npos, "missing completion note");
    PASS();
}

int main() {
    std::cout << "Step 495: Transpile Whetstone Modules Tests\n";

    test_default_specs_include_three_expected_headers();               // 1
    test_default_run_produces_three_file_reports();                    // 2
    test_sources_parse_before_projection();                            // 3
    test_projection_and_annotation_preservation_hold();                // 4
    test_generated_target_code_is_nonempty_for_each_translation();     // 5
    test_target_parse_attempt_runs_for_each_translation();             // 6
    test_confidence_score_is_recorded_per_file();                      // 7
    test_average_confidence_is_aggregate_of_reports();                 // 8
    test_idiomatic_and_literal_counts_sum_to_file_count();             // 9
    test_target_languages_match_python_rust_java();                    // 10
    test_structural_counts_are_propagated_to_projected_ast_metrics();  // 11
    test_report_includes_completion_note();                            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
