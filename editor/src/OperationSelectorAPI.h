#pragma once
// Step 534: Operation Selector API

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "LegalOperationGraph.h"
#include "SymbolScopeExtractor.h"
#include "TypedTaskitemContractSchema.h"

struct OperationCandidate {
    std::string operation;
    int score = 0;
    std::vector<std::string> reasons;
};

struct OperationSelectionResult {
    bool supported = false;
    std::vector<OperationCandidate> candidates;
};

class OperationSelectorAPI {
public:
    static OperationSelectionResult select(
        const TaskitemContract& contract,
        const std::string& nodeKind,
        const ScopeSnapshot& snapshot,
        const LegalOperationGraph& graph,
        const std::vector<std::string>& preferredOps = {}) {
        OperationSelectionResult result;
        auto legal = graph.allowedOps(contract.language, nodeKind);
        if (!legal.supported) return result;

        result.supported = true;
        auto preferred = asSet(preferredOps);
        for (const auto& op : legal.operations) {
            if (!TypedTaskitemContractSchema::opAllowed(contract, op)) continue;
            OperationCandidate c;
            c.operation = op;
            c.score = baseScore(op);
            if (preferred.count(op) > 0) {
                c.score += 40;
                c.reasons.push_back("preferred_op");
            }
            if (snapshot.supported && !snapshot.candidates.empty()) {
                c.score += 15;
                c.reasons.push_back("symbols_in_scope");
            } else {
                c.score -= 10;
                c.reasons.push_back("weak_scope_context");
            }
            if (op == "rename" || op == "update") {
                c.score += 10;
                c.reasons.push_back("low_constraint_risk");
            }
            if (op == "delete") {
                c.score -= 25;
                c.reasons.push_back("high_constraint_risk");
            }
            result.candidates.push_back(c);
        }

        std::sort(result.candidates.begin(), result.candidates.end(),
                  [](const OperationCandidate& a, const OperationCandidate& b) {
                      if (a.score != b.score) return a.score > b.score;
                      return a.operation < b.operation;
                  });
        return result;
    }

private:
    static int baseScore(const std::string& op) {
        if (op == "rename") return 100;
        if (op == "update") return 95;
        if (op == "extract") return 85;
        if (op == "inline") return 75;
        if (op == "insert") return 70;
        if (op == "reorder") return 65;
        if (op == "delete") return 50;
        return 40;
    }

    static std::set<std::string> asSet(const std::vector<std::string>& values) {
        return std::set<std::string>(values.begin(), values.end());
    }
};
