// Step 528: Phase 27a Integration (8 tests)

#include "ConstrainedTaskitemIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json validTaskitem() {
    return {
        {"id", "ti-528"},
        {"nodeId", "n-1"},
        {"language", "cpp"},
        {"allowedTargets", json::array({"Function"})},
        {"allowedOps", json::array({"rename", "update"})},
        {"allowedSymbols", json::array({"cursor", "count", "EditorState"})},
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

static bool hasCode(const ConstrainedExecutionResult& result,
                    const std::string& code) {
    for (const auto& v : result.diagnostics.violations) {
        if (v.code == code) return true;
    }
    return false;
}

void test_valid_taskitem_routes_and_executes() {
    TEST(valid_taskitem_routes_and_executes);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    ConstrainedExecutionRequest request{
        validTaskitem(), "Function", "rename", {"cursor", "count"}, visibleSymbols()
    };
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(result.routed, "request should route");
    CHECK(result.executed, "request should execute");
    CHECK(result.diagnostics.ok, "diagnostics should pass");
    PASS();
}

void test_missing_allowed_ops_is_rejected_as_under_constrained() {
    TEST(missing_allowed_ops_is_rejected_as_under_constrained);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    auto item = validTaskitem();
    item.erase("allowedOps");
    ConstrainedExecutionRequest request{item, "Function", "rename", {"cursor"}, visibleSymbols()};
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.routed, "under-constrained request should not route");
    CHECK(!result.executed, "under-constrained request should not execute");
    CHECK(hasCode(result, "under_constrained_contract"), "missing schema failure code");
    PASS();
}

void test_missing_diagnostics_delta_is_rejected() {
    TEST(missing_diagnostics_delta_is_rejected);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    auto item = validTaskitem();
    item["expectedDiagnosticsAdd"] = json::array();
    item["expectedDiagnosticsRemove"] = json::array();
    ConstrainedExecutionRequest request{item, "Function", "rename", {"cursor"}, visibleSymbols()};
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.routed, "missing diagnostics delta should not route");
    CHECK(hasCode(result, "under_constrained_contract"), "schema failure code expected");
    PASS();
}

void test_illegal_operation_fails_with_actionable_diagnostics() {
    TEST(illegal_operation_fails_with_actionable_diagnostics);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    ConstrainedExecutionRequest request{
        validTaskitem(), "Function", "delete", {"cursor"}, visibleSymbols()
    };
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.executed, "illegal op should block execution");
    CHECK(hasCode(result, "illegal_operation"), "illegal operation code missing");
    CHECK(result.diagnostics.recommendedAction == "escalate", "illegal op should escalate");
    PASS();
}

void test_out_of_scope_symbol_fails_with_retry_action() {
    TEST(out_of_scope_symbol_fails_with_retry_action);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    auto item = validTaskitem();
    item["allowedSymbols"].push_back("ghost");
    ConstrainedExecutionRequest request{
        item, "Function", "rename", {"ghost"}, visibleSymbols()
    };
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.executed, "out-of-scope symbol should block execution");
    CHECK(hasCode(result, "out_of_scope_symbol"), "out-of-scope code missing");
    CHECK(result.diagnostics.recommendedAction == "retry", "retryable-only failure should retry");
    PASS();
}

void test_forbidden_symbol_fails_with_escalation() {
    TEST(forbidden_symbol_fails_with_escalation);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    ConstrainedExecutionRequest request{
        validTaskitem(), "Function", "rename", {"unsafeGlobal"}, visibleSymbols()
    };
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.executed, "forbidden symbol should block execution");
    CHECK(hasCode(result, "forbidden_symbol"), "forbidden symbol code missing");
    CHECK(result.diagnostics.recommendedAction == "escalate", "forbidden symbol should escalate");
    PASS();
}

void test_unsupported_context_fails_fast() {
    TEST(unsupported_context_fails_fast);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    ConstrainedExecutionRequest request{
        validTaskitem(), "MacroBlock", "rename", {"cursor"}, visibleSymbols()
    };
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.executed, "unsupported context should block execution");
    CHECK(hasCode(result, "unsupported_context"), "unsupported context code missing");
    PASS();
}

void test_duplicate_allowed_ops_is_rejected_by_schema_gate() {
    TEST(duplicate_allowed_ops_is_rejected_by_schema_gate);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;

    auto item = validTaskitem();
    item["allowedOps"] = json::array({"rename", "rename"});
    ConstrainedExecutionRequest request{item, "Function", "rename", {"cursor"}, visibleSymbols()};
    auto result = ConstrainedTaskitemIntegration::run(request, graph, extractor);
    CHECK(!result.executed, "duplicate allowedOps should fail schema gate");
    CHECK(hasCode(result, "under_constrained_contract"), "schema gate should annotate failure");
    PASS();
}

int main() {
    std::cout << "Step 528: Phase 27a Integration\n";

    test_valid_taskitem_routes_and_executes();                      // 1
    test_missing_allowed_ops_is_rejected_as_under_constrained();    // 2
    test_missing_diagnostics_delta_is_rejected();                   // 3
    test_illegal_operation_fails_with_actionable_diagnostics();     // 4
    test_out_of_scope_symbol_fails_with_retry_action();             // 5
    test_forbidden_symbol_fails_with_escalation();                  // 6
    test_unsupported_context_fails_fast();                          // 7
    test_duplicate_allowed_ops_is_rejected_by_schema_gate();        // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
