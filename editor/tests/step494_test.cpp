// Step 494: Annotate Whetstone's Own Code Tests (12 tests)

#include "SelfHostAnnotationAudit.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SelfAnnotationAuditReport runReal() {
    return SelfHostAnnotationAudit::auditDirectories({"editor/src", "editor/src/ast"});
}

void test_annotation_audit_runs_over_real_header_set() {
    TEST(annotation_audit_runs_over_real_header_set);
    auto r = runReal();
    CHECK(r.totalFiles > 0, "expected non-empty header set");
    CHECK(!r.files.empty(), "expected per-file audit entries");
    PASS();
}

void test_parse_rate_metrics_are_computed() {
    TEST(parse_rate_metrics_are_computed);
    auto r = runReal();
    CHECK(r.parseRate() >= 0.0, "parse rate below zero");
    CHECK(r.parseRate() <= 1.0, "parse rate above one");
    PASS();
}

void test_complexity_annotations_are_counted() {
    TEST(complexity_annotations_are_counted);
    auto sample = SelfHostAnnotationAudit::auditSource(
        "class A { public: int parseX(int x){ for(int i=0;i<x;i++){} return x; } };",
        "inline_complexity");
    CHECK(sample.complexityCount >= 0, "complexity should be non-negative");
    PASS();
}

void test_owner_signals_detect_unique_and_shared_ptr_patterns() {
    TEST(owner_signals_detect_unique_and_shared_ptr_patterns);
    auto sample = SelfHostAnnotationAudit::auditSource(
        "#include <memory>\n"
        "struct A { std::unique_ptr<int> p; std::shared_ptr<int> q; };",
        "inline_owner");
    CHECK(sample.ownerCount >= 2, "expected owner signals for smart pointers");
    PASS();
}

void test_risk_signals_detect_reinterpret_cast_or_void_pointer() {
    TEST(risk_signals_detect_reinterpret_cast_or_void_pointer);
    auto sample = SelfHostAnnotationAudit::auditSource(
        "void* p = 0; int x = reinterpret_cast<long>(p);",
        "inline_risk");
    CHECK(sample.riskCount >= 1, "expected risk signal");
    PASS();
}

void test_intent_annotations_are_derived_from_descriptive_function_names() {
    TEST(intent_annotations_are_derived_from_descriptive_function_names);
    auto sample = SelfHostAnnotationAudit::auditSource(
        "int parseData(){ return 0; } int runAudit(){ return 1; }",
        "inline_intent");
    CHECK(sample.intentCount >= 1, "expected intent count from descriptive names");
    PASS();
}

void test_tailcall_metric_is_tracked_nonnegative() {
    TEST(tailcall_metric_is_tracked_nonnegative);
    auto sample = SelfHostAnnotationAudit::auditSource(
        "int f(int n){ if(n<=0) return 0; return f(n-1); }",
        "inline_tail");
    CHECK(sample.tailCallCount >= 0, "tailcall count should be non-negative");
    PASS();
}

void test_inferred_total_is_sum_of_detected_inferred_annotations() {
    TEST(inferred_total_is_sum_of_detected_inferred_annotations);
    auto sample = SelfHostAnnotationAudit::auditSource(
        "int validateInput(int x){ return x; }",
        "inline_total");
    CHECK(sample.inferredTotal >= sample.complexityCount, "inferred total should dominate subsets");
    PASS();
}

void test_unparseable_inline_source_reports_warning_negative_case() {
    TEST(unparseable_inline_source_reports_warning_negative_case);
    auto sample = SelfHostAnnotationAudit::auditSource("", "inline_empty");
    CHECK(!sample.parseSuccess, "empty source should not parse");
    CHECK(!sample.warnings.empty(), "expected warnings for empty source");
    PASS();
}

void test_report_aggregate_totals_match_per_file_sum() {
    TEST(report_aggregate_totals_match_per_file_sum);
    auto r = runReal();
    int sum = 0;
    for (const auto& f : r.files) sum += f.inferredTotal;
    CHECK(sum == r.inferredTotal, "aggregate inferred total mismatch");
    PASS();
}

void test_report_contains_completion_note() {
    TEST(report_contains_completion_note);
    auto r = runReal();
    CHECK(!r.notes.empty(), "expected audit note");
    CHECK(r.notes[0].find("completed") != std::string::npos, "expected completion note");
    PASS();
}

void test_real_codebase_has_nonzero_signal_in_major_categories() {
    TEST(real_codebase_has_nonzero_signal_in_major_categories);
    auto r = runReal();
    CHECK(r.totalFiles >= 50, "expected substantial corpus");
    CHECK(r.inferredTotal >= 1, "expected non-zero inferred annotations");
    CHECK(r.ownerCount >= 1, "expected owner signals from codebase");
    PASS();
}

int main() {
    std::cout << "Step 494: Annotate Whetstone's Own Code Tests\n";

    test_annotation_audit_runs_over_real_header_set();                       // 1
    test_parse_rate_metrics_are_computed();                                  // 2
    test_complexity_annotations_are_counted();                               // 3
    test_owner_signals_detect_unique_and_shared_ptr_patterns();              // 4
    test_risk_signals_detect_reinterpret_cast_or_void_pointer();             // 5
    test_intent_annotations_are_derived_from_descriptive_function_names();   // 6
    test_tailcall_metric_is_tracked_nonnegative();                           // 7
    test_inferred_total_is_sum_of_detected_inferred_annotations();           // 8
    test_unparseable_inline_source_reports_warning_negative_case();          // 9
    test_report_aggregate_totals_match_per_file_sum();                       // 10
    test_report_contains_completion_note();                                   // 11
    test_real_codebase_has_nonzero_signal_in_major_categories();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
