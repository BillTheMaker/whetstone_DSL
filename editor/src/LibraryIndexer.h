#pragma once
#include "DependencyParser.h"
#include "LSPClient.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include "ast/Module.h"
#include <unordered_map>
#include <unordered_set>
#include <string>

struct LibraryIndexData {
    std::unordered_map<std::string, std::vector<LSPClient::WorkspaceSymbol>> symbolsByLibrary;
    std::unordered_map<std::string, std::vector<std::string>> completionsByLibrary;
};

static inline std::string languageForDependencySource(const std::string& source) {
    std::string base = source;
    auto pos = base.find(':');
    if (pos != std::string::npos) base = base.substr(0, pos);
    if (base == "requirements.txt" || base == "setup.py" || base == "pyproject.toml") return "python";
    if (base == "package.json") return "javascript";
    if (base == "Cargo.toml") return "rust";
    if (base == "go.mod") return "go";
    if (base == "pom.xml" || base == "build.gradle") return "java";
    return "cpp";
}

static inline void clearExternalModules(Module* module) {
    if (!module) return;
    std::vector<ASTNode*> existing = module->getChildren("externalModules");
    for (auto* child : existing) {
        module->removeChild(child);
        delete child;
    }
}

static inline bool symbolMatchesLibrary(const LSPClient::WorkspaceSymbol& sym,
                                        const std::string& library) {
    if (sym.containerName == library) return true;
    std::string prefixDot = library + ".";
    std::string prefixScope = library + "::";
    return sym.name.rfind(prefixDot, 0) == 0 || sym.name.rfind(prefixScope, 0) == 0;
}

static inline void rebuildExternalModules(Module* module,
                                          const std::vector<DependencySpec>& deps,
                                          const LibraryIndexData& index) {
    if (!module) return;
    clearExternalModules(module);
    std::unordered_map<std::string, int> nameCounts;
    int signatureId = 0;

    for (const auto& dep : deps) {
        if (dep.name.empty()) continue;
        int count = nameCounts[dep.name]++;
        std::string id = "ext_" + dep.name + (count > 0 ? "_" + std::to_string(count) : "");
        auto* ext = new ExternalModule(id, dep.name,
                                       languageForDependencySource(dep.source),
                                       dep.version);

        std::unordered_set<std::string> added;
        auto it = index.symbolsByLibrary.find(dep.name);
        if (it != index.symbolsByLibrary.end()) {
            for (const auto& sym : it->second) {
                if (!symbolMatchesLibrary(sym, dep.name)) continue;
                if (!added.insert(sym.name).second) continue;
                auto* sig = new TypeSignature("sig_" + std::to_string(signatureId++), sym.name);
                ext->addChild("signatures", sig);
            }
        }
        auto it2 = index.completionsByLibrary.find(dep.name);
        if (it2 != index.completionsByLibrary.end()) {
            for (const auto& name : it2->second) {
                if (!added.insert(name).second) continue;
                auto* sig = new TypeSignature("sig_" + std::to_string(signatureId++), name);
                ext->addChild("signatures", sig);
            }
        }

        module->addChild("externalModules", ext);
    }
}
