// Step 527: Constraint Violation Diagnostics (12 tests)

#include "ConstraintViolationDiagnostics.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TaskitemContract makeContract() {
    json j = {
        {"id", "ti-527"},
        {"nodeId", "n-1"},
        {"language", "cpp"},
        {"allowedTargets", json::array({"Function"})},
        {"allowedOps", json::array({"rename", "update"})},
        {"allowedSymbols", json::array({"cursor", "count", "EditorState"})},
        {"forbiddenSymbols", json::array({"unsafeGlobal"})},
        {"expectedDiagnosticsAdd", json::array({"none"})},
        {"expectedDiagnosticsRemove", json::array()}
    };
    auto result = TypedTaskitemContractSchema::parseAndValidate(j);
    return result.contract;
}

static ScopeSnapshot makeSnapshot(const SymbolScopeExtractor& extractor) {
    return extractor.buildSnapshot("cpp", "Function", {
        {"cursor", "Variable", 3},
        {"count", "Parameter", 3},
        {"EditorState", "Type", 0},
        {"render", "Function", 1}
    });
}

static bool hasCode(const ConstraintDiagnosticPacket& packet,
                    const std::string& code) {
    for (const auto& v : packet.violations) {
        if (v.code == code) return true;
    }
    return false;
}

static int countCode(const ConstraintDiagnosticPacket& packet,
                     const std::string& code) {
    int count = 0;
    for (const auto& v : packet.violations) {
        if (v.code == code) ++count;
    }
    return count;
}

void test_valid_request_produces_no_violations() {
    TEST(valid_request_produces_no_violations);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {"cursor", "count"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(packet.ok, "packet should be ok");
    CHECK(packet.recommendedAction == "proceed", "action should be proceed");
    CHECK(packet.violations.empty(), "no violations expected");
    PASS();
}

void test_contract_forbidden_operation_is_reported() {
    TEST(contract_forbidden_operation_is_reported);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "extract",
                               {"cursor"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "contract_forbidden_operation"), "missing violation");
    CHECK(packet.recommendedAction == "escalate", "non-retryable should escalate");
    PASS();
}

void test_illegal_graph_operation_is_reported() {
    TEST(illegal_graph_operation_is_reported);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "delete",
                               {"cursor"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "illegal_operation"), "illegal op violation expected");
    PASS();
}

void test_unsupported_context_is_reported() {
    TEST(unsupported_context_is_reported);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "MysteryNode", "rename",
                               {"cursor"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "unsupported_context"), "unsupported context violation expected");
    CHECK(packet.recommendedAction == "escalate", "unsupported context should escalate");
    PASS();
}

void test_out_of_scope_symbol_is_reported() {
    TEST(out_of_scope_symbol_is_reported);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {"ghost"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "out_of_scope_symbol"), "missing out-of-scope violation");
    PASS();
}

void test_retry_action_for_retryable_only_violations() {
    TEST(retry_action_for_retryable_only_violations);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    auto contract = makeContract();
    contract.allowedSymbols.push_back("ghost");
    ConstraintCheckInput input{contract, "cpp", "Function", "rename",
                               {"ghost"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(packet.recommendedAction == "retry", "retryable-only violations should retry");
    CHECK(countCode(packet, "out_of_scope_symbol") == 1, "expected out-of-scope violation");
    PASS();
}

void test_forbidden_symbol_is_reported() {
    TEST(forbidden_symbol_is_reported);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {"unsafeGlobal"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "forbidden_symbol"), "forbidden symbol violation expected");
    CHECK(packet.recommendedAction == "escalate", "forbidden symbol should escalate");
    PASS();
}

void test_contract_disallowed_symbol_is_reported() {
    TEST(contract_disallowed_symbol_is_reported);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {"render"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "contract_disallowed_symbol"), "contract disallowed violation expected");
    PASS();
}

void test_duplicate_symbol_requests_are_deduplicated() {
    TEST(duplicate_symbol_requests_are_deduplicated);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {"ghost", "ghost", "ghost"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(countCode(packet, "out_of_scope_symbol") == 1, "out-of-scope violations should dedupe");
    CHECK(countCode(packet, "contract_disallowed_symbol") == 1,
          "contract disallowed violations should dedupe");
    PASS();
}

void test_multiple_violation_types_are_aggregated() {
    TEST(multiple_violation_types_are_aggregated);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "delete",
                               {"ghost", "unsafeGlobal"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(hasCode(packet, "illegal_operation"), "missing illegal operation");
    CHECK(hasCode(packet, "contract_forbidden_operation"), "missing forbidden operation");
    CHECK(hasCode(packet, "out_of_scope_symbol"), "missing out-of-scope symbol");
    CHECK(hasCode(packet, "forbidden_symbol"), "missing forbidden symbol");
    PASS();
}

void test_to_json_contains_machine_readable_fields() {
    TEST(to_json_contains_machine_readable_fields);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {"ghost"}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    auto encoded = packet.toJson();
    CHECK(encoded.contains("ok"), "json should contain ok");
    CHECK(encoded.contains("recommendedAction"), "json should contain recommendedAction");
    CHECK(encoded.contains("violations"), "json should contain violations");
    CHECK(encoded["violations"].is_array(), "violations should be array");
    PASS();
}

void test_empty_symbol_request_is_valid() {
    TEST(empty_symbol_request_is_valid);
    LegalOperationGraph graph;
    SymbolScopeExtractor extractor;
    ConstraintCheckInput input{makeContract(), "cpp", "Function", "rename",
                               {}, makeSnapshot(extractor)};
    auto packet = ConstraintViolationDiagnostics::evaluate(input, graph, extractor);
    CHECK(packet.ok, "empty symbol set should be valid");
    CHECK(packet.recommendedAction == "proceed", "empty request should proceed");
    PASS();
}

int main() {
    std::cout << "Step 527: Constraint Violation Diagnostics\n";

    test_valid_request_produces_no_violations();              // 1
    test_contract_forbidden_operation_is_reported();          // 2
    test_illegal_graph_operation_is_reported();               // 3
    test_unsupported_context_is_reported();                   // 4
    test_out_of_scope_symbol_is_reported();                   // 5
    test_retry_action_for_retryable_only_violations();        // 6
    test_forbidden_symbol_is_reported();                      // 7
    test_contract_disallowed_symbol_is_reported();            // 8
    test_duplicate_symbol_requests_are_deduplicated();        // 9
    test_multiple_violation_types_are_aggregated();           // 10
    test_to_json_contains_machine_readable_fields();          // 11
    test_empty_symbol_request_is_valid();                     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
