#pragma once
// Step 535: Symbol Selector API

#include <algorithm>
#include <set>
#include <string>
#include <vector>

#include "TypedTaskitemContractSchema.h"
#include "SymbolScopeExtractor.h"

struct SymbolCandidate {
    std::string name;
    std::string kind;
    std::string category;
    int scopeDepth = 0;
    bool contractAllowed = false;
};

struct SymbolSelectionResult {
    bool supported = false;
    std::vector<SymbolCandidate> candidates;
};

class SymbolSelectorAPI {
public:
    static SymbolSelectionResult select(const TaskitemContract& contract,
                                        const ScopeSnapshot& scope,
                                        const std::vector<std::string>& categories = {}) {
        SymbolSelectionResult result;
        if (!scope.supported) return result;
        result.supported = true;

        auto filters = asSet(categories);
        for (const auto& symbol : scope.candidates) {
            if (!TypedTaskitemContractSchema::symbolAllowed(contract, symbol.name)) continue;
            SymbolCandidate c;
            c.name = symbol.name;
            c.kind = symbol.kind;
            c.category = toCategory(symbol.kind);
            c.scopeDepth = symbol.scopeDepth;
            c.contractAllowed = true;
            if (!filters.empty() && filters.count(c.category) == 0) continue;
            result.candidates.push_back(c);
        }

        std::sort(result.candidates.begin(), result.candidates.end(),
                  [](const SymbolCandidate& a, const SymbolCandidate& b) {
                      if (a.scopeDepth != b.scopeDepth) return a.scopeDepth > b.scopeDepth;
                      if (a.category != b.category) return a.category < b.category;
                      return a.name < b.name;
                  });
        dedupeByName(result.candidates);
        return result;
    }

private:
    static std::string toCategory(const std::string& kind) {
        if (kind == "Function" || kind == "Method") return "function";
        if (kind == "Type" || kind == "Class" || kind == "Trait") return "type";
        if (kind == "Field") return "field";
        if (kind == "Namespace" || kind == "Import" || kind == "Module") return "module";
        if (kind == "Constant" || kind == "Enum") return "constant";
        return "symbol";
    }

    static std::set<std::string> asSet(const std::vector<std::string>& values) {
        return std::set<std::string>(values.begin(), values.end());
    }

    static void dedupeByName(std::vector<SymbolCandidate>& candidates) {
        std::set<std::string> seen;
        std::vector<SymbolCandidate> out;
        for (const auto& candidate : candidates) {
            if (seen.insert(candidate.name).second) {
                out.push_back(candidate);
            }
        }
        candidates.swap(out);
    }
};
