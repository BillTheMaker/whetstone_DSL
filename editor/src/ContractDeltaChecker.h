#pragma once
// Step 531: Contract Delta Checker

#include <set>
#include <string>
#include <vector>

#include "ConstraintViolationDiagnostics.h"

struct ContractDeltaInput {
    TaskitemContract contract;
    std::vector<std::string> diagnosticsBefore;
    std::vector<std::string> diagnosticsAfter;
    std::vector<std::string> requiredSymbolsAfter;
    std::vector<std::string> actualSymbolsAfter;
    bool strictUnexpectedDiagnosticDrift = true;
};

struct ContractDeltaResult {
    bool valid = false;
    ConstraintDiagnosticPacket diagnostics;
};

class ContractDeltaChecker {
public:
    static ContractDeltaResult validate(const ContractDeltaInput& input) {
        ContractDeltaResult result;

        auto before = asSet(input.diagnosticsBefore);
        auto after = asSet(input.diagnosticsAfter);
        auto added = setDifference(after, before);
        auto removed = setDifference(before, after);

        validateExpectedAdds(input.contract.expectedDiagnosticsAdd,
                             added,
                             result.diagnostics.violations);
        validateExpectedRemoves(input.contract.expectedDiagnosticsRemove,
                                removed,
                                result.diagnostics.violations);

        if (input.strictUnexpectedDiagnosticDrift) {
            validateUnexpectedDrift(input.contract.expectedDiagnosticsAdd,
                                    input.contract.expectedDiagnosticsRemove,
                                    added,
                                    removed,
                                    result.diagnostics.violations);
        }

        validateRequiredSymbols(input.requiredSymbolsAfter,
                                input.actualSymbolsAfter,
                                result.diagnostics.violations);

        result.valid = result.diagnostics.violations.empty();
        result.diagnostics.ok = result.valid;
        result.diagnostics.recommendedAction = result.valid ? "proceed" : "escalate";
        return result;
    }

private:
    static void validateExpectedAdds(const std::vector<std::string>& expectedAdds,
                                     const std::set<std::string>& added,
                                     std::vector<ConstraintViolation>& violations) {
        for (const auto& expected : asSet(expectedAdds)) {
            if (added.count(expected) == 0) {
                violations.push_back({
                    "missing_expected_diagnostic_add",
                    "Expected diagnostic add did not occur",
                    expected,
                    false
                });
            }
        }
    }

    static void validateExpectedRemoves(const std::vector<std::string>& expectedRemoves,
                                        const std::set<std::string>& removed,
                                        std::vector<ConstraintViolation>& violations) {
        for (const auto& expected : asSet(expectedRemoves)) {
            if (removed.count(expected) == 0) {
                violations.push_back({
                    "missing_expected_diagnostic_remove",
                    "Expected diagnostic removal did not occur",
                    expected,
                    false
                });
            }
        }
    }

    static void validateUnexpectedDrift(const std::vector<std::string>& expectedAdds,
                                        const std::vector<std::string>& expectedRemoves,
                                        const std::set<std::string>& added,
                                        const std::set<std::string>& removed,
                                        std::vector<ConstraintViolation>& violations) {
        auto expectedAddSet = asSet(expectedAdds);
        auto expectedRemoveSet = asSet(expectedRemoves);

        for (const auto& d : added) {
            if (expectedAddSet.count(d) == 0) {
                violations.push_back({
                    "unexpected_diagnostic_add",
                    "Unexpected diagnostic added by edit",
                    d,
                    false
                });
            }
        }

        for (const auto& d : removed) {
            if (expectedRemoveSet.count(d) == 0) {
                violations.push_back({
                    "unexpected_diagnostic_remove",
                    "Unexpected diagnostic removed by edit",
                    d,
                    false
                });
            }
        }
    }

    static void validateRequiredSymbols(const std::vector<std::string>& requiredSymbols,
                                        const std::vector<std::string>& actualSymbols,
                                        std::vector<ConstraintViolation>& violations) {
        auto actual = asSet(actualSymbols);
        for (const auto& required : asSet(requiredSymbols)) {
            if (actual.count(required) == 0) {
                violations.push_back({
                    "postcondition_symbol_missing",
                    "Required post-condition symbol missing after apply",
                    required,
                    false
                });
            }
        }
    }

    static std::set<std::string> asSet(const std::vector<std::string>& values) {
        return std::set<std::string>(values.begin(), values.end());
    }

    static std::set<std::string> setDifference(const std::set<std::string>& a,
                                               const std::set<std::string>& b) {
        std::set<std::string> out;
        for (const auto& v : a) {
            if (b.count(v) == 0) out.insert(v);
        }
        return out;
    }
};
