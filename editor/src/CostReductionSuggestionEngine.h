#pragma once
// Step 549: Suggestion Engine for Cost Reductions

#include <algorithm>
#include <string>
#include <vector>

struct CostReductionInput {
    int candidateOperationCount = 0;
    int candidateSymbolCount = 0;
    bool deterministicTemplateAvailable = false;
    bool batchingPossible = false;
    bool narrowContextPossible = false;
    int stepTokens = 0;
    int stepTokenCeiling = 0;
    int workflowTokens = 0;
    int workflowTokenCeiling = 0;
};

struct CostSuggestion {
    std::string type;       // template_substitution/batching/context_narrowing
    std::string rationale;
    int estimatedTokenSavings = 0;
    int priority = 0;
};

class CostReductionSuggestionEngine {
public:
    static std::vector<CostSuggestion> suggest(const CostReductionInput& input) {
        std::vector<CostSuggestion> out;
        const int stepOver = std::max(0, input.stepTokens - input.stepTokenCeiling);
        const int workflowOver = std::max(0, input.workflowTokens - input.workflowTokenCeiling);
        const int pressure = stepOver + workflowOver;

        if (input.deterministicTemplateAvailable) {
            CostSuggestion s;
            s.type = "template_substitution";
            s.rationale = "deterministic template can replace free-form generation";
            s.estimatedTokenSavings = std::max(20, input.candidateOperationCount * 8);
            s.priority = 80 + (pressure > 0 ? 10 : 0);
            out.push_back(s);
        }

        if (input.batchingPossible && input.candidateOperationCount >= 2) {
            CostSuggestion s;
            s.type = "batching";
            s.rationale = "multiple compatible operations can be executed in one pass";
            s.estimatedTokenSavings = std::max(15, input.candidateOperationCount * 5);
            s.priority = 70 + (pressure > 0 ? 15 : 0);
            out.push_back(s);
        }

        if (input.narrowContextPossible && input.candidateSymbolCount > 3) {
            CostSuggestion s;
            s.type = "context_narrowing";
            s.rationale = "reduce context width to contract-relevant symbols only";
            s.estimatedTokenSavings = std::max(10, (input.candidateSymbolCount - 3) * 6);
            s.priority = 65 + (pressure > 0 ? 20 : 0);
            out.push_back(s);
        }

        std::sort(out.begin(), out.end(), [](const CostSuggestion& a, const CostSuggestion& b) {
            if (a.priority != b.priority) return a.priority > b.priority;
            return a.estimatedTokenSavings > b.estimatedTokenSavings;
        });
        return out;
    }
};
