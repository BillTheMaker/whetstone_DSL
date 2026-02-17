// Step 534: Operation Selector API (12 tests)

#include "OperationSelectorAPI.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TaskitemContract makeContract() {
    TaskitemContract c;
    c.id = "ti-534";
    c.nodeId = "n-1";
    c.language = "cpp";
    c.allowedTargets = {"Function"};
    c.allowedOps = {"rename", "update", "extract", "inline", "delete"};
    c.allowedSymbols = {"cursor", "count"};
    c.forbiddenSymbols = {"unsafeGlobal"};
    c.expectedDiagnosticsAdd = {"none"};
    c.expectedDiagnosticsRemove = {};
    return c;
}

static ScopeSnapshot supportedScope() {
    ScopeSnapshot s;
    s.supported = true;
    s.candidates = {{"cursor", "Variable", 2}};
    return s;
}

void test_supported_node_returns_candidates() {
    TEST(supported_node_returns_candidates);
    LegalOperationGraph graph;
    auto r = OperationSelectorAPI::select(makeContract(), "Function", supportedScope(), graph);
    CHECK(r.supported, "function node should be supported");
    CHECK(!r.candidates.empty(), "candidates expected");
    PASS();
}

void test_unsupported_node_returns_not_supported() {
    TEST(unsupported_node_returns_not_supported);
    LegalOperationGraph graph;
    auto r = OperationSelectorAPI::select(makeContract(), "MacroBlock", supportedScope(), graph);
    CHECK(!r.supported, "unsupported node should not be supported");
    CHECK(r.candidates.empty(), "unsupported node should have no candidates");
    PASS();
}

void test_contract_filters_out_disallowed_ops() {
    TEST(contract_filters_out_disallowed_ops);
    LegalOperationGraph graph;
    auto c = makeContract();
    c.allowedOps = {"rename"};
    auto r = OperationSelectorAPI::select(c, "Function", supportedScope(), graph);
    CHECK(r.candidates.size() == 1, "only one op should remain");
    CHECK(r.candidates[0].operation == "rename", "rename should remain");
    PASS();
}

void test_candidates_sorted_by_score_descending() {
    TEST(candidates_sorted_by_score_descending);
    LegalOperationGraph graph;
    auto r = OperationSelectorAPI::select(makeContract(), "Function", supportedScope(), graph);
    CHECK(r.candidates.size() >= 2, "need at least two candidates");
    CHECK(r.candidates[0].score >= r.candidates[1].score, "candidates should be score-sorted");
    PASS();
}

void test_preferred_ops_receive_score_boost() {
    TEST(preferred_ops_receive_score_boost);
    LegalOperationGraph graph;
    auto r = OperationSelectorAPI::select(makeContract(), "Function", supportedScope(), graph,
                                          {"extract"});
    int extractScore = -1;
    int inlineScore = -1;
    for (const auto& c : r.candidates) {
        if (c.operation == "extract") extractScore = c.score;
        if (c.operation == "inline") inlineScore = c.score;
    }
    CHECK(extractScore > inlineScore, "preferred extract should outrank inline");
    PASS();
}

void test_scope_context_penalizes_when_empty() {
    TEST(scope_context_penalizes_when_empty);
    LegalOperationGraph graph;
    ScopeSnapshot weak;
    weak.supported = true;
    auto withScope = OperationSelectorAPI::select(makeContract(), "Function", supportedScope(), graph);
    auto weakScope = OperationSelectorAPI::select(makeContract(), "Function", weak, graph);
    CHECK(!withScope.candidates.empty() && !weakScope.candidates.empty(), "need candidates");
    CHECK(withScope.candidates[0].score > weakScope.candidates[0].score,
          "empty scope should lower top score");
    PASS();
}

void test_rename_and_update_get_low_risk_bonus() {
    TEST(rename_and_update_get_low_risk_bonus);
    LegalOperationGraph graph;
    auto c = makeContract();
    c.allowedOps = {"rename", "update", "delete"};
    auto r = OperationSelectorAPI::select(c, "Variable", supportedScope(), graph);
    int renameScore = -1;
    int deleteScore = -1;
    for (const auto& c : r.candidates) {
        if (c.operation == "rename") renameScore = c.score;
        if (c.operation == "delete") deleteScore = c.score;
    }
    CHECK(renameScore > deleteScore, "rename should outrank delete due to risk weighting");
    PASS();
}

void test_high_risk_delete_gets_penalty() {
    TEST(high_risk_delete_gets_penalty);
    LegalOperationGraph graph;
    auto c = makeContract();
    c.allowedOps = {"rename", "update", "delete"};
    auto r = OperationSelectorAPI::select(c, "Variable", supportedScope(), graph);
    for (const auto& c : r.candidates) {
        if (c.operation == "delete") {
            CHECK(c.score < 80, "delete should be strongly penalized");
            PASS();
            return;
        }
    }
    FAIL("delete candidate not found");
}

void test_reasons_include_preference_and_scope_tags() {
    TEST(reasons_include_preference_and_scope_tags);
    LegalOperationGraph graph;
    auto r = OperationSelectorAPI::select(makeContract(), "Function", supportedScope(), graph,
                                          {"rename"});
    for (const auto& c : r.candidates) {
        if (c.operation == "rename") {
            bool hasPreferred = false;
            bool hasScope = false;
            for (const auto& reason : c.reasons) {
                if (reason == "preferred_op") hasPreferred = true;
                if (reason == "symbols_in_scope") hasScope = true;
            }
            CHECK(hasPreferred, "preferred tag expected");
            CHECK(hasScope, "scope tag expected");
            PASS();
            return;
        }
    }
    FAIL("rename candidate not found");
}

void test_tie_breaker_orders_by_operation_name() {
    TEST(tie_breaker_orders_by_operation_name);
    LegalOperationGraph graph;
    auto c = makeContract();
    c.allowedOps = {"alpha", "beta"};
    // Override legal graph for deterministic equal base scores.
    graph.registerRule("cpp", "Function", {"alpha", "beta"});
    ScopeSnapshot weak;
    weak.supported = false;
    auto r = OperationSelectorAPI::select(c, "Function", weak, graph);
    CHECK(r.candidates.size() == 2, "two candidates expected");
    CHECK(r.candidates[0].operation == "alpha", "lexicographic tie-break expected");
    PASS();
}

void test_empty_allowed_ops_yields_no_candidates() {
    TEST(empty_allowed_ops_yields_no_candidates);
    LegalOperationGraph graph;
    auto c = makeContract();
    c.allowedOps.clear();
    auto r = OperationSelectorAPI::select(c, "Function", supportedScope(), graph);
    CHECK(r.supported, "node support should remain true");
    CHECK(r.candidates.empty(), "no allowed ops should yield no candidates");
    PASS();
}

void test_unknown_language_in_contract_not_supported() {
    TEST(unknown_language_in_contract_not_supported);
    LegalOperationGraph graph;
    auto c = makeContract();
    c.language = "haskell";
    auto r = OperationSelectorAPI::select(c, "Function", supportedScope(), graph);
    CHECK(!r.supported, "unknown language should not be supported");
    PASS();
}

int main() {
    std::cout << "Step 534: Operation Selector API\n";

    test_supported_node_returns_candidates();             // 1
    test_unsupported_node_returns_not_supported();        // 2
    test_contract_filters_out_disallowed_ops();           // 3
    test_candidates_sorted_by_score_descending();         // 4
    test_preferred_ops_receive_score_boost();             // 5
    test_scope_context_penalizes_when_empty();            // 6
    test_rename_and_update_get_low_risk_bonus();          // 7
    test_high_risk_delete_gets_penalty();                 // 8
    test_reasons_include_preference_and_scope_tags();     // 9
    test_tie_breaker_orders_by_operation_name();          // 10
    test_empty_allowed_ops_yields_no_candidates();        // 11
    test_unknown_language_in_contract_not_supported();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
