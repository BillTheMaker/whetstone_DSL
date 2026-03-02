#pragma once
#include "CrossLanguageBoundaryDetector.h"
#include "CrossLanguageSymbolTable.h"
#include <string>

namespace whetstone {

struct DefinitionLocation {
    bool        found             = false;
    std::string providerComponent;
    std::string providerLanguage;
    std::string callerLanguage;
    std::string scipSymbol;
};

class CrossLanguageDefinitionResolver {
public:
    explicit CrossLanguageDefinitionResolver(const CrossLanguageSymbolTable& table)
        : table_(table), detector_(table) {}

    DefinitionLocation resolve(const std::string& symbolName) const {
        auto det = detector_.detect(symbolName);
        if (!det.isBoundary) return {};
        return makeLocation(det.record, "");
    }

    DefinitionLocation resolveFromUri(const std::string& callerUri,
                                      const std::string& symbolName) const {
        auto det = detector_.detectWithUri(callerUri, symbolName);
        if (!det.isBoundary) return {};
        std::string callerLang = langFromUri(callerUri);
        return makeLocation(det.record, callerLang);
    }

private:
    const CrossLanguageSymbolTable& table_;
    CrossLanguageBoundaryDetector   detector_;

    static DefinitionLocation makeLocation(const SymbolRecord& r,
                                           const std::string& callerLang) {
        DefinitionLocation loc;
        loc.found             = true;
        loc.providerComponent = r.toComponent;
        loc.providerLanguage  = r.toLanguage;
        loc.callerLanguage    = callerLang.empty() ? r.fromLanguage : callerLang;
        loc.scipSymbol        = r.toComponent + "/" + r.name + ".";
        return loc;
    }

    static std::string langFromUri(const std::string& uri) {
        auto dot = uri.rfind('.');
        if (dot == std::string::npos) return "";
        std::string ext = uri.substr(dot);
        if (ext == ".py")  return "Python";
        if (ext == ".rs")  return "Rust";
        if (ext == ".go")  return "Go";
        if (ext == ".ts" || ext == ".js") return "TypeScript";
        if (ext == ".cpp" || ext == ".cc" || ext == ".h" || ext == ".hpp") return "C++";
        return "";
    }
};

} // namespace whetstone
