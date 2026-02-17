// Step 526: Symbol Scope Extractor (12 tests)

#include "SymbolScopeExtractor.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::vector<ScopedSymbol> sampleCppFunctionSymbols() {
    return {
        {"render", "Function", 1},
        {"buffer", "Variable", 2},
        {"cursor", "Variable", 3},
        {"cursor", "Variable", 1},
        {"count", "Parameter", 3},
        {"EditorState", "Type", 0},
        {"std", "Namespace", 0},
        {"#include", "Import", 0}
    };
}

void test_cpp_function_snapshot_filters_by_allowed_kinds() {
    TEST(cpp_function_snapshot_filters_by_allowed_kinds);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols());
    CHECK(snapshot.supported, "cpp function should be supported");
    CHECK(snapshot.candidates.size() == 6, "only allowed symbol kinds should be included");
    PASS();
}

void test_snapshot_prefers_nearest_scope_for_same_symbol_name() {
    TEST(snapshot_prefers_nearest_scope_for_same_symbol_name);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols());
    int depth = -1;
    for (const auto& c : snapshot.candidates) {
        if (c.name == "cursor") depth = c.scopeDepth;
    }
    CHECK(depth == 3, "nearest symbol depth should be retained");
    PASS();
}

void test_unknown_language_is_unsupported() {
    TEST(unknown_language_is_unsupported);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("haskell", "Function", sampleCppFunctionSymbols());
    CHECK(!snapshot.supported, "unknown language should be unsupported");
    CHECK(snapshot.candidates.empty(), "unsupported snapshot should have no candidates");
    PASS();
}

void test_unknown_node_kind_is_unsupported() {
    TEST(unknown_node_kind_is_unsupported);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "LambdaMagic", sampleCppFunctionSymbols());
    CHECK(!snapshot.supported, "unknown node kind should be unsupported");
    PASS();
}

void test_unresolved_symbols_empty_for_in_scope_requests() {
    TEST(unresolved_symbols_empty_for_in_scope_requests);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols());
    auto unresolved = e.unresolvedSymbols(snapshot, {"cursor", "count", "EditorState"});
    CHECK(unresolved.empty(), "all requested symbols are in scope");
    PASS();
}

void test_unresolved_symbols_reports_out_of_scope_requests() {
    TEST(unresolved_symbols_reports_out_of_scope_requests);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols());
    auto unresolved = e.unresolvedSymbols(snapshot, {"cursor", "ghost", "unknownType"});
    CHECK(unresolved.size() == 2, "two symbols should be unresolved");
    CHECK(unresolved[0] == "ghost", "results should be sorted");
    CHECK(unresolved[1] == "unknownType", "results should include both unresolved names");
    PASS();
}

void test_can_apply_blocks_unresolved_symbol_requests() {
    TEST(can_apply_blocks_unresolved_symbol_requests);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols());
    CHECK(!e.canApply(snapshot, {"cursor", "ghost"}), "unresolved symbol should block apply");
    PASS();
}

void test_can_apply_false_when_snapshot_is_unsupported() {
    TEST(can_apply_false_when_snapshot_is_unsupported);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("zig", "Function", sampleCppFunctionSymbols());
    CHECK(!e.canApply(snapshot, {"cursor"}), "unsupported snapshot should block apply");
    PASS();
}

void test_forbidden_symbols_are_excluded_from_snapshot() {
    TEST(forbidden_symbols_are_excluded_from_snapshot);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols(), {"cursor"});
    auto unresolved = e.unresolvedSymbols(snapshot, {"cursor"});
    CHECK(unresolved.size() == 1, "forbidden symbol should be excluded");
    CHECK(unresolved[0] == "cursor", "forbidden symbol should be unresolved");
    PASS();
}

void test_register_rule_adds_new_language_mapping() {
    TEST(register_rule_adds_new_language_mapping);
    SymbolScopeExtractor e;
    e.registerRule("java", "Method", {"Variable", "Parameter"});
    auto snapshot = e.buildSnapshot("java", "Method", {{"value", "Variable", 1}});
    CHECK(snapshot.supported, "new rule should become supported");
    CHECK(snapshot.candidates.size() == 1, "registered rule should include matching kind");
    PASS();
}

void test_register_rule_overrides_existing_mapping() {
    TEST(register_rule_overrides_existing_mapping);
    SymbolScopeExtractor e;
    e.registerRule("cpp", "Function", {"Function"});
    auto snapshot = e.buildSnapshot("cpp", "Function", sampleCppFunctionSymbols());
    CHECK(snapshot.supported, "overridden rule should remain supported");
    CHECK(snapshot.candidates.size() == 1, "override should restrict allowed kinds");
    CHECK(snapshot.candidates[0].name == "render", "function should remain");
    PASS();
}

void test_empty_visible_symbol_set_yields_empty_candidates() {
    TEST(empty_visible_symbol_set_yields_empty_candidates);
    SymbolScopeExtractor e;
    auto snapshot = e.buildSnapshot("cpp", "Function", {});
    CHECK(snapshot.supported, "supported rule should remain supported");
    CHECK(snapshot.candidates.empty(), "empty input should produce empty candidates");
    PASS();
}

int main() {
    std::cout << "Step 526: Symbol Scope Extractor\n";

    test_cpp_function_snapshot_filters_by_allowed_kinds();          // 1
    test_snapshot_prefers_nearest_scope_for_same_symbol_name();     // 2
    test_unknown_language_is_unsupported();                         // 3
    test_unknown_node_kind_is_unsupported();                        // 4
    test_unresolved_symbols_empty_for_in_scope_requests();          // 5
    test_unresolved_symbols_reports_out_of_scope_requests();        // 6
    test_can_apply_blocks_unresolved_symbol_requests();             // 7
    test_can_apply_false_when_snapshot_is_unsupported();            // 8
    test_forbidden_symbols_are_excluded_from_snapshot();            // 9
    test_register_rule_adds_new_language_mapping();                 // 10
    test_register_rule_overrides_existing_mapping();                // 11
    test_empty_visible_symbol_set_yields_empty_candidates();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
