#pragma once
// Step 538: Phase 28a Integration

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ArgumentShapeValidator.h"
#include "ConstrainedExecutionTelemetry.h"
#include "OperationSelectorAPI.h"
#include "SymbolSelectorAPI.h"

using json = nlohmann::json;

struct Phase28aRequest {
    TaskitemContract contract;
    std::string nodeKind;
    ScopeSnapshot scope;
    std::string requestedOperation;
    std::string requestedSymbol;
    json args;
    int constrainedTokenCount = 0;
    int baselineTokenCount = 0;
};

struct Phase28aResult {
    bool menuReady = false;
    bool executed = false;
    std::vector<std::string> errors;
    OperationSelectionResult operationMenu;
    SymbolSelectionResult symbolMenu;
    TelemetrySummary telemetrySummary;
};

class Phase28aIntegration {
public:
    static Phase28aResult run(const Phase28aRequest& request,
                              const LegalOperationGraph& graph,
                              ConstrainedExecutionTelemetry& telemetry) {
        Phase28aResult result;

        result.operationMenu = OperationSelectorAPI::select(
            request.contract, request.nodeKind, request.scope, graph);
        result.symbolMenu = SymbolSelectorAPI::select(request.contract, request.scope);

        result.menuReady = result.operationMenu.supported && result.symbolMenu.supported;

        TelemetryEvent event;
        event.taskitemId = request.contract.id;
        event.candidateOperationCount = (int)result.operationMenu.candidates.size();
        event.candidateSymbolCount = (int)result.symbolMenu.candidates.size();
        event.baselineTokenCount = request.baselineTokenCount;
        event.constrainedTokenCount = request.constrainedTokenCount;

        if (!result.menuReady) {
            result.errors.push_back("menu_unavailable");
            event.rejectionReasons.push_back("menu_unavailable");
            telemetry.record(event);
            result.telemetrySummary = telemetry.summarize();
            return result;
        }

        if (!hasOperation(result.operationMenu, request.requestedOperation)) {
            result.errors.push_back("operation_not_in_legal_menu");
            event.rejectionReasons.push_back("operation_not_in_legal_menu");
            telemetry.record(event);
            result.telemetrySummary = telemetry.summarize();
            return result;
        }

        if (!hasSymbol(result.symbolMenu, request.requestedSymbol)) {
            result.errors.push_back("symbol_not_in_legal_menu");
            event.rejectionReasons.push_back("symbol_not_in_legal_menu");
            telemetry.record(event);
            result.telemetrySummary = telemetry.summarize();
            return result;
        }

        auto shape = ArgumentShapeValidator::validate(request.contract,
                                                      request.requestedOperation,
                                                      request.requestedSymbol,
                                                      request.args);
        if (!shape.valid) {
            result.errors.insert(result.errors.end(), shape.errors.begin(), shape.errors.end());
            event.rejectionReasons.push_back("invalid_argument_shape");
            telemetry.record(event);
            result.telemetrySummary = telemetry.summarize();
            return result;
        }

        event.selectedOperation = request.requestedOperation;
        event.success = true;
        telemetry.record(event);

        result.executed = true;
        result.telemetrySummary = telemetry.summarize();
        return result;
    }

private:
    static bool hasOperation(const OperationSelectionResult& menu,
                             const std::string& operation) {
        for (const auto& c : menu.candidates) {
            if (c.operation == operation) return true;
        }
        return false;
    }

    static bool hasSymbol(const SymbolSelectionResult& menu,
                          const std::string& symbol) {
        for (const auto& c : menu.candidates) {
            if (c.name == symbol) return true;
        }
        return false;
    }
};
