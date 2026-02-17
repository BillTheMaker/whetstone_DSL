// Step 530: Post-Apply Structural Gate (12 tests)

#include "PostApplyStructuralGate.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasCode(const PostApplyResult& result, const std::string& code) {
    for (const auto& v : result.diagnostics.violations) {
        if (v.code == code) return true;
    }
    return false;
}

static int countCode(const PostApplyResult& result, const std::string& code) {
    int count = 0;
    for (const auto& v : result.diagnostics.violations) {
        if (v.code == code) ++count;
    }
    return count;
}

void test_clean_reparse_with_no_drift_passes() {
    TEST(clean_reparse_with_no_drift_passes);
    PostApplyInput input{true, 0, {"cursor", "count"}, {"cursor", "count"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(result.valid, "clean post-apply state should pass");
    CHECK(result.diagnostics.recommendedAction == "proceed", "should proceed");
    PASS();
}

void test_parse_errors_after_apply_fail_integrity_gate() {
    TEST(parse_errors_after_apply_fail_integrity_gate);
    PostApplyInput input{true, 2, {"cursor"}, {"cursor"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(!result.valid, "parse errors should fail gate");
    CHECK(hasCode(result, "ast_integrity_failure"), "missing integrity violation");
    PASS();
}

void test_missing_reparse_fails_gate() {
    TEST(missing_reparse_fails_gate);
    PostApplyInput input{false, 0, {"cursor"}, {"cursor"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(!result.valid, "missing reparse should fail");
    CHECK(hasCode(result, "region_not_reparsed"), "missing reparse violation");
    PASS();
}

void test_unexpected_added_symbol_fails_gate() {
    TEST(unexpected_added_symbol_fails_gate);
    PostApplyInput input{true, 0, {"cursor"}, {"cursor", "ghost"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(!result.valid, "unexpected added symbol should fail");
    CHECK(hasCode(result, "unexpected_symbol_drift_add"), "missing add drift violation");
    PASS();
}

void test_unexpected_removed_symbol_fails_gate() {
    TEST(unexpected_removed_symbol_fails_gate);
    PostApplyInput input{true, 0, {"cursor", "count"}, {"cursor"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(!result.valid, "unexpected removed symbol should fail");
    CHECK(hasCode(result, "unexpected_symbol_drift_remove"), "missing remove drift violation");
    PASS();
}

void test_allowed_added_symbol_drift_passes() {
    TEST(allowed_added_symbol_drift_passes);
    PostApplyInput input{true, 0, {"cursor"}, {"cursor", "tempAlias"}, {"tempAlias"}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(result.valid, "allowed added drift should pass");
    PASS();
}

void test_allowed_removed_symbol_drift_passes() {
    TEST(allowed_removed_symbol_drift_passes);
    PostApplyInput input{true, 0, {"cursor", "tempAlias"}, {"cursor"}, {"tempAlias"}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(result.valid, "allowed removed drift should pass");
    PASS();
}

void test_duplicate_symbols_do_not_create_duplicate_violations() {
    TEST(duplicate_symbols_do_not_create_duplicate_violations);
    PostApplyInput input{true, 0, {"cursor"}, {"cursor", "ghost", "ghost"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(countCode(result, "unexpected_symbol_drift_add") == 1,
          "duplicate added symbol violations should dedupe");
    PASS();
}

void test_multiple_unexpected_drift_entries_are_aggregated() {
    TEST(multiple_unexpected_drift_entries_are_aggregated);
    PostApplyInput input{true, 0, {"cursor", "count"}, {"cursor", "newA", "newB"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(!result.valid, "multiple unexpected drift should fail");
    CHECK(result.addedSymbols.size() == 2, "expected two added symbols");
    CHECK(result.removedSymbols.size() == 1, "expected one removed symbol");
    PASS();
}

void test_empty_symbol_graphs_are_valid_when_reparse_is_clean() {
    TEST(empty_symbol_graphs_are_valid_when_reparse_is_clean);
    PostApplyInput input{true, 0, {}, {}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(result.valid, "empty symbol graphs with clean parse should pass");
    PASS();
}

void test_combined_reparse_and_drift_failures_report_all() {
    TEST(combined_reparse_and_drift_failures_report_all);
    PostApplyInput input{false, 1, {"cursor"}, {"ghost"}, {}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(hasCode(result, "region_not_reparsed"), "missing reparse violation");
    CHECK(hasCode(result, "ast_integrity_failure"), "missing integrity violation");
    CHECK(hasCode(result, "unexpected_symbol_drift_add"), "missing add drift violation");
    CHECK(hasCode(result, "unexpected_symbol_drift_remove"), "missing remove drift violation");
    CHECK(result.diagnostics.recommendedAction == "escalate", "combined failures should escalate");
    PASS();
}

void test_allowed_drift_with_clean_parse_proceeds() {
    TEST(allowed_drift_with_clean_parse_proceeds);
    PostApplyInput input{true, 0, {"cursor"}, {"cursor", "alias"}, {"alias"}};
    auto result = PostApplyStructuralGate::validate(input);
    CHECK(result.valid, "allowed drift should remain valid");
    CHECK(result.diagnostics.recommendedAction == "proceed", "allowed drift should proceed");
    PASS();
}

int main() {
    std::cout << "Step 530: Post-Apply Structural Gate\n";

    test_clean_reparse_with_no_drift_passes();                    // 1
    test_parse_errors_after_apply_fail_integrity_gate();          // 2
    test_missing_reparse_fails_gate();                            // 3
    test_unexpected_added_symbol_fails_gate();                    // 4
    test_unexpected_removed_symbol_fails_gate();                  // 5
    test_allowed_added_symbol_drift_passes();                     // 6
    test_allowed_removed_symbol_drift_passes();                   // 7
    test_duplicate_symbols_do_not_create_duplicate_violations();  // 8
    test_multiple_unexpected_drift_entries_are_aggregated();      // 9
    test_empty_symbol_graphs_are_valid_when_reparse_is_clean();   // 10
    test_combined_reparse_and_drift_failures_report_all();        // 11
    test_allowed_drift_with_clean_parse_proceeds();               // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
