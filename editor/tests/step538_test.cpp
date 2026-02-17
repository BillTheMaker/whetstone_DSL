// Step 538: Phase 28a Integration (8 tests)

#include "Phase28aIntegration.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TaskitemContract contract() {
    TaskitemContract c;
    c.id = "ti-538";
    c.nodeId = "n-1";
    c.language = "cpp";
    c.allowedTargets = {"Function"};
    c.allowedOps = {"rename", "update"};
    c.allowedSymbols = {"cursor", "count"};
    c.forbiddenSymbols = {"unsafeGlobal"};
    c.expectedDiagnosticsAdd = {"none"};
    return c;
}

static ScopeSnapshot scope() {
    ScopeSnapshot s;
    s.supported = true;
    s.candidates = {
        {"cursor", "Variable", 3},
        {"count", "Parameter", 3},
        {"unsafeGlobal", "Variable", 0}
    };
    return s;
}

void test_valid_legal_choice_executes_successfully() {
    TEST(valid_legal_choice_executes_successfully);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "rename", "cursor",
                        json{{"newName", "cursor2"}}, 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(result.menuReady, "menu should be ready");
    CHECK(result.executed, "valid legal choice should execute");
    PASS();
}

void test_operation_not_in_menu_is_rejected() {
    TEST(operation_not_in_menu_is_rejected);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "delete", "cursor",
                        json{{"confirm", true}}, 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(!result.executed, "illegal op should not execute");
    CHECK(!result.errors.empty() && result.errors[0] == "operation_not_in_legal_menu",
          "operation rejection expected");
    PASS();
}

void test_symbol_not_in_menu_is_rejected() {
    TEST(symbol_not_in_menu_is_rejected);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "rename", "ghost",
                        json{{"newName", "ghost2"}}, 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(!result.executed, "illegal symbol should not execute");
    CHECK(!result.errors.empty() && result.errors[0] == "symbol_not_in_legal_menu",
          "symbol rejection expected");
    PASS();
}

void test_invalid_argument_shape_is_rejected() {
    TEST(invalid_argument_shape_is_rejected);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "rename", "cursor",
                        json::object(), 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(!result.executed, "invalid args should not execute");
    CHECK(!result.errors.empty(), "shape errors expected");
    PASS();
}

void test_menu_unavailable_when_scope_unsupported() {
    TEST(menu_unavailable_when_scope_unsupported);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    auto s = scope();
    s.supported = false;
    Phase28aRequest req{contract(), "Function", s, "rename", "cursor",
                        json{{"newName", "cursor2"}}, 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(!result.menuReady, "menu should be unavailable");
    CHECK(!result.executed, "execution should be blocked");
    PASS();
}

void test_telemetry_records_candidate_breadth() {
    TEST(telemetry_records_candidate_breadth);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "rename", "cursor",
                        json{{"newName", "cursor2"}}, 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(result.telemetrySummary.events == 1, "one telemetry event expected");
    CHECK(result.telemetrySummary.avgCandidateOperationCount > 0,
          "operation candidate breadth expected");
    CHECK(result.telemetrySummary.avgCandidateSymbolCount > 0,
          "symbol candidate breadth expected");
    PASS();
}

void test_telemetry_shows_token_savings() {
    TEST(telemetry_shows_token_savings);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "rename", "cursor",
                        json{{"newName", "cursor2"}}, 18, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(result.telemetrySummary.tokenSavings == 22, "token savings should be recorded");
    PASS();
}

void test_rejection_path_records_failure_telemetry() {
    TEST(rejection_path_records_failure_telemetry);
    LegalOperationGraph graph;
    ConstrainedExecutionTelemetry telemetry;
    Phase28aRequest req{contract(), "Function", scope(), "delete", "cursor",
                        json{{"confirm", true}}, 20, 40};
    auto result = Phase28aIntegration::run(req, graph, telemetry);
    CHECK(result.telemetrySummary.failureCount == 1, "failure telemetry expected");
    CHECK(result.telemetrySummary.rejectionHistogram["operation_not_in_legal_menu"] == 1,
          "rejection reason histogram mismatch");
    PASS();
}

int main() {
    std::cout << "Step 538: Phase 28a Integration\n";

    test_valid_legal_choice_executes_successfully();   // 1
    test_operation_not_in_menu_is_rejected();          // 2
    test_symbol_not_in_menu_is_rejected();             // 3
    test_invalid_argument_shape_is_rejected();         // 4
    test_menu_unavailable_when_scope_unsupported();    // 5
    test_telemetry_records_candidate_breadth();        // 6
    test_telemetry_shows_token_savings();              // 7
    test_rejection_path_records_failure_telemetry();   // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
