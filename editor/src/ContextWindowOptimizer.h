#pragma once

#include "ModelProfileRegistry.h"

#include <algorithm>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

struct ContextPack {
    std::string instructions;
    std::string localContext;
    std::string fileContext;
    std::string projectContext;
};

struct OptimizedContext {
    std::string scope;  // local|file|project
    std::string context;
    int tokenEstimate = 0;
    int budgetTokens = 0;
    bool truncated = false;

    json toJson() const {
        return {
            {"scope", scope},
            {"context", context},
            {"tokenEstimate", tokenEstimate},
            {"budgetTokens", budgetTokens},
            {"truncated", truncated}
        };
    }
};

class ContextWindowOptimizer {
public:
    static int estimateTokens(const std::string& text) {
        if (text.empty()) return 0;
        int chars = (int)text.size();
        int estimate = chars / 4;
        return estimate > 0 ? estimate : 1;
    }

    static int budgetForContextWindow(int contextWindow) {
        if (contextWindow <= 0) return 512;
        // Reserve ~40% for output/system/tool wrappers.
        int budget = (int)(contextWindow * 0.6);
        if (budget < 512) budget = 512;
        return budget;
    }

    static OptimizedContext optimize(const ContextPack& pack,
                                     const ModelProfile& profile) {
        OptimizedContext out;
        out.budgetTokens = budgetForContextWindow(profile.contextWindow);

        std::string chosenScope = chooseScope(profile.contextWindow);
        std::string base = pack.instructions;
        if (!base.empty()) base += "\n\n";

        if (chosenScope == "project") {
            base += "Project Context:\n" + pack.projectContext + "\n\n";
            base += "File Context:\n" + pack.fileContext + "\n\n";
            base += "Local Context:\n" + pack.localContext;
        } else if (chosenScope == "file") {
            base += "File Context:\n" + pack.fileContext + "\n\n";
            base += "Local Context:\n" + pack.localContext;
        } else {
            base += "Local Context:\n" + pack.localContext;
        }

        out.scope = chosenScope;
        out.context = trimToBudget(base, out.budgetTokens, out.truncated);
        out.tokenEstimate = estimateTokens(out.context);
        return out;
    }

    static OptimizedContext optimizeForWorker(const ContextPack& pack,
                                              const std::string& workerType,
                                              const ModelProfileRegistry& registry) {
        auto profile = registry.resolveForWorker(workerType);
        if (profile.name.empty()) {
            // Fall back to local-only conservative budget.
            profile.name = "fallback";
            profile.contextWindow = 8192;
        }
        return optimize(pack, profile);
    }

private:
    static std::string chooseScope(int contextWindow) {
        if (contextWindow >= 120000) return "project";
        if (contextWindow >= 24000) return "file";
        return "local";
    }

    static std::string trimToBudget(const std::string& text,
                                    int budgetTokens,
                                    bool& truncated) {
        int est = estimateTokens(text);
        if (est <= budgetTokens) {
            truncated = false;
            return text;
        }
        truncated = true;
        int maxChars = budgetTokens * 4;
        if (maxChars <= 0) return "";
        if ((int)text.size() <= maxChars) return text;
        std::string out = text.substr(0, (size_t)maxChars);
        out += "\n\n[truncated to fit model context window]";
        return out;
    }
};
