#pragma once
// Step 52: Syntax highlighting via tree-sitter
//
// SyntaxHighlighter parses source text with tree-sitter and walks the CST
// to produce a list of colored spans. Each span has a byte range and a
// token category (keyword, string, comment, number, identifier, operator,
// type, punctuation, function, parameter).
//
// The highlight data is language-agnostic at the output level â€” consumers
// map categories to colors via a theme.

#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
#include <tree_sitter/api.h>

extern "C" {
    const TSLanguage* tree_sitter_python();
    const TSLanguage* tree_sitter_cpp();
    const TSLanguage* tree_sitter_elisp();
    const TSLanguage* tree_sitter_javascript();
    const TSLanguage* tree_sitter_typescript();
    const TSLanguage* tree_sitter_java();
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_go();
#if !defined(WHETSTONE_ENABLE_ORG)
#define WHETSTONE_ENABLE_ORG 0
#endif
#if WHETSTONE_ENABLE_ORG
    const TSLanguage* tree_sitter_org();
#endif
}

enum class TokenCategory {
    Plain,
    Keyword,
    String,
    Comment,
    Number,
    Identifier,
    Operator,
    Type,
    Punctuation,
    Function,
    Parameter,
    Builtin
};

struct HighlightSpan {
    uint32_t start; // byte offset
    uint32_t end;   // byte offset (exclusive)
    TokenCategory category;
};

class SyntaxHighlighter {
public:
    static std::vector<HighlightSpan> highlight(const std::string& source,
                                                 const std::string& language) {
        std::vector<HighlightSpan> spans;
        if (source.empty()) return spans;

        TSParser* parser = ts_parser_new();
        const TSLanguage* lang = nullptr;

        if (language == "python") lang = tree_sitter_python();
        else if (language == "cpp") lang = tree_sitter_cpp();
        else if (language == "elisp") lang = tree_sitter_elisp();
        else if (language == "javascript") lang = tree_sitter_javascript();
        else if (language == "typescript") lang = tree_sitter_typescript();
        else if (language == "java") lang = tree_sitter_java();
        else if (language == "rust") lang = tree_sitter_rust();
        else if (language == "go") lang = tree_sitter_go();
#if WHETSTONE_ENABLE_ORG
        else if (language == "org") lang = tree_sitter_org();
#endif

        if (!lang) {
            ts_parser_delete(parser);
            return spans;
        }

        ts_parser_set_language(parser, lang);
        TSTree* tree = ts_parser_parse_string(parser, nullptr,
                                               source.c_str(),
                                               (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        if (language == "python") walkPython(root, source, spans);
        else if (language == "cpp") walkCpp(root, source, spans);
        else if (language == "elisp") walkElisp(root, source, spans);
        else if (language == "javascript") walkJavaScript(root, source, spans);
        else if (language == "typescript") walkTypeScript(root, source, spans);
        else if (language == "java") walkJava(root, source, spans);
        else if (language == "rust") walkRust(root, source, spans);
        else if (language == "go") walkGo(root, source, spans);
#if WHETSTONE_ENABLE_ORG
        else if (language == "org") walkOrgSimple(source, spans);
#endif

        ts_tree_delete(tree);
        ts_parser_delete(parser);

        // Sort by start position
        std::sort(spans.begin(), spans.end(),
                  [](const HighlightSpan& a, const HighlightSpan& b) {
                      return a.start < b.start;
                  });

        return spans;
    }

    static const char* categoryName(TokenCategory cat) {
        switch (cat) {
            case TokenCategory::Plain:       return "plain";
            case TokenCategory::Keyword:     return "keyword";
            case TokenCategory::String:      return "string";
            case TokenCategory::Comment:     return "comment";
            case TokenCategory::Number:      return "number";
            case TokenCategory::Identifier:  return "identifier";
            case TokenCategory::Operator:    return "operator";
            case TokenCategory::Type:        return "type";
            case TokenCategory::Punctuation: return "punctuation";
            case TokenCategory::Function:    return "function";
            case TokenCategory::Parameter:   return "parameter";
            case TokenCategory::Builtin:     return "builtin";
        }
        return "plain";
    }

private:
    static std::string nodeType(TSNode node) {
        return ts_node_type(node);
    }

    static std::string nodeText(TSNode node, const std::string& source) {
        uint32_t s = ts_node_start_byte(node);
        uint32_t e = ts_node_end_byte(node);
        if (s >= source.size() || e > source.size()) return "";
        return source.substr(s, e - s);
    }

    static void addSpan(std::vector<HighlightSpan>& spans, TSNode node,
                        TokenCategory cat) {
        uint32_t s = ts_node_start_byte(node);
        uint32_t e = ts_node_end_byte(node);
        if (s < e) {
            spans.push_back({s, e, cat});
        }
    }

#include "SyntaxLanguages.h"
#include "SyntaxHighlighterPython.h"
#include "SyntaxHighlighterCpp.h"
#include "SyntaxHighlighterJavaScript.h"
#include "SyntaxHighlighterJava.h"
#include "SyntaxHighlighterRust.h"
#include "SyntaxHighlighterGo.h"
#include "SyntaxHighlighterElisp.h"
#include "SyntaxHighlighterOrg.h"
    // --- Python --------------------------------------------------------




    // --- C++ -----------------------------------------------------------



    // --- JavaScript / TypeScript --------------------------------------




    // --- Java -----------------------------------------------------------



    // --- Rust -----------------------------------------------------------



    // --- Go -------------------------------------------------------------




    // --- Elisp ---------------------------------------------------------



    // --- Org -----------------------------------------------------------

};
