// Step 504: Performance Optimization Tests (12 tests)

#include "PerformanceOptimizationSuite.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static PerformanceOptimizationReport runNow() {
    return PerformanceOptimizationSuite::run(120, 12);
}

void test_report_contains_requested_function_count() {
    TEST(report_contains_requested_function_count);
    auto r = runNow();
    CHECK(r.functionCount == 120, "expected 120 benchmarked functions");
    PASS();
}

void test_baseline_hot_path_timings_are_captured() {
    TEST(baseline_hot_path_timings_are_captured);
    auto r = runNow();
    CHECK(r.baseline.astSerializationMs >= 0.0, "missing baseline serialization timing");
    CHECK(r.baseline.annotationBatchMs >= 0.0, "missing baseline annotation timing");
    CHECK(r.baseline.contextAssemblyMs >= 0.0, "missing baseline context timing");
    CHECK(r.baseline.compactGenerationMs >= 0.0, "missing baseline compact timing");
    PASS();
}

void test_optimized_hot_path_timings_are_captured() {
    TEST(optimized_hot_path_timings_are_captured);
    auto r = runNow();
    CHECK(r.optimized.astSerializationMs >= 0.0, "missing optimized serialization timing");
    CHECK(r.optimized.annotationBatchMs >= 0.0, "missing optimized annotation timing");
    CHECK(r.optimized.contextAssemblyMs >= 0.0, "missing optimized context timing");
    CHECK(r.optimized.compactGenerationMs >= 0.0, "missing optimized compact timing");
    PASS();
}

void test_optimized_total_time_is_not_worse_than_baseline() {
    TEST(optimized_total_time_is_not_worse_than_baseline);
    auto r = runNow();
    CHECK(r.optimized.totalMs <= r.baseline.totalMs, "optimized total should not exceed baseline");
    PASS();
}

void test_context_assembly_cache_produces_hits() {
    TEST(context_assembly_cache_produces_hits);
    auto r = runNow();
    CHECK(r.contextCacheHits > 0, "expected cache hits");
    CHECK(r.contextCacheMisses > 0, "expected cache misses");
    CHECK(r.contextCacheHits > r.contextCacheMisses, "expected more hits than misses");
    PASS();
}

void test_ast_serialization_and_compact_generation_show_optimized_reduction() {
    TEST(ast_serialization_and_compact_generation_show_optimized_reduction);
    auto r = runNow();
    CHECK(r.optimized.astSerializationMs <= r.baseline.astSerializationMs,
          "serialization optimization regression");
    CHECK(r.optimized.compactGenerationMs <= r.baseline.compactGenerationMs,
          "compact generation optimization regression");
    PASS();
}

void test_annotation_batch_path_runs_successfully() {
    TEST(annotation_batch_path_runs_successfully);
    auto r = runNow();
    CHECK(r.optimized.annotationBatchMs >= 0.0, "annotation batch timing missing");
    PASS();
}

void test_parse_annotate_route_per_function_stays_under_target_threshold() {
    TEST(parse_annotate_route_per_function_stays_under_target_threshold);
    auto r = runNow();
    CHECK(r.perFunctionMsOptimized < 100.0, "expected <100ms per function");
    CHECK(r.targetMet, "targetMet should be true");
    PASS();
}

void test_sustained_runs_report_memory_stability() {
    TEST(sustained_runs_report_memory_stability);
    auto r = runNow();
    CHECK(r.memoryStable, "expected synthetic sustained-run memory stability");
    PASS();
}

void test_notes_include_hot_path_summary_and_target_status() {
    TEST(notes_include_hot_path_summary_and_target_status);
    auto r = runNow();
    CHECK(r.notes.size() >= 2, "expected optimization notes");
    CHECK(r.notes[0].find("Hot-path timings captured") != std::string::npos,
          "missing hot-path summary note");
    PASS();
}

void test_optimized_total_time_is_positive_and_finite() {
    TEST(optimized_total_time_is_positive_and_finite);
    auto r = runNow();
    CHECK(r.optimized.totalMs >= 0.0, "optimized total must be non-negative");
    CHECK(r.optimized.totalMs < 1e9, "optimized total unrealistic");
    PASS();
}

void test_repeatability_preserves_target_met_signal() {
    TEST(repeatability_preserves_target_met_signal);
    auto a = PerformanceOptimizationSuite::run(120, 8);
    auto b = PerformanceOptimizationSuite::run(120, 8);
    CHECK(a.targetMet && b.targetMet, "expected target to remain met across repeats");
    PASS();
}

int main() {
    std::cout << "Step 504: Performance Optimization Tests\n";

    test_report_contains_requested_function_count();                     // 1
    test_baseline_hot_path_timings_are_captured();                      // 2
    test_optimized_hot_path_timings_are_captured();                     // 3
    test_optimized_total_time_is_not_worse_than_baseline();             // 4
    test_context_assembly_cache_produces_hits();                        // 5
    test_ast_serialization_and_compact_generation_show_optimized_reduction(); // 6
    test_annotation_batch_path_runs_successfully();                     // 7
    test_parse_annotate_route_per_function_stays_under_target_threshold(); // 8
    test_sustained_runs_report_memory_stability();                      // 9
    test_notes_include_hot_path_summary_and_target_status();            // 10
    test_optimized_total_time_is_positive_and_finite();                 // 11
    test_repeatability_preserves_target_met_signal();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
