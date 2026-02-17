#pragma once
// Step 527: Constraint Violation Diagnostics

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "LegalOperationGraph.h"
#include "SymbolScopeExtractor.h"
#include "TypedTaskitemContractSchema.h"

using json = nlohmann::json;

struct ConstraintViolation {
    std::string code;
    std::string message;
    std::string field;
    bool retryable = false;
};

struct ConstraintDiagnosticPacket {
    bool ok = true;
    std::string recommendedAction = "proceed";
    std::vector<ConstraintViolation> violations;

    json toJson() const {
        json out;
        out["ok"] = ok;
        out["recommendedAction"] = recommendedAction;
        out["violations"] = json::array();
        for (const auto& v : violations) {
            out["violations"].push_back({
                {"code", v.code},
                {"message", v.message},
                {"field", v.field},
                {"retryable", v.retryable}
            });
        }
        return out;
    }
};

struct ConstraintCheckInput {
    TaskitemContract contract;
    std::string language;
    std::string nodeKind;
    std::string requestedOp;
    std::vector<std::string> requestedSymbols;
    ScopeSnapshot scopeSnapshot;
};

class ConstraintViolationDiagnostics {
public:
    static ConstraintDiagnosticPacket evaluate(
        const ConstraintCheckInput& input,
        const LegalOperationGraph& graph,
        const SymbolScopeExtractor& scopeExtractor) {
        ConstraintDiagnosticPacket packet;

        addOperationViolations(packet, input, graph);
        addSymbolViolations(packet, input, scopeExtractor);

        deduplicate(packet.violations);
        packet.ok = packet.violations.empty();
        packet.recommendedAction = decideAction(packet.violations);
        return packet;
    }

private:
    static void addOperationViolations(ConstraintDiagnosticPacket& packet,
                                       const ConstraintCheckInput& input,
                                       const LegalOperationGraph& graph) {
        auto legal = graph.allowedOps(input.language, input.nodeKind);
        if (!legal.supported) {
            packet.violations.push_back({
                "unsupported_context",
                "No legal operation mapping for language/nodeKind",
                input.language + "::" + input.nodeKind,
                false
            });
            return;
        }

        if (!graph.opAllowed(input.language, input.nodeKind, input.requestedOp)) {
            packet.violations.push_back({
                "illegal_operation",
                "Requested operation is illegal for this AST node",
                input.requestedOp,
                false
            });
        }

        if (!TypedTaskitemContractSchema::opAllowed(input.contract, input.requestedOp)) {
            packet.violations.push_back({
                "contract_forbidden_operation",
                "Requested operation is outside taskitem contract",
                input.requestedOp,
                false
            });
        }
    }

    static void addSymbolViolations(ConstraintDiagnosticPacket& packet,
                                    const ConstraintCheckInput& input,
                                    const SymbolScopeExtractor& scopeExtractor) {
        auto unresolved = scopeExtractor.unresolvedSymbols(
            input.scopeSnapshot, input.requestedSymbols);
        for (const auto& symbol : unresolved) {
            packet.violations.push_back({
                "out_of_scope_symbol",
                "Requested symbol is not in extracted scope snapshot",
                symbol,
                true
            });
        }

        for (const auto& symbol : input.requestedSymbols) {
            if (isForbidden(input.contract, symbol)) {
                packet.violations.push_back({
                    "forbidden_symbol",
                    "Requested symbol is explicitly forbidden by contract",
                    symbol,
                    false
                });
                continue;
            }
            if (!TypedTaskitemContractSchema::symbolAllowed(input.contract, symbol)) {
                packet.violations.push_back({
                    "contract_disallowed_symbol",
                    "Requested symbol is outside contract allowedSymbols",
                    symbol,
                    false
                });
            }
        }
    }

    static bool isForbidden(const TaskitemContract& contract,
                            const std::string& symbol) {
        return std::find(contract.forbiddenSymbols.begin(),
                         contract.forbiddenSymbols.end(),
                         symbol) != contract.forbiddenSymbols.end();
    }

    static void deduplicate(std::vector<ConstraintViolation>& violations) {
        std::vector<ConstraintViolation> out;
        for (const auto& v : violations) {
            bool exists = false;
            for (const auto& existing : out) {
                if (existing.code == v.code && existing.field == v.field) {
                    exists = true;
                    break;
                }
            }
            if (!exists) out.push_back(v);
        }
        violations.swap(out);
    }

    static std::string decideAction(const std::vector<ConstraintViolation>& violations) {
        if (violations.empty()) return "proceed";
        for (const auto& v : violations) {
            if (!v.retryable) return "escalate";
        }
        return "retry";
    }
};
