#pragma once
#include "ContextAPI.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include "ast/Module.h"
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cctype>

struct PrimitiveSymbol {
    std::string name;
    std::string kind;   // function, type, constant, variable
    std::string source; // builtin, import, user
    std::string library;
    bool vulnerable = false;
    std::vector<std::string> tags;
};

class PrimitivesRegistry {
public:
    void setRoot(Module* root) { root_ = root; }
    void setLanguage(const std::string& lang) { language_ = lang; }
    void setVulnerableLibraries(const std::unordered_set<std::string>& libs) {
        vulnerableLibraries_ = libs;
    }
    void setContextTags(const std::vector<std::string>& tags) { contextTags_ = tags; }
    void setBlockVulnerableImports(bool block) { blockVulnerableImports_ = block; }

    std::vector<PrimitiveSymbol> getAvailableFunctions(const std::string& nodeId = "") {
        return collect(nodeId, "function");
    }

    std::vector<PrimitiveSymbol> getAvailableTypes(const std::string& nodeId = "") {
        return collect(nodeId, "type");
    }

    std::vector<PrimitiveSymbol> getAvailableConstants(const std::string& nodeId = "") {
        return collect(nodeId, "constant");
    }

private:
    std::vector<PrimitiveSymbol> collect(const std::string& nodeId,
                                         const std::string& filterKind) {
        std::vector<PrimitiveSymbol> out;
        std::unordered_set<std::string> seen;

        // Builtins
        for (const auto& sym : builtinsForLanguage(language_)) {
            if (!filterKind.empty() && sym.kind != filterKind) continue;
            if (seen.insert(sym.name).second) out.push_back(sym);
        }

        // Imported libraries
        if (root_) {
            for (auto* modNode : root_->getChildren("externalModules")) {
                auto* ext = static_cast<ExternalModule*>(modNode);
                for (auto* sigNode : ext->getChildren("signatures")) {
                    auto* sig = static_cast<TypeSignature*>(sigNode);
                    PrimitiveSymbol ps;
                    ps.name = sig->name;
                    ps.source = "import";
                    ps.library = ext->name;
                    ps.vulnerable = (vulnerableLibraries_.find(ps.library) != vulnerableLibraries_.end());
                    ps.tags = sig->semanticTags;
                    ps.tags.insert(ps.tags.end(),
                                   ext->semanticTags.begin(),
                                   ext->semanticTags.end());
                    ps.kind = classifySymbol(ps.name);
                    if (!filterKind.empty() && ps.kind != filterKind) continue;
                    if (ps.vulnerable && blockVulnerableImports_) continue;
                    if (seen.insert(ps.name).second) out.push_back(ps);
                }
            }
        }

        // User-defined (ContextAPI)
        if (root_ && !nodeId.empty()) {
            ContextAPI ctx;
            ctx.setRoot(root_);
            auto symbols = ctx.getInScopeSymbols(nodeId);
            for (const auto& sym : symbols) {
                PrimitiveSymbol ps;
                ps.name = sym.name;
                ps.source = "user";
                ps.library.clear();
                ps.kind = sym.kind == "variable" ? "constant" : sym.kind;
                if (!filterKind.empty() && ps.kind != filterKind) continue;
                if (seen.insert(ps.name).second) out.push_back(ps);
            }
        }

        if (!out.empty()) {
            auto score = [this](const PrimitiveSymbol& sym) {
                int s = 0;
                if (!contextTags_.empty()) {
                    for (const auto& tag : sym.tags) {
                        if (std::find(contextTags_.begin(), contextTags_.end(), tag) != contextTags_.end()) {
                            s += 5;
                            break;
                        }
                    }
                }
                if (sym.source == "import") s += 1;
                if (sym.vulnerable) s -= 5;
                return s;
            };
            std::stable_sort(out.begin(), out.end(),
                             [&](const PrimitiveSymbol& a, const PrimitiveSymbol& b) {
                                 int sa = score(a);
                                 int sb = score(b);
                                 if (sa != sb) return sa > sb;
                                 return false;
                             });
        }
        return out;
    }

    static std::string classifySymbol(const std::string& name) {
        if (!name.empty() && std::isupper((unsigned char)name[0])) return "type";
        if (std::all_of(name.begin(), name.end(),
                        [](unsigned char c) { return std::isupper(c) || c == '_' || std::isdigit(c); })) {
            return "constant";
        }
        return "function";
    }

    static std::vector<PrimitiveSymbol> builtinsForLanguage(const std::string& lang) {
        std::vector<PrimitiveSymbol> out;
        auto add = [&](const std::string& name, const std::string& kind) {
            out.push_back({name, kind, "builtin", ""});
        };
        if (lang == "python") {
            add("print", "function");
            add("len", "function");
            add("list", "type");
            add("dict", "type");
            add("True", "constant");
            add("False", "constant");
        } else if (lang == "javascript" || lang == "typescript") {
            add("console.log", "function");
            add("Array", "type");
            add("Promise", "type");
            add("undefined", "constant");
        } else if (lang == "cpp") {
            add("std::vector", "type");
            add("std::string", "type");
            add("printf", "function");
            add("nullptr", "constant");
        } else if (lang == "rust") {
            add("println!", "function");
            add("Vec", "type");
            add("Option", "type");
        } else if (lang == "go") {
            add("fmt.Println", "function");
            add("string", "type");
            add("nil", "constant");
        } else if (lang == "elisp") {
            add("message", "function");
            add("require", "function");
            add("use-package", "function");
            add("t", "constant");
            add("nil", "constant");
        }
        return out;
    }

    Module* root_ = nullptr;
    std::string language_ = "python";
    std::unordered_set<std::string> vulnerableLibraries_;
    std::vector<std::string> contextTags_;
    bool blockVulnerableImports_ = false;
};
