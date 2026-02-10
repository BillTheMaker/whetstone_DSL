#pragma once
#include "LSPClient.h"
#include "PrimitivesRegistry.h"
#include <unordered_set>
#include <algorithm>

struct CompletionBuildResult {
    std::vector<LSPClient::CompletionItem> items;
    std::unordered_set<std::string> preferredNames;
};

static inline bool matchesPrefix(const std::string& text, const std::string& prefix) {
    if (prefix.empty()) return true;
    return text.rfind(prefix, 0) == 0;
}

static inline CompletionBuildResult buildLibraryAwareCompletions(
    const std::vector<LSPClient::CompletionItem>& lspItems,
    const std::vector<PrimitiveSymbol>& primitives,
    const std::string& prefix) {
    CompletionBuildResult out;
    out.items = lspItems;
    std::unordered_set<std::string> existing;
    for (const auto& item : lspItems) {
        existing.insert(item.label);
    }

    for (const auto& prim : primitives) {
        out.preferredNames.insert(prim.name);
        if (!matchesPrefix(prim.name, prefix)) continue;
        if (existing.insert(prim.name).second) {
            LSPClient::CompletionItem ci;
            ci.label = prim.name;
            ci.insertText = prim.name;
            ci.detail = prim.source;
            out.items.push_back(std::move(ci));
        }
    }

    std::stable_sort(out.items.begin(), out.items.end(),
        [&](const LSPClient::CompletionItem& a, const LSPClient::CompletionItem& b) {
            bool aPref = out.preferredNames.count(a.label) > 0;
            bool bPref = out.preferredNames.count(b.label) > 0;
            if (aPref != bPref) return aPref > bPref;
            return a.label < b.label;
        });

    return out;
}
