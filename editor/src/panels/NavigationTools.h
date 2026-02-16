#pragma once
// Step 358: Go-to-Definition + Go-to-Symbol headless navigation tools.

#include <algorithm>
#include <cctype>
#include <optional>
#include <string>
#include <utility>
#include <vector>
#include "../CompactAST.h"
#include "../Icons.h"
#include "../ast/Expression.h"
#include "../ast/Module.h"

struct SymbolLocation {
    std::string name;
    std::string type;      // function/class/variable/method/annotation/...
    std::string icon;
    std::string filePath;
    int line = -1;
    int column = -1;
    const ASTNode* node = nullptr;
};

inline std::string navLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

inline void collectSymbolsRecursive(const ASTNode* node,
                                    const std::string& filePath,
                                    std::vector<SymbolLocation>& out) {
    if (!node) return;
    const std::string ct = node->conceptType;
    const bool isSymbol =
        ct == "Function" || ct == "MethodDeclaration" || ct == "ClassDeclaration" ||
        ct == "Variable" || ct == "InterfaceDeclaration" || ct == "EnumDeclaration";
    if (isSymbol) {
        SymbolLocation s;
        s.name = getNodeName(node);
        s.type = navLower(ct);
        s.icon = WhetstoneIcons::iconForNodeType(ct);
        s.filePath = filePath;
        s.line = node->spanStartLine;
        s.column = node->spanStartCol;
        s.node = node;
        out.push_back(s);
    }
    for (auto* child : node->allChildren()) {
        collectSymbolsRecursive(child, filePath, out);
    }
}

inline std::vector<SymbolLocation> collectFileSymbols(const Module* module,
                                                      const std::string& filePath) {
    std::vector<SymbolLocation> out;
    collectSymbolsRecursive(module, filePath, out);
    std::sort(out.begin(), out.end(), [](const SymbolLocation& a, const SymbolLocation& b) {
        int la = a.line < 0 ? 1'000'000 : a.line;
        int lb = b.line < 0 ? 1'000'000 : b.line;
        if (la != lb) return la < lb;
        return a.name < b.name;
    });
    return out;
}

inline std::vector<SymbolLocation> collectProjectSymbols(
    const std::vector<std::pair<std::string, const Module*>>& modules) {
    std::vector<SymbolLocation> out;
    for (const auto& [filePath, module] : modules) {
        auto fileSymbols = collectFileSymbols(module, filePath);
        out.insert(out.end(), fileSymbols.begin(), fileSymbols.end());
    }
    std::sort(out.begin(), out.end(), [](const SymbolLocation& a, const SymbolLocation& b) {
        if (a.filePath != b.filePath) return a.filePath < b.filePath;
        int la = a.line < 0 ? 1'000'000 : a.line;
        int lb = b.line < 0 ? 1'000'000 : b.line;
        if (la != lb) return la < lb;
        return a.name < b.name;
    });
    return out;
}

inline std::vector<SymbolLocation> filterSymbols(const std::vector<SymbolLocation>& symbols,
                                                 const std::string& query) {
    if (query.empty()) return symbols;
    std::string q = navLower(query);
    std::vector<SymbolLocation> out;
    for (const auto& s : symbols) {
        std::string hay = navLower(s.name + " " + s.type + " " + s.filePath);
        if (hay.find(q) != std::string::npos) out.push_back(s);
    }
    return out;
}

inline std::optional<SymbolLocation> goToDefinition(
    const FunctionCall* call,
    const std::string& currentFile,
    const std::vector<std::pair<std::string, const Module*>>& modules) {
    if (!call || call->functionName.empty()) return std::nullopt;
    const std::string wanted = call->functionName;

    // Prefer current file definitions first.
    for (const auto& [filePath, module] : modules) {
        if (filePath != currentFile) continue;
        for (const auto& s : collectFileSymbols(module, filePath)) {
            if ((s.type == "function" || s.type == "methoddeclaration") && s.name == wanted) {
                return s;
            }
        }
    }
    // Then project-wide.
    for (const auto& [filePath, module] : modules) {
        for (const auto& s : collectFileSymbols(module, filePath)) {
            if ((s.type == "function" || s.type == "methoddeclaration") && s.name == wanted) {
                return s;
            }
        }
    }
    return std::nullopt;
}

inline std::string definitionStatusMessage(const std::optional<SymbolLocation>& loc) {
    if (loc.has_value()) return "Definition found";
    return "No definition found";
}
