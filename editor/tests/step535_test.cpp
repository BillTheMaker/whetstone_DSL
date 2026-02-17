// Step 535: Symbol Selector API (12 tests)

#include "SymbolSelectorAPI.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TaskitemContract contract() {
    TaskitemContract c;
    c.id = "ti-535";
    c.nodeId = "n-1";
    c.language = "cpp";
    c.allowedTargets = {"Function"};
    c.allowedOps = {"rename"};
    c.allowedSymbols = {"render", "EditorState", "value", "MAX_COUNT", "std", "fieldA"};
    c.forbiddenSymbols = {"unsafeGlobal"};
    c.expectedDiagnosticsAdd = {"none"};
    return c;
}

static ScopeSnapshot richScope() {
    ScopeSnapshot s;
    s.supported = true;
    s.candidates = {
        {"render", "Function", 2},
        {"EditorState", "Type", 0},
        {"value", "Variable", 3},
        {"MAX_COUNT", "Constant", 0},
        {"std", "Namespace", 0},
        {"fieldA", "Field", 2},
        {"unsafeGlobal", "Variable", 0}
    };
    return s;
}

void test_supported_scope_returns_candidates() {
    TEST(supported_scope_returns_candidates);
    auto r = SymbolSelectorAPI::select(contract(), richScope());
    CHECK(r.supported, "scope should be supported");
    CHECK(!r.candidates.empty(), "candidates expected");
    PASS();
}

void test_unsupported_scope_returns_not_supported() {
    TEST(unsupported_scope_returns_not_supported);
    ScopeSnapshot s;
    s.supported = false;
    auto r = SymbolSelectorAPI::select(contract(), s);
    CHECK(!r.supported, "unsupported scope expected");
    CHECK(r.candidates.empty(), "no candidates expected");
    PASS();
}

void test_forbidden_symbols_are_not_returned() {
    TEST(forbidden_symbols_are_not_returned);
    auto r = SymbolSelectorAPI::select(contract(), richScope());
    for (const auto& c : r.candidates) {
        CHECK(c.name != "unsafeGlobal", "forbidden symbol should be excluded");
    }
    PASS();
}

void test_contract_allowed_symbols_only() {
    TEST(contract_allowed_symbols_only);
    auto c = contract();
    c.allowedSymbols = {"render"};
    auto r = SymbolSelectorAPI::select(c, richScope());
    CHECK(r.candidates.size() == 1, "only one symbol should remain");
    CHECK(r.candidates[0].name == "render", "render should remain");
    PASS();
}

void test_function_category_filter() {
    TEST(function_category_filter);
    auto r = SymbolSelectorAPI::select(contract(), richScope(), {"function"});
    CHECK(r.candidates.size() == 1, "one function expected");
    CHECK(r.candidates[0].name == "render", "render should match function category");
    PASS();
}

void test_type_category_filter() {
    TEST(type_category_filter);
    auto r = SymbolSelectorAPI::select(contract(), richScope(), {"type"});
    CHECK(r.candidates.size() == 1, "one type expected");
    CHECK(r.candidates[0].name == "EditorState", "type category mismatch");
    PASS();
}

void test_field_category_filter() {
    TEST(field_category_filter);
    auto r = SymbolSelectorAPI::select(contract(), richScope(), {"field"});
    CHECK(r.candidates.size() == 1, "one field expected");
    CHECK(r.candidates[0].name == "fieldA", "field category mismatch");
    PASS();
}

void test_module_category_filter() {
    TEST(module_category_filter);
    auto r = SymbolSelectorAPI::select(contract(), richScope(), {"module"});
    CHECK(r.candidates.size() == 1, "one module expected");
    CHECK(r.candidates[0].name == "std", "module category mismatch");
    PASS();
}

void test_constant_category_filter() {
    TEST(constant_category_filter);
    auto r = SymbolSelectorAPI::select(contract(), richScope(), {"constant"});
    CHECK(r.candidates.size() == 1, "one constant expected");
    CHECK(r.candidates[0].name == "MAX_COUNT", "constant category mismatch");
    PASS();
}

void test_multiple_category_filter() {
    TEST(multiple_category_filter);
    auto r = SymbolSelectorAPI::select(contract(), richScope(), {"function", "type"});
    CHECK(r.candidates.size() == 2, "two category matches expected");
    PASS();
}

void test_candidates_sorted_by_scope_depth_then_name() {
    TEST(candidates_sorted_by_scope_depth_then_name);
    auto r = SymbolSelectorAPI::select(contract(), richScope());
    CHECK(r.candidates.size() >= 2, "at least two candidates required");
    CHECK(r.candidates[0].scopeDepth >= r.candidates[1].scopeDepth,
          "depth sort expected");
    PASS();
}

void test_duplicate_symbol_names_are_deduped() {
    TEST(duplicate_symbol_names_are_deduped);
    auto s = richScope();
    s.candidates.push_back({"value", "Variable", 1});
    auto r = SymbolSelectorAPI::select(contract(), s);
    int count = 0;
    for (const auto& c : r.candidates) if (c.name == "value") ++count;
    CHECK(count == 1, "duplicate symbol names should dedupe");
    PASS();
}

int main() {
    std::cout << "Step 535: Symbol Selector API\n";

    test_supported_scope_returns_candidates();         // 1
    test_unsupported_scope_returns_not_supported();    // 2
    test_forbidden_symbols_are_not_returned();         // 3
    test_contract_allowed_symbols_only();              // 4
    test_function_category_filter();                   // 5
    test_type_category_filter();                       // 6
    test_field_category_filter();                      // 7
    test_module_category_filter();                     // 8
    test_constant_category_filter();                   // 9
    test_multiple_category_filter();                   // 10
    test_candidates_sorted_by_scope_depth_then_name(); // 11
    test_duplicate_symbol_names_are_deduped();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
