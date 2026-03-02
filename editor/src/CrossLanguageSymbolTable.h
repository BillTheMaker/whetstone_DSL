#pragma once
#include "ABIBoundaryExtractor.h"
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace whetstone {

struct SymbolRecord {
    bool        found         = false;
    std::string name;
    std::string kind;
    std::string fromComponent;
    std::string toComponent;
    std::string fromLanguage;
    std::string toLanguage;
    std::string scipSymbol;   // "<toComponent>/<name>."
};

struct SymbolAppearance {
    std::string language;     // "Python" | "Rust" | etc.
    std::string role;         // "caller" | "provider"
    std::string scipSymbol;
};

class CrossLanguageSymbolTable {
public:
    void insert(const ABIBoundaryNode& node) {
        std::string scip = makeScipSymbol(node);

        SymbolRecord rec;
        rec.found         = true;
        rec.name          = node.name;
        rec.kind          = node.kind;
        rec.fromComponent = node.fromComponent;
        rec.toComponent   = node.toComponent;
        rec.fromLanguage  = node.fromLanguage;
        rec.toLanguage    = node.toLanguage;
        rec.scipSymbol    = scip;

        byLangName_[{node.toLanguage,   node.name}] = rec;
        byLangName_[{node.fromLanguage, node.name}] = rec;

        byScip_[scip].push_back({node.toLanguage,   "provider", scip});
        byScip_[scip].push_back({node.fromLanguage, "caller",   scip});
    }

    SymbolRecord lookup(const std::string& language, const std::string& name) const {
        auto it = byLangName_.find({language, name});
        if (it == byLangName_.end()) return {};
        return it->second;
    }

    bool contains(const std::string& language, const std::string& name) const {
        return byLangName_.count({language, name}) > 0;
    }

    std::vector<SymbolAppearance> lookupByScip(const std::string& scipSymbol) const {
        auto it = byScip_.find(scipSymbol);
        if (it == byScip_.end()) return {};
        return it->second;
    }

private:
    std::map<std::pair<std::string,std::string>, SymbolRecord> byLangName_;
    std::map<std::string, std::vector<SymbolAppearance>>       byScip_;

    static std::string makeScipSymbol(const ABIBoundaryNode& node) {
        return node.toComponent + "/" + node.name + ".";
    }
};

} // namespace whetstone
