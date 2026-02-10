#pragma once
#include "EmacsIntegration.h"
#include "EmacsPackageBrowser.h"
#include "LibraryIndexer.h"
#include "NotificationSystem.h"
#include "ast/Module.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct EmacsFunctionDoc {
    std::string name;
    std::string signature;
    std::string doc;
};

struct EmacsFunctionIndex {
    std::unordered_map<std::string, std::vector<EmacsFunctionDoc>> functionsByPackage;
};

static std::vector<std::string> parseLines(const std::string& text) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start < text.size()) {
        size_t end = text.find('\n', start);
        if (end == std::string::npos) end = text.size();
        std::string line = trimCopy(text.substr(start, end - start));
        if (!line.empty()) out.push_back(std::move(line));
        start = end + 1;
    }
    return out;
}

static std::vector<std::string> queryEmacsFunctions(EmacsConnection& emacs,
                                                    const std::string& prefix,
                                                    NotificationSystem& notifications) {
    std::string cmd = ElispCommandBuilder::aproposFunctions(prefix);
    std::string resp = emacs.sendCommand(cmd);
    if (resp.empty() && !emacs.getLastError().empty()) {
        notifications.notify(NotificationLevel::Error,
                             "[emacs] " + emacs.getLastError());
        return {};
    }
    return parseLines(resp);
}

static EmacsFunctionDoc queryEmacsFunctionDoc(EmacsConnection& emacs,
                                              const std::string& name,
                                              NotificationSystem& notifications) {
    EmacsFunctionDoc info;
    info.name = name;
    std::string cmd = ElispCommandBuilder::describeFunction(name);
    std::string resp = emacs.sendCommand(cmd);
    if (resp.empty() && !emacs.getLastError().empty()) {
        notifications.notify(NotificationLevel::Error,
                             "[emacs] " + emacs.getLastError());
        return info;
    }
    auto pos = resp.find('\n');
    if (pos == std::string::npos) {
        info.signature = trimCopy(resp);
    } else {
        info.signature = trimCopy(resp.substr(0, pos));
        info.doc = trimCopy(resp.substr(pos + 1));
    }
    return info;
}

static std::vector<std::string> buildPackageFunctionPrefixes(const std::string& pkgName) {
    std::vector<std::string> prefixes;
    if (pkgName.empty()) return prefixes;
    prefixes.push_back(pkgName + "-");
    prefixes.push_back(pkgName);
    auto dash = pkgName.find('-');
    if (dash != std::string::npos) {
        prefixes.push_back(pkgName.substr(0, dash) + "-");
    }
    return prefixes;
}

static void refreshEmacsFunctionIndex(EmacsFunctionIndex& index,
                                      EmacsConnection& emacs,
                                      const std::vector<EmacsPackageEntry>& packages,
                                      NotificationSystem& notifications) {
    index.functionsByPackage.clear();
    for (const auto& pkg : packages) {
        if (pkg.status != "loaded") continue;
        std::vector<std::string> functions;
        for (const auto& prefix : buildPackageFunctionPrefixes(pkg.name)) {
            functions = queryEmacsFunctions(emacs, "^" + prefix, notifications);
            if (!functions.empty()) break;
        }
        if (functions.empty()) continue;
        std::vector<EmacsFunctionDoc> docs;
        docs.reserve(functions.size());
        for (const auto& fn : functions) {
            docs.push_back(queryEmacsFunctionDoc(emacs, fn, notifications));
        }
        index.functionsByPackage[pkg.name] = std::move(docs);
    }
}

static void appendEmacsExternalModules(Module* module,
                                       const EmacsFunctionIndex& index,
                                       int& signatureId) {
    if (!module) return;
    for (const auto& kv : index.functionsByPackage) {
        const std::string& pkgName = kv.first;
        auto* ext = new ExternalModule("emacs_" + pkgName, pkgName, "elisp");
        for (const auto& fn : kv.second) {
            auto* sig = new TypeSignature("emacs_sig_" + std::to_string(signatureId++), fn.name);
            ext->addChild("signatures", sig);
        }
        module->addChild("externalModules", ext);
    }
}

static void addEmacsSymbolsToLibraryIndex(LibraryIndexData& libIndex,
                                          const EmacsFunctionIndex& index) {
    for (const auto& kv : index.functionsByPackage) {
        const std::string& pkgName = kv.first;
        std::vector<LSPClient::WorkspaceSymbol> symbols;
        symbols.reserve(kv.second.size());
        for (const auto& fn : kv.second) {
            LSPClient::WorkspaceSymbol sym;
            sym.name = fn.name;
            sym.containerName = pkgName;
            if (!fn.signature.empty()) {
                sym.detail = fn.signature;
                if (!fn.doc.empty()) sym.detail += "\n" + fn.doc;
            } else {
                sym.detail = fn.doc;
            }
            sym.kind = 12; // Function
            symbols.push_back(std::move(sym));
        }
        if (!symbols.empty()) libIndex.symbolsByLibrary[pkgName] = std::move(symbols);
    }
}
