// Step 529: Pre-Apply Validation Gate (12 tests)

#include "PreApplyValidationGate.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json validTaskitem() {
    return {
        {"id", "ti-529"},
        {"nodeId", "n-1"},
        {"language", "cpp"},
        {"allowedTargets", json::array({"Function"})},
        {"allowedOps", json::array({"rename", "update"})},
        {"allowedSymbols", json::array({"cursor", "count", "EditorState", "render", "ghost"})},
        {"forbiddenSymbols", json::array({"unsafeGlobal"})},
        {"expectedDiagnosticsAdd", json::array({"none"})},
        {"expectedDiagnosticsRemove", json::array()}
    };
}

static std::vector<ScopedSymbol> visibleSymbols() {
    return {
        {"cursor", "Variable", 3},
        {"count", "Parameter", 3},
        {"EditorState", "Type", 0},
        {"render", "Function", 1},
        {"unsafeGlobal", "Variable", 0}
    };
}

static bool hasCode(const PreApplyValidationResult& result,
                    const std::string& code) {
    for (const auto& v : result.diagnostics.violations) {
        if (v.code == code) return true;
    }
    return false;
}

void test_valid_edit_is_allowed_before_apply() {
    TEST(valid_edit_is_allowed_before_apply);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "rename", {"cursor", "count"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(result.allowed, "valid edit should be allowed");
    CHECK(result.mayMutateBuffer, "valid edit may mutate buffer");
    CHECK(result.diagnostics.recommendedAction == "proceed", "valid edit should proceed");
    PASS();
}

void test_illegal_operation_rejected_before_mutation() {
    TEST(illegal_operation_rejected_before_mutation);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "delete", {"cursor"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "illegal op should be rejected");
    CHECK(!result.mayMutateBuffer, "illegal op should block mutation");
    CHECK(hasCode(result, "illegal_operation"), "missing illegal operation diagnostic");
    PASS();
}

void test_out_of_scope_symbol_rejected_before_mutation() {
    TEST(out_of_scope_symbol_rejected_before_mutation);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "rename", {"ghost"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "out-of-scope symbol should be rejected");
    CHECK(!result.mayMutateBuffer, "out-of-scope symbol should block mutation");
    CHECK(hasCode(result, "out_of_scope_symbol"), "missing out-of-scope diagnostic");
    PASS();
}

void test_unsupported_context_rejected_before_mutation() {
    TEST(unsupported_context_rejected_before_mutation);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"MacroBlock", "rename", {"cursor"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "unsupported context should be rejected");
    CHECK(hasCode(result, "unsupported_context"), "missing unsupported context diagnostic");
    PASS();
}

void test_target_outside_allowed_targets_rejected() {
    TEST(target_outside_allowed_targets_rejected);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Class", "rename", {"cursor"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "target outside contract should be rejected");
    CHECK(hasCode(result, "target_out_of_contract"), "missing target contract diagnostic");
    PASS();
}

void test_forbidden_symbol_rejected() {
    TEST(forbidden_symbol_rejected);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "rename", {"unsafeGlobal"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "forbidden symbol should be rejected");
    CHECK(hasCode(result, "forbidden_symbol"), "missing forbidden symbol diagnostic");
    PASS();
}

void test_missing_schema_field_rejected() {
    TEST(missing_schema_field_rejected);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    auto item = validTaskitem();
    item.erase("allowedOps");
    CandidateEdit edit{"Function", "rename", {"cursor"}};
    auto result = PreApplyValidationGate::validate(item, edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "schema-invalid taskitem should be rejected");
    CHECK(hasCode(result, "under_constrained_contract"), "missing schema violation code");
    PASS();
}

void test_duplicate_allowed_ops_rejected_by_schema() {
    TEST(duplicate_allowed_ops_rejected_by_schema);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    auto item = validTaskitem();
    item["allowedOps"] = json::array({"rename", "rename"});
    CandidateEdit edit{"Function", "rename", {"cursor"}};
    auto result = PreApplyValidationGate::validate(item, edit, visibleSymbols(), graph, extractor);
    CHECK(!result.allowed, "duplicate allowedOps should fail schema gate");
    CHECK(hasCode(result, "under_constrained_contract"), "expected schema rejection diagnostic");
    PASS();
}

void test_retry_action_for_retryable_only_failure() {
    TEST(retry_action_for_retryable_only_failure);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "rename", {"ghost"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(result.diagnostics.recommendedAction == "retry", "retryable-only failure should retry");
    PASS();
}

void test_multiple_failures_are_aggregated() {
    TEST(multiple_failures_are_aggregated);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Class", "delete", {"unsafeGlobal"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(hasCode(result, "target_out_of_contract"), "missing target violation");
    CHECK(hasCode(result, "unsupported_context"), "missing unsupported context violation");
    CHECK(hasCode(result, "forbidden_symbol"), "missing forbidden symbol violation");
    PASS();
}

void test_empty_symbol_list_can_still_validate() {
    TEST(empty_symbol_list_can_still_validate);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "rename", {}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, visibleSymbols(), graph, extractor);
    CHECK(result.allowed, "empty symbol list should be valid for rename");
    PASS();
}

void test_forbidden_symbol_not_in_snapshot_still_reports_violation() {
    TEST(forbidden_symbol_not_in_snapshot_still_reports_violation);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    CandidateEdit edit{"Function", "rename", {"unsafeGlobal"}};
    auto result = PreApplyValidationGate::validate(validTaskitem(), edit, {
        {"cursor", "Variable", 3}
    }, graph, extractor);
    CHECK(hasCode(result, "forbidden_symbol"), "forbidden symbol should still be diagnosed");
    PASS();
}

int main() {
    std::cout << "Step 529: Pre-Apply Validation Gate\n";

    test_valid_edit_is_allowed_before_apply();                      // 1
    test_illegal_operation_rejected_before_mutation();              // 2
    test_out_of_scope_symbol_rejected_before_mutation();            // 3
    test_unsupported_context_rejected_before_mutation();            // 4
    test_target_outside_allowed_targets_rejected();                 // 5
    test_forbidden_symbol_rejected();                               // 6
    test_missing_schema_field_rejected();                           // 7
    test_duplicate_allowed_ops_rejected_by_schema();                // 8
    test_retry_action_for_retryable_only_failure();                 // 9
    test_multiple_failures_are_aggregated();                        // 10
    test_empty_symbol_list_can_still_validate();                    // 11
    test_forbidden_symbol_not_in_snapshot_still_reports_violation();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
