#pragma once
// Step 530: Post-Apply Structural Gate

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "ConstraintViolationDiagnostics.h"

struct PostApplyInput {
    bool regionReparsed = false;
    int parseErrorsAfter = 0;
    std::vector<std::string> symbolsBefore;
    std::vector<std::string> symbolsAfter;
    std::vector<std::string> allowedSymbolDrift;
};

struct PostApplyResult {
    bool valid = false;
    std::vector<std::string> addedSymbols;
    std::vector<std::string> removedSymbols;
    ConstraintDiagnosticPacket diagnostics;
};

class PostApplyStructuralGate {
public:
    static PostApplyResult validate(const PostApplyInput& input) {
        PostApplyResult result;

        if (!input.regionReparsed) {
            result.diagnostics.violations.push_back({
                "region_not_reparsed",
                "Touched region was not reparsed after apply",
                "regionReparsed",
                false
            });
        }

        if (input.parseErrorsAfter > 0) {
            result.diagnostics.violations.push_back({
                "ast_integrity_failure",
                "AST parse reported errors after apply",
                "parseErrorsAfter",
                false
            });
        }

        computeSymbolDiff(input, result.addedSymbols, result.removedSymbols);
        addUnexpectedDriftViolations(input.allowedSymbolDrift,
                                     result.addedSymbols,
                                     result.removedSymbols,
                                     result.diagnostics.violations);

        dedupe(result.diagnostics.violations);
        result.valid = result.diagnostics.violations.empty();
        result.diagnostics.ok = result.valid;
        result.diagnostics.recommendedAction = result.valid ? "proceed" : "escalate";
        return result;
    }

private:
    static void computeSymbolDiff(const PostApplyInput& input,
                                  std::vector<std::string>& added,
                                  std::vector<std::string>& removed) {
        auto before = asSet(input.symbolsBefore);
        auto after = asSet(input.symbolsAfter);

        for (const auto& s : after) {
            if (before.count(s) == 0) added.push_back(s);
        }
        for (const auto& s : before) {
            if (after.count(s) == 0) removed.push_back(s);
        }
    }

    static void addUnexpectedDriftViolations(
        const std::vector<std::string>& allowedDrift,
        const std::vector<std::string>& added,
        const std::vector<std::string>& removed,
        std::vector<ConstraintViolation>& violations) {
        auto allowed = asSet(allowedDrift);
        for (const auto& symbol : added) {
            if (allowed.count(symbol) == 0) {
                violations.push_back({
                    "unexpected_symbol_drift_add",
                    "Unexpected symbol added after apply",
                    symbol,
                    false
                });
            }
        }
        for (const auto& symbol : removed) {
            if (allowed.count(symbol) == 0) {
                violations.push_back({
                    "unexpected_symbol_drift_remove",
                    "Unexpected symbol removed after apply",
                    symbol,
                    false
                });
            }
        }
    }

    static std::set<std::string> asSet(const std::vector<std::string>& symbols) {
        return std::set<std::string>(symbols.begin(), symbols.end());
    }

    static void dedupe(std::vector<ConstraintViolation>& violations) {
        std::vector<ConstraintViolation> out;
        for (const auto& v : violations) {
            bool exists = false;
            for (const auto& current : out) {
                if (current.code == v.code && current.field == v.field) {
                    exists = true;
                    break;
                }
            }
            if (!exists) out.push_back(v);
        }
        violations.swap(out);
    }
};
