// Step 537: Constrained Execution Telemetry (12 tests)

#include "ConstrainedExecutionTelemetry.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TelemetryEvent event(const std::string& id,
                            int opCount,
                            int symCount,
                            const std::string& op,
                            const std::vector<std::string>& reasons,
                            int constrained,
                            int baseline,
                            bool success) {
    TelemetryEvent e;
    e.taskitemId = id;
    e.candidateOperationCount = opCount;
    e.candidateSymbolCount = symCount;
    e.selectedOperation = op;
    e.rejectionReasons = reasons;
    e.constrainedTokenCount = constrained;
    e.baselineTokenCount = baseline;
    e.success = success;
    return e;
}

void test_empty_summary_is_zeroed() {
    TEST(empty_summary_is_zeroed);
    ConstrainedExecutionTelemetry t;
    auto s = t.summarize();
    CHECK(s.events == 0, "no events expected");
    CHECK(s.constrainedTokens == 0 && s.baselineTokens == 0, "token counts should be zero");
    PASS();
}

void test_record_increments_event_count() {
    TEST(record_increments_event_count);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 3, 5, "rename", {}, 20, 30, true));
    CHECK(t.size() == 1, "size should be one");
    PASS();
}

void test_summary_counts_success_and_failure() {
    TEST(summary_counts_success_and_failure);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 3, 5, "rename", {}, 20, 30, true));
    t.record(event("b", 2, 4, "", {"out_of_scope_symbol"}, 22, 35, false));
    auto s = t.summarize();
    CHECK(s.successCount == 1, "success count mismatch");
    CHECK(s.failureCount == 1, "failure count mismatch");
    PASS();
}

void test_average_candidate_counts_are_computed() {
    TEST(average_candidate_counts_are_computed);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 4, 10, "rename", {}, 10, 20, true));
    t.record(event("b", 2, 6, "update", {}, 10, 20, true));
    auto s = t.summarize();
    CHECK(s.avgCandidateOperationCount == 3, "avg op count mismatch");
    CHECK(s.avgCandidateSymbolCount == 8, "avg symbol count mismatch");
    PASS();
}

void test_token_savings_computation() {
    TEST(token_savings_computation);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 4, 10, "rename", {}, 15, 30, true));
    t.record(event("b", 2, 6, "update", {}, 20, 40, true));
    auto s = t.summarize();
    CHECK(s.constrainedTokens == 35, "constrained total mismatch");
    CHECK(s.baselineTokens == 70, "baseline total mismatch");
    CHECK(s.tokenSavings == 35, "token savings mismatch");
    PASS();
}

void test_token_savings_ratio_computation() {
    TEST(token_savings_ratio_computation);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 4, 10, "rename", {}, 25, 50, true));
    auto s = t.summarize();
    CHECK(s.tokenSavingsRatio > 0.49 && s.tokenSavingsRatio < 0.51,
          "token ratio should be about 0.5");
    PASS();
}

void test_rejection_histogram_counts_reasons() {
    TEST(rejection_histogram_counts_reasons);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 2, 5, "", {"out_of_scope_symbol", "illegal_operation"}, 20, 30, false));
    t.record(event("b", 2, 5, "", {"out_of_scope_symbol"}, 22, 35, false));
    auto s = t.summarize();
    CHECK(s.rejectionHistogram["out_of_scope_symbol"] == 2, "out_of_scope count mismatch");
    CHECK(s.rejectionHistogram["illegal_operation"] == 1, "illegal_operation count mismatch");
    PASS();
}

void test_operation_selection_histogram_counts_ops() {
    TEST(operation_selection_histogram_counts_ops);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 3, 5, "rename", {}, 10, 20, true));
    t.record(event("b", 3, 5, "rename", {}, 10, 20, true));
    t.record(event("c", 3, 5, "update", {}, 10, 20, true));
    auto s = t.summarize();
    CHECK(s.opSelectionHistogram["rename"] == 2, "rename histogram mismatch");
    CHECK(s.opSelectionHistogram["update"] == 1, "update histogram mismatch");
    PASS();
}

void test_failed_events_with_empty_selected_op_do_not_count_selection() {
    TEST(failed_events_with_empty_selected_op_do_not_count_selection);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 1, 1, "", {"unsupported_context"}, 5, 10, false));
    auto s = t.summarize();
    CHECK(s.opSelectionHistogram.empty(), "empty op should not be counted");
    PASS();
}

void test_baseline_zero_avoids_ratio_division_issue() {
    TEST(baseline_zero_avoids_ratio_division_issue);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 1, 1, "rename", {}, 5, 0, true));
    auto s = t.summarize();
    CHECK(s.tokenSavingsRatio == 0.0, "ratio should be zero when baseline total is zero");
    PASS();
}

void test_negative_token_savings_supported() {
    TEST(negative_token_savings_supported);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 1, 1, "rename", {}, 60, 50, true));
    auto s = t.summarize();
    CHECK(s.tokenSavings == -10, "negative savings should be represented");
    PASS();
}

void test_mixed_events_aggregate_all_metrics() {
    TEST(mixed_events_aggregate_all_metrics);
    ConstrainedExecutionTelemetry t;
    t.record(event("a", 4, 7, "rename", {}, 20, 40, true));
    t.record(event("b", 2, 4, "", {"out_of_scope_symbol"}, 15, 30, false));
    t.record(event("c", 3, 8, "update", {}, 18, 36, true));
    auto s = t.summarize();
    CHECK(s.events == 3, "event count mismatch");
    CHECK(s.successCount == 2 && s.failureCount == 1, "success/failure mismatch");
    CHECK(s.rejectionHistogram["out_of_scope_symbol"] == 1, "rejection histogram mismatch");
    PASS();
}

int main() {
    std::cout << "Step 537: Constrained Execution Telemetry\n";

    test_empty_summary_is_zeroed();                              // 1
    test_record_increments_event_count();                        // 2
    test_summary_counts_success_and_failure();                   // 3
    test_average_candidate_counts_are_computed();                // 4
    test_token_savings_computation();                            // 5
    test_token_savings_ratio_computation();                      // 6
    test_rejection_histogram_counts_reasons();                   // 7
    test_operation_selection_histogram_counts_ops();             // 8
    test_failed_events_with_empty_selected_op_do_not_count_selection(); // 9
    test_baseline_zero_avoids_ratio_division_issue();            // 10
    test_negative_token_savings_supported();                     // 11
    test_mixed_events_aggregate_all_metrics();                   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
