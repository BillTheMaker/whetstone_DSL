#pragma once
// Step 529: Pre-Apply Validation Gate

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ConstraintViolationDiagnostics.h"
#include "ConstraintSchemaFailurePacket.h"

using json = nlohmann::json;

struct CandidateEdit {
    std::string nodeKind;
    std::string operation;
    std::vector<std::string> referencedSymbols;
};

struct PreApplyValidationResult {
    bool allowed = false;
    bool mayMutateBuffer = false;
    TaskitemContract contract;
    ScopeSnapshot scopeSnapshot;
    ConstraintDiagnosticPacket diagnostics;
};

class PreApplyValidationGate {
public:
    static PreApplyValidationResult validate(const json& taskitemJson,
                                             const CandidateEdit& edit,
                                             const std::vector<ScopedSymbol>& visibleSymbols,
                                             const LegalOperationGraph& graph,
                                             const SymbolScopeExtractor& scopeExtractor) {
        PreApplyValidationResult result;

        auto schema = TypedTaskitemContractSchema::parseAndValidate(taskitemJson);
        if (!schema.valid) {
            result.diagnostics = makeUnderConstrainedContractPacket(schema.errors);
            return result;
        }
        result.contract = schema.contract;

        std::set<std::string> forbidden(result.contract.forbiddenSymbols.begin(),
                                        result.contract.forbiddenSymbols.end());
        result.scopeSnapshot = scopeExtractor.buildSnapshot(result.contract.language,
                                                            edit.nodeKind,
                                                            visibleSymbols,
                                                            forbidden);

        ConstraintCheckInput input;
        input.contract = result.contract;
        input.language = result.contract.language;
        input.nodeKind = edit.nodeKind;
        input.requestedOp = edit.operation;
        input.requestedSymbols = edit.referencedSymbols;
        input.scopeSnapshot = result.scopeSnapshot;

        result.diagnostics = ConstraintViolationDiagnostics::evaluate(input, graph, scopeExtractor);

        if (!targetAllowed(result.contract, edit.nodeKind)) {
            result.diagnostics.violations.push_back({
                "target_out_of_contract",
                "Target node kind is outside contract allowedTargets",
                edit.nodeKind,
                false
            });
            result.diagnostics.ok = false;
            result.diagnostics.recommendedAction = "escalate";
        }

        dedupe(result.diagnostics.violations);
        result.allowed = result.diagnostics.violations.empty();
        result.mayMutateBuffer = result.allowed;
        result.diagnostics.ok = result.allowed;
        if (result.allowed) {
            result.diagnostics.recommendedAction = "proceed";
        } else if (result.diagnostics.recommendedAction == "proceed") {
            result.diagnostics.recommendedAction = deriveAction(result.diagnostics.violations);
        }
        return result;
    }

private:
    static bool targetAllowed(const TaskitemContract& contract,
                              const std::string& nodeKind) {
        return std::find(contract.allowedTargets.begin(),
                         contract.allowedTargets.end(),
                         nodeKind) != contract.allowedTargets.end();
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

    static std::string deriveAction(const std::vector<ConstraintViolation>& violations) {
        for (const auto& v : violations) {
            if (!v.retryable) return "escalate";
        }
        return violations.empty() ? "proceed" : "retry";
    }
};
