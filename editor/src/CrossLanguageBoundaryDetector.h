#pragma once
#include "CrossLanguageSymbolTable.h"
#include <string>
#include <vector>

namespace whetstone {

struct DetectionResult {
    bool         isBoundary = false;
    SymbolRecord record;
};

class CrossLanguageBoundaryDetector {
public:
    explicit CrossLanguageBoundaryDetector(const CrossLanguageSymbolTable& table)
        : table_(table) {}

    DetectionResult detect(const std::string& symbolName) const {
        static const std::vector<std::string> langs =
            {"Python","Rust","Go","TypeScript","C++"};
        for (const auto& lang : langs) {
            auto r = table_.lookup(lang, symbolName);
            if (r.found) return {true, r};
        }
        return {};
    }

    DetectionResult detectWithUri(const std::string& uri,
                                  const std::string& symbolName) const {
        std::string lang = langFromUri(uri);
        if (!lang.empty()) {
            auto r = detectInLanguage(lang, symbolName);
            if (r.isBoundary) return r;
        }
        return detect(symbolName);
    }

    DetectionResult detectInLanguage(const std::string& language,
                                     const std::string& symbolName) const {
        auto r = table_.lookup(language, symbolName);
        if (r.found) return {true, r};
        return {};
    }

private:
    const CrossLanguageSymbolTable& table_;

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
