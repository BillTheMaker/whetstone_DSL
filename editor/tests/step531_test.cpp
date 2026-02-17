// Step 531: Contract Delta Checker (12 tests)

#include "ContractDeltaChecker.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasCode(const ContractDeltaResult& result, const std::string& code) {
    for (const auto& v : result.diagnostics.violations) {
        if (v.code == code) return true;
    }
    return false;
}

static TaskitemContract baseContract() {
    TaskitemContract c;
    c.id = "ti-531";
    c.nodeId = "n-1";
    c.language = "cpp";
    c.allowedTargets = {"Function"};
    c.allowedOps = {"rename"};
    c.allowedSymbols = {"cursor", "count"};
    c.forbiddenSymbols = {"unsafeGlobal"};
    c.expectedDiagnosticsAdd = {"diag.new_warning"};
    c.expectedDiagnosticsRemove = {"diag.old_warning"};
    return c;
}

void test_expected_add_and_remove_pass() {
    TEST(expected_add_and_remove_pass);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning"};
    input.diagnosticsAfter = {"diag.new_warning"};
    input.requiredSymbolsAfter = {"cursor"};
    input.actualSymbolsAfter = {"cursor", "count"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(result.valid, "expected delta should pass");
    PASS();
}

void test_missing_expected_add_fails() {
    TEST(missing_expected_add_fails);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning"};
    input.diagnosticsAfter = {};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(hasCode(result, "missing_expected_diagnostic_add"), "missing add violation expected");
    PASS();
}

void test_missing_expected_remove_fails() {
    TEST(missing_expected_remove_fails);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning", "diag.other"};
    input.diagnosticsAfter = {"diag.new_warning", "diag.old_warning"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(hasCode(result, "missing_expected_diagnostic_remove"), "missing remove violation expected");
    PASS();
}

void test_unexpected_add_fails_in_strict_mode() {
    TEST(unexpected_add_fails_in_strict_mode);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning"};
    input.diagnosticsAfter = {"diag.new_warning", "diag.extra"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(hasCode(result, "unexpected_diagnostic_add"), "unexpected add violation expected");
    PASS();
}

void test_unexpected_remove_fails_in_strict_mode() {
    TEST(unexpected_remove_fails_in_strict_mode);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning", "diag.extra"};
    input.diagnosticsAfter = {"diag.new_warning"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(hasCode(result, "unexpected_diagnostic_remove"), "unexpected remove violation expected");
    PASS();
}

void test_non_strict_mode_allows_unexpected_drift() {
    TEST(non_strict_mode_allows_unexpected_drift);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.strictUnexpectedDiagnosticDrift = false;
    input.diagnosticsBefore = {"diag.old_warning", "diag.extra_old"};
    input.diagnosticsAfter = {"diag.new_warning", "diag.extra_new"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(result.valid, "non-strict mode should allow unexpected drift");
    PASS();
}

void test_required_post_symbol_present_passes() {
    TEST(required_post_symbol_present_passes);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning"};
    input.diagnosticsAfter = {"diag.new_warning"};
    input.requiredSymbolsAfter = {"cursor"};
    input.actualSymbolsAfter = {"cursor", "count"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(result.valid, "required symbol present should pass");
    PASS();
}

void test_required_post_symbol_missing_fails() {
    TEST(required_post_symbol_missing_fails);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning"};
    input.diagnosticsAfter = {"diag.new_warning"};
    input.requiredSymbolsAfter = {"cursor", "count"};
    input.actualSymbolsAfter = {"cursor"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(hasCode(result, "postcondition_symbol_missing"), "missing postcondition symbol violation");
    PASS();
}

void test_duplicate_diagnostics_do_not_break_delta_math() {
    TEST(duplicate_diagnostics_do_not_break_delta_math);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning", "diag.old_warning"};
    input.diagnosticsAfter = {"diag.new_warning", "diag.new_warning"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(result.valid, "duplicate diagnostics should normalize by set semantics");
    PASS();
}

void test_empty_delta_expectations_can_pass() {
    TEST(empty_delta_expectations_can_pass);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.contract.expectedDiagnosticsAdd.clear();
    input.contract.expectedDiagnosticsRemove.clear();
    input.diagnosticsBefore = {"diag.keep"};
    input.diagnosticsAfter = {"diag.keep"};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(result.valid, "no expected delta and no drift should pass");
    PASS();
}

void test_multiple_failures_are_aggregated() {
    TEST(multiple_failures_are_aggregated);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning", "diag.x"};
    input.diagnosticsAfter = {"diag.old_warning", "diag.y"};
    input.requiredSymbolsAfter = {"cursor"};
    input.actualSymbolsAfter = {};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(hasCode(result, "missing_expected_diagnostic_add"), "missing expected add violation");
    CHECK(hasCode(result, "missing_expected_diagnostic_remove"), "missing expected remove violation");
    CHECK(hasCode(result, "unexpected_diagnostic_add"), "missing unexpected add violation");
    CHECK(hasCode(result, "unexpected_diagnostic_remove"), "missing unexpected remove violation");
    CHECK(hasCode(result, "postcondition_symbol_missing"), "missing postcondition symbol violation");
    PASS();
}

void test_failed_delta_check_escalates() {
    TEST(failed_delta_check_escalates);
    ContractDeltaInput input;
    input.contract = baseContract();
    input.diagnosticsBefore = {"diag.old_warning"};
    input.diagnosticsAfter = {};
    auto result = ContractDeltaChecker::validate(input);
    CHECK(!result.valid, "failed delta should be invalid");
    CHECK(result.diagnostics.recommendedAction == "escalate", "failed delta should escalate");
    PASS();
}

int main() {
    std::cout << "Step 531: Contract Delta Checker\n";

    test_expected_add_and_remove_pass();                    // 1
    test_missing_expected_add_fails();                      // 2
    test_missing_expected_remove_fails();                   // 3
    test_unexpected_add_fails_in_strict_mode();             // 4
    test_unexpected_remove_fails_in_strict_mode();          // 5
    test_non_strict_mode_allows_unexpected_drift();         // 6
    test_required_post_symbol_present_passes();             // 7
    test_required_post_symbol_missing_fails();              // 8
    test_duplicate_diagnostics_do_not_break_delta_math();   // 9
    test_empty_delta_expectations_can_pass();               // 10
    test_multiple_failures_are_aggregated();                // 11
    test_failed_delta_check_escalates();                    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
