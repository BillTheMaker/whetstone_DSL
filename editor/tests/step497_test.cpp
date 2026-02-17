// Step 497: Self-Hosting Metrics Report Tests (12 tests)

#include "SelfHostingMetricsReport.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static SelfHostingMetrics runNow() {
    return SelfHostingMetricsReport::generateFromCurrentState();
}

void test_report_populates_parse_coverage_metrics() {
    TEST(report_populates_parse_coverage_metrics);
    auto r = runNow();
    CHECK(r.filesTotal > 0, "filesTotal should be > 0");
    CHECK(r.filesParsed >= 0, "filesParsed should be >= 0");
    CHECK(r.parseCoveragePercent >= 0.0, "parse coverage should be non-negative");
    PASS();
}

void test_report_populates_annotation_metrics() {
    TEST(report_populates_annotation_metrics);
    auto r = runNow();
    CHECK(r.inferredAnnotations >= 0, "inferredAnnotations invalid");
    CHECK(r.annotationSignals >= 0, "annotationSignals invalid");
    CHECK(r.annotationSignalDensity >= 0.0, "annotation density invalid");
    PASS();
}

void test_report_populates_transpilation_metrics() {
    TEST(report_populates_transpilation_metrics);
    auto r = runNow();
    CHECK(r.averageTranspileConfidence >= 0.0f && r.averageTranspileConfidence <= 1.0f,
          "confidence out of range");
    CHECK(r.transpileReviewRequired >= 0, "review count invalid");
    PASS();
}

void test_report_populates_workflow_viability_metrics() {
    TEST(report_populates_workflow_viability_metrics);
    auto r = runNow();
    CHECK(r.workflowTasks >= 0, "workflow task count invalid");
    CHECK(r.workflowDeterministicValid >= 0, "workflow deterministic valid invalid");
    PASS();
}

void test_gap_analysis_flags_parse_coverage_below_target() {
    TEST(gap_analysis_flags_parse_coverage_below_target);
    CodebaseParseAudit p; p.totalFiles = 100; p.parsedFiles = 90;
    SelfAnnotationAuditReport a;
    SelfTranspileAuditReport t;
    ModernizationWorkflowReport w;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    bool found = false;
    for (const auto& g : r.gaps) if (g.find("Parse coverage below 95%") != std::string::npos) found = true;
    CHECK(found, "expected parse-coverage gap");
    PASS();
}

void test_gap_analysis_flags_low_annotation_density() {
    TEST(gap_analysis_flags_low_annotation_density);
    CodebaseParseAudit p; p.totalFiles = 10; p.parsedFiles = 10;
    SelfAnnotationAuditReport a; a.parsedFiles = 10; a.complexityCount = 1;
    SelfTranspileAuditReport t; t.averageConfidence = 0.9f;
    ModernizationWorkflowReport w; w.deterministicCount = 1; w.deterministicValidCount = 1;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    bool found = false;
    for (const auto& g : r.gaps) if (g.find("annotation signal density") != std::string::npos) found = true;
    CHECK(found, "expected annotation density gap");
    PASS();
}

void test_gap_analysis_flags_low_transpilation_confidence() {
    TEST(gap_analysis_flags_low_transpilation_confidence);
    CodebaseParseAudit p; p.totalFiles = 10; p.parsedFiles = 10;
    SelfAnnotationAuditReport a; a.parsedFiles = 10; a.complexityCount = 20;
    SelfTranspileAuditReport t; t.averageConfidence = 0.2f;
    ModernizationWorkflowReport w; w.deterministicCount = 1; w.deterministicValidCount = 1;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    bool found = false;
    for (const auto& g : r.gaps) if (g.find("transpilation confidence") != std::string::npos) found = true;
    CHECK(found, "expected transpilation confidence gap");
    PASS();
}

void test_gap_analysis_flags_nonviable_workflow() {
    TEST(gap_analysis_flags_nonviable_workflow);
    CodebaseParseAudit p; p.totalFiles = 10; p.parsedFiles = 10;
    SelfAnnotationAuditReport a; a.parsedFiles = 10; a.complexityCount = 20;
    SelfTranspileAuditReport t; t.averageConfidence = 0.9f;
    ModernizationWorkflowReport w; w.deterministicCount = 3; w.deterministicValidCount = 1;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    bool found = false;
    for (const auto& g : r.gaps) if (g.find("modernization outputs") != std::string::npos) found = true;
    CHECK(found, "expected workflow viability gap");
    PASS();
}

void test_no_blocking_gaps_message_when_all_thresholds_pass() {
    TEST(no_blocking_gaps_message_when_all_thresholds_pass);
    CodebaseParseAudit p; p.totalFiles = 10; p.parsedFiles = 10;
    SelfAnnotationAuditReport a; a.parsedFiles = 10; a.complexityCount = 20;
    SelfTranspileAuditReport t; t.averageConfidence = 0.9f;
    ModernizationWorkflowReport w; w.deterministicCount = 2; w.deterministicValidCount = 2;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    CHECK(r.gaps.size() == 1, "expected single non-blocking gap message");
    CHECK(r.gaps[0].find("No blocking gaps") != std::string::npos, "missing no-gap message");
    PASS();
}

void test_generated_report_includes_summary_note() {
    TEST(generated_report_includes_summary_note);
    auto r = runNow();
    CHECK(!r.notes.empty(), "expected notes");
    CHECK(r.notes[0].find("metrics report generated") != std::string::npos, "missing summary note");
    PASS();
}

void test_parse_coverage_percent_matches_ratio() {
    TEST(parse_coverage_percent_matches_ratio);
    CodebaseParseAudit p; p.totalFiles = 4; p.parsedFiles = 3;
    SelfAnnotationAuditReport a; a.parsedFiles = 4;
    SelfTranspileAuditReport t;
    ModernizationWorkflowReport w;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    CHECK(r.parseCoveragePercent > 74.9 && r.parseCoveragePercent < 75.1, "expected 75%");
    PASS();
}

void test_workflow_viable_true_when_all_deterministic_outputs_valid() {
    TEST(workflow_viable_true_when_all_deterministic_outputs_valid);
    CodebaseParseAudit p; p.totalFiles = 10; p.parsedFiles = 10;
    SelfAnnotationAuditReport a; a.parsedFiles = 10; a.complexityCount = 20;
    SelfTranspileAuditReport t; t.averageConfidence = 0.9f;
    ModernizationWorkflowReport w; w.deterministicCount = 4; w.deterministicValidCount = 4;
    auto r = SelfHostingMetricsReport::generate(p, a, t, w);
    CHECK(r.workflowViable, "workflow should be viable");
    PASS();
}

int main() {
    std::cout << "Step 497: Self-Hosting Metrics Report Tests\n";

    test_report_populates_parse_coverage_metrics();             // 1
    test_report_populates_annotation_metrics();                 // 2
    test_report_populates_transpilation_metrics();              // 3
    test_report_populates_workflow_viability_metrics();         // 4
    test_gap_analysis_flags_parse_coverage_below_target();      // 5
    test_gap_analysis_flags_low_annotation_density();           // 6
    test_gap_analysis_flags_low_transpilation_confidence();     // 7
    test_gap_analysis_flags_nonviable_workflow();               // 8
    test_no_blocking_gaps_message_when_all_thresholds_pass();   // 9
    test_generated_report_includes_summary_note();              // 10
    test_parse_coverage_percent_matches_ratio();                // 11
    test_workflow_viable_true_when_all_deterministic_outputs_valid(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
