#pragma once
// Step 526: Symbol Scope Extractor

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>

struct ScopedSymbol {
    std::string name;
    std::string kind;
    int scopeDepth = 0;
};

struct ScopeSnapshot {
    bool supported = false;
    std::vector<ScopedSymbol> candidates;
};

class SymbolScopeExtractor {
public:
    SymbolScopeExtractor() { initDefaults(); }

    void registerRule(const std::string& language,
                      const std::string& nodeKind,
                      const std::vector<std::string>& allowedKinds) {
        rules_[key(language, nodeKind)] =
            std::set<std::string>(allowedKinds.begin(), allowedKinds.end());
    }

    ScopeSnapshot buildSnapshot(const std::string& language,
                                const std::string& nodeKind,
                                const std::vector<ScopedSymbol>& visible,
                                const std::set<std::string>& forbidden = {}) const {
        auto it = rules_.find(key(language, nodeKind));
        if (it == rules_.end()) return {};

        std::map<std::string, ScopedSymbol> selectedByName;
        for (const auto& symbol : visible) {
            if (it->second.count(symbol.kind) == 0) continue;
            if (forbidden.count(symbol.name) != 0) continue;

            auto existing = selectedByName.find(symbol.name);
            if (existing == selectedByName.end() ||
                symbol.scopeDepth > existing->second.scopeDepth) {
                selectedByName[symbol.name] = symbol;
            }
        }

        ScopeSnapshot snapshot;
        snapshot.supported = true;
        for (const auto& entry : selectedByName) {
            snapshot.candidates.push_back(entry.second);
        }
        return snapshot;
    }

    std::vector<std::string> unresolvedSymbols(
        const ScopeSnapshot& snapshot,
        const std::vector<std::string>& requestedSymbols) const {
        std::set<std::string> candidateNames;
        for (const auto& symbol : snapshot.candidates) {
            candidateNames.insert(symbol.name);
        }

        std::set<std::string> unresolved;
        for (const auto& requested : requestedSymbols) {
            if (candidateNames.count(requested) == 0) {
                unresolved.insert(requested);
            }
        }
        return std::vector<std::string>(unresolved.begin(), unresolved.end());
    }

    bool canApply(const ScopeSnapshot& snapshot,
                  const std::vector<std::string>& requestedSymbols) const {
        if (!snapshot.supported) return false;
        return unresolvedSymbols(snapshot, requestedSymbols).empty();
    }

private:
    std::map<std::string, std::set<std::string>> rules_;

    static std::string key(const std::string& language,
                           const std::string& nodeKind) {
        return language + "::" + nodeKind;
    }

    void initDefaults() {
        registerRule("cpp", "Function",
                     {"Function", "Variable", "Parameter", "Type", "Namespace"});
        registerRule("cpp", "Class", {"Method", "Field", "Type", "Namespace"});
        registerRule("python", "Function", {"Function", "Variable", "Parameter", "Class"});
        registerRule("python", "Class", {"Function", "Variable", "Class"});
        registerRule("typescript", "Function",
                     {"Function", "Variable", "Parameter", "Type", "Import"});
        registerRule("typescript", "Class", {"Function", "Variable", "Type", "Import"});
        registerRule("rust", "Function", {"Function", "Variable", "Parameter", "Type", "Trait"});
        registerRule("go", "Function", {"Function", "Variable", "Parameter", "Type", "Import"});
    }
};
