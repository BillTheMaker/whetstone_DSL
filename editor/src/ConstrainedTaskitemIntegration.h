#pragma once
// Step 528: Phase 27a Integration

#include <set>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ConstraintViolationDiagnostics.h"
#include "ConstraintSchemaFailurePacket.h"

using json = nlohmann::json;

struct ConstrainedExecutionRequest {
    json taskitemJson;
    std::string nodeKind;
    std::string requestedOp;
    std::vector<std::string> requestedSymbols;
    std::vector<ScopedSymbol> visibleSymbols;
};

struct ConstrainedExecutionResult {
    bool routed = false;
    bool executed = false;
    TaskitemContract contract;
    ScopeSnapshot scopeSnapshot;
    ConstraintDiagnosticPacket diagnostics;
    std::vector<std::string> schemaErrors;
};

class ConstrainedTaskitemIntegration {
public:
    static ConstrainedExecutionResult run(const ConstrainedExecutionRequest& request,
                                          const LegalOperationGraph& graph,
                                          const SymbolScopeExtractor& scopeExtractor) {
        ConstrainedExecutionResult result;

        auto validation = TypedTaskitemContractSchema::parseAndValidate(request.taskitemJson);
        if (!validation.valid) {
            result.schemaErrors = validation.errors;
            result.diagnostics = makeUnderConstrainedContractPacket(validation.errors);
            return result;
        }

        result.contract = validation.contract;
        std::set<std::string> forbidden(result.contract.forbiddenSymbols.begin(),
                                        result.contract.forbiddenSymbols.end());

        result.scopeSnapshot = scopeExtractor.buildSnapshot(
            result.contract.language,
            request.nodeKind,
            request.visibleSymbols,
            forbidden);

        ConstraintCheckInput checkInput;
        checkInput.contract = result.contract;
        checkInput.language = result.contract.language;
        checkInput.nodeKind = request.nodeKind;
        checkInput.requestedOp = request.requestedOp;
        checkInput.requestedSymbols = request.requestedSymbols;
        checkInput.scopeSnapshot = result.scopeSnapshot;

        result.diagnostics = ConstraintViolationDiagnostics::evaluate(
            checkInput, graph, scopeExtractor);
        result.routed = result.diagnostics.ok;
        result.executed = result.diagnostics.ok;
        return result;
    }
};
