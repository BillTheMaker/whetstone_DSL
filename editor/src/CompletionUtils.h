#pragma once
#include "LSPClient.h"
#include "PrimitivesRegistry.h"
#include <unordered_set>
#include <algorithm>
#include <cctype>

enum class CompletionContext {
    General,
    CppInclude
};

struct CompletionBuildResult {
    std::vector<LSPClient::CompletionItem> items;
    std::unordered_set<std::string> preferredNames;
};

static inline bool matchesPrefix(const std::string& text, const std::string& prefix) {
    if (prefix.empty()) return true;
    return text.rfind(prefix, 0) == 0;
}

static inline std::string trimLeft(const std::string& in) {
    size_t i = 0;
    while (i < in.size() && std::isspace(static_cast<unsigned char>(in[i]))) ++i;
    return in.substr(i);
}

static inline std::string currentLinePrefix(const std::string& text, int cursor) {
    if (cursor < 0) cursor = 0;
    if (cursor > (int)text.size()) cursor = (int)text.size();
    int lineStart = cursor;
    while (lineStart > 0 && text[lineStart - 1] != '\n') --lineStart;
    return text.substr(lineStart, cursor - lineStart);
}

static inline bool isCppLikeLanguage(const std::string& language) {
    return language == "cpp" || language == "c";
}

static inline CompletionContext detectCompletionContext(const std::string& text,
                                                        int cursor,
                                                        const std::string& language) {
    if (!isCppLikeLanguage(language)) return CompletionContext::General;
    std::string line = trimLeft(currentLinePrefix(text, cursor));
    if (line.rfind("#inc", 0) == 0 || line.rfind("#include", 0) == 0) {
        return CompletionContext::CppInclude;
    }
    return CompletionContext::General;
}

static inline bool looksLikeIncludeCandidate(const LSPClient::CompletionItem& item) {
    const std::string& label = item.label;
    const std::string& filter = item.filterText.empty() ? item.label : item.filterText;
    const std::string& insert = item.insertText.empty() ? item.label : item.insertText;
    if (label.rfind("<", 0) == 0 || label.rfind("\"", 0) == 0) return true;
    if (filter.rfind("<", 0) == 0 || filter.rfind("\"", 0) == 0) return true;
    if (insert.rfind("<", 0) == 0 || insert.rfind("\"", 0) == 0) return true;
    if (label.find(".h") != std::string::npos || filter.find(".h") != std::string::npos) return true;
    if (label.find("/") != std::string::npos || filter.find("/") != std::string::npos) return true;
    if (label == "include" || filter == "include") return true;
    return false;
}

static inline std::vector<LSPClient::CompletionItem> includeSnippetCompletions() {
    std::vector<LSPClient::CompletionItem> out;
    auto add = [&](const std::string& label, const std::string& insert, const std::string& detail) {
        LSPClient::CompletionItem ci;
        ci.label = label;
        ci.insertText = insert;
        ci.filterText = label;
        ci.detail = detail;
        out.push_back(std::move(ci));
    };
    add("include", "include ", "preprocessor directive");
    add("<string>", "<string>", "C++ header");
    add("<vector>", "<vector>", "C++ header");
    add("<iostream>", "<iostream>", "C++ header");
    add("<memory>", "<memory>", "C++ header");
    return out;
}

static inline CompletionBuildResult buildLibraryAwareCompletions(
    const std::vector<LSPClient::CompletionItem>& lspItems,
    const std::vector<PrimitiveSymbol>& primitives,
    const std::string& prefix,
    CompletionContext context = CompletionContext::General) {
    CompletionBuildResult out;
    if (context == CompletionContext::CppInclude) {
        for (const auto& item : lspItems) {
            if (looksLikeIncludeCandidate(item)) out.items.push_back(item);
        }
    } else {
        out.items = lspItems;
    }

    std::unordered_set<std::string> existing;
    for (const auto& item : out.items) {
        existing.insert(item.label);
    }

    if (context == CompletionContext::CppInclude) {
        for (const auto& ci : includeSnippetCompletions()) {
            if (!matchesPrefix(ci.label, prefix)) continue;
            if (existing.insert(ci.label).second) {
                out.preferredNames.insert(ci.label);
                out.items.push_back(ci);
            }
        }
    } else {
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
