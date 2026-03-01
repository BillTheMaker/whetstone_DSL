#pragma once
// Step 1881a: Token budget gate (GR-014).
// Closes GR-014: low-yield (token-bloated) runs had no hard enforcement.
// TokenBudgetGate uses ContextWindowOptimizer to estimate tokens and blocks
// when a generation output would exceed the allowed budget.

#include "ContextWindowOptimizer.h"

#include <string>

struct TokenBudgetResult {
    bool blocked = false;
    int estimatedTokens = 0;
    int budgetTokens = 0;
    float yieldRatio = 0.0f;  // output useful content / total tokens
};

class TokenBudgetGate {
public:
    static constexpr float kMinYieldRatio = 0.30f;  // at least 30% of tokens must be useful

    // Block if the output text exceeds the budget or yield is too low.
    static TokenBudgetResult evaluate(const std::string& outputText,
                                      int contextWindowTokens,
                                      int usefulContentTokens = -1) {
        TokenBudgetResult result;
        result.estimatedTokens = ContextWindowOptimizer::estimateTokens(outputText);
        result.budgetTokens    = ContextWindowOptimizer::budgetForContextWindow(contextWindowTokens);

        if (result.estimatedTokens > result.budgetTokens) {
            result.blocked = true;
            return result;
        }

        // Yield check: if caller provides useful content token count
        if (usefulContentTokens >= 0 && result.estimatedTokens > 0) {
            result.yieldRatio = static_cast<float>(usefulContentTokens) / result.estimatedTokens;
            if (result.yieldRatio < kMinYieldRatio) {
                result.blocked = true;
            }
        }
        return result;
    }
};
