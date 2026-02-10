#pragma once
// Step 52: Syntax highlighting via tree-sitter
//
// SyntaxHighlighter parses source text with tree-sitter and walks the CST
// to produce a list of colored spans. Each span has a byte range and a
// token category (keyword, string, comment, number, identifier, operator,
// type, punctuation, function, parameter).
//
// The highlight data is language-agnostic at the output level — consumers
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

    // --- Python --------------------------------------------------------

    static bool isPythonKeyword(const std::string& text) {
        static const char* keywords[] = {
            "def", "class", "if", "elif", "else", "for", "while", "return",
            "import", "from", "as", "try", "except", "finally", "raise",
            "with", "yield", "lambda", "pass", "break", "continue",
            "and", "or", "not", "in", "is", "del", "global", "nonlocal",
            "assert", "async", "await", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static bool isPythonBuiltin(const std::string& text) {
        static const char* builtins[] = {
            "True", "False", "None", "print", "len", "range", "int",
            "str", "float", "list", "dict", "set", "tuple", "type",
            "isinstance", "super", "self", nullptr
        };
        for (const char** b = builtins; *b; ++b) {
            if (text == *b) return true;
        }
        return false;
    }

    static void walkPython(TSNode node, const std::string& source,
                           std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        // Leaf / terminal categorization
        if (type == "comment") {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type == "string" || type == "concatenated_string") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type == "integer" || type == "float") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "identifier") {
            // Context-dependent: function name, parameter, builtin, or plain
            TSNode parent = ts_node_parent(node);
            std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);

            if (parentType == "function_definition") {
                TSNode nameField = ts_node_child_by_field_name(parent, "name", 4);
                if (!ts_node_is_null(nameField) &&
                    ts_node_start_byte(nameField) == ts_node_start_byte(node)) {
                    addSpan(spans, node, TokenCategory::Function);
                } else {
                    addSpan(spans, node, TokenCategory::Identifier);
                }
            } else if (parentType == "parameters" || parentType == "default_parameter") {
                addSpan(spans, node, TokenCategory::Parameter);
            } else if (parentType == "call") {
                TSNode funcField = ts_node_child_by_field_name(parent, "function", 8);
                if (!ts_node_is_null(funcField) &&
                    ts_node_start_byte(funcField) == ts_node_start_byte(node)) {
                    addSpan(spans, node, TokenCategory::Function);
                } else {
                    addSpan(spans, node, TokenCategory::Identifier);
                }
            } else if (parentType == "type") {
                addSpan(spans, node, TokenCategory::Type);
            } else if (isPythonBuiltin(text)) {
                addSpan(spans, node, TokenCategory::Builtin);
            } else {
                addSpan(spans, node, TokenCategory::Identifier);
            }
            descend = false;
        } else if (type == "type") {
            addSpan(spans, node, TokenCategory::Type);
            descend = false;
        } else if (!ts_node_is_named(node)) {
            // Anonymous/unnamed nodes are operators, keywords, punctuation
            if (isPythonKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text == "(" || text == ")" || text == "[" || text == "]" ||
                       text == "{" || text == "}" || text == ":" || text == "," ||
                       text == "." || text == ";") {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else if (text == "+" || text == "-" || text == "*" || text == "/" ||
                       text == "%" || text == "=" || text == "==" || text == "!=" ||
                       text == "<" || text == ">" || text == "<=" || text == ">=" ||
                       text == "+=" || text == "-=" || text == "*=" || text == "/=" ||
                       text == "**" || text == "//" || text == "->" || text == "@") {
                addSpan(spans, node, TokenCategory::Operator);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkPython(ts_node_child(node, i), source, spans);
            }
        }
    }

    // --- C++ -----------------------------------------------------------

    static bool isCppKeyword(const std::string& text) {
        static const char* keywords[] = {
            "auto", "break", "case", "catch", "class", "const", "constexpr",
            "continue", "default", "delete", "do", "else", "enum", "explicit",
            "extern", "for", "friend", "goto", "if", "inline", "mutable",
            "namespace", "new", "noexcept", "operator", "private", "protected",
            "public", "register", "return", "sizeof", "static", "static_assert",
            "static_cast", "struct", "switch", "template", "this", "throw",
            "try", "typedef", "typeid", "typename", "union", "using",
            "virtual", "volatile", "while", "override", "final",
            "co_await", "co_return", "co_yield", "concept", "requires",
            "consteval", "constinit", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static bool isCppType(const std::string& text) {
        static const char* types[] = {
            "void", "bool", "char", "int", "float", "double", "long",
            "short", "unsigned", "signed", "size_t", "nullptr_t",
            "int8_t", "int16_t", "int32_t", "int64_t",
            "uint8_t", "uint16_t", "uint32_t", "uint64_t",
            "string", "vector", "map", "set", "array", "tuple",
            "unique_ptr", "shared_ptr", "weak_ptr", "optional",
            nullptr
        };
        for (const char** t = types; *t; ++t) {
            if (text == *t) return true;
        }
        return false;
    }

    // --- JavaScript / TypeScript --------------------------------------

    static bool isJsKeyword(const std::string& text) {
        static const char* keywords[] = {
            "function", "return", "if", "else", "for", "while", "class",
            "const", "let", "var", "import", "export", "new", "try", "catch",
            "finally", "switch", "case", "break", "continue", "throw",
            "async", "await", "yield", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static void walkJavaScript(TSNode node, const std::string& source,
                               std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        if (type.find("comment") != std::string::npos) {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type == "string" || type == "string_fragment" || type == "template_string" ||
                   type == "template_substitution") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type == "number" || type == "number_literal") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "identifier" || type == "property_identifier") {
            TSNode parent = ts_node_parent(node);
            std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);
            if (parentType == "function_declaration" || parentType == "method_definition") {
                addSpan(spans, node, TokenCategory::Function);
            } else {
                addSpan(spans, node, TokenCategory::Identifier);
            }
            descend = false;
        } else if (type == "true" || type == "false" || type == "null" || type == "undefined") {
            addSpan(spans, node, TokenCategory::Builtin);
            descend = false;
        } else if (!ts_node_is_named(node)) {
            if (isJsKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text == "(" || text == ")" || text == "[" || text == "]" ||
                       text == "{" || text == "}" || text == ";" || text == "," ||
                       text == "." || text == ":" ) {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else {
                addSpan(spans, node, TokenCategory::Operator);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkJavaScript(ts_node_child(node, i), source, spans);
            }
        }
    }

    static void walkTypeScript(TSNode node, const std::string& source,
                               std::vector<HighlightSpan>& spans) {
        walkJavaScript(node, source, spans);
    }

    // --- Java -----------------------------------------------------------

    static bool isJavaKeyword(const std::string& text) {
        static const char* keywords[] = {
            "class", "interface", "enum", "public", "private", "protected",
            "static", "final", "void", "int", "float", "double", "boolean",
            "return", "if", "else", "for", "while", "switch", "case",
            "break", "continue", "new", "try", "catch", "finally", "throw",
            "extends", "implements", "import", "package", "this", "super",
            nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static void walkJava(TSNode node, const std::string& source,
                         std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        if (type.find("comment") != std::string::npos) {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type.find("string") != std::string::npos || type == "character_literal") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type.find("integer") != std::string::npos || type.find("floating") != std::string::npos ||
                   type == "decimal_integer_literal" || type == "decimal_floating_point_literal") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "identifier") {
            TSNode parent = ts_node_parent(node);
            std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);
            if (parentType == "method_declaration" || parentType == "constructor_declaration") {
                addSpan(spans, node, TokenCategory::Function);
            } else {
                addSpan(spans, node, TokenCategory::Identifier);
            }
            descend = false;
        } else if (type == "type_identifier" || type == "integral_type" || type == "floating_point_type") {
            addSpan(spans, node, TokenCategory::Type);
            descend = false;
        } else if (type == "true" || type == "false" || type == "null_literal") {
            addSpan(spans, node, TokenCategory::Builtin);
            descend = false;
        } else if (!ts_node_is_named(node)) {
            if (isJavaKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text == "(" || text == ")" || text == "[" || text == "]" ||
                       text == "{" || text == "}" || text == ";" || text == "," ||
                       text == "." || text == ":" ) {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else {
                addSpan(spans, node, TokenCategory::Operator);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkJava(ts_node_child(node, i), source, spans);
            }
        }
    }

    // --- Rust -----------------------------------------------------------

    static bool isRustKeyword(const std::string& text) {
        static const char* keywords[] = {
            "fn", "let", "mut", "pub", "impl", "trait", "struct", "enum",
            "use", "mod", "crate", "self", "super", "return", "if", "else",
            "for", "while", "loop", "match", "break", "continue", "const",
            "static", "async", "await", "move", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static void walkRust(TSNode node, const std::string& source,
                         std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        if (type.find("comment") != std::string::npos) {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type == "string_literal" || type == "char_literal" ||
                   type == "raw_string_literal") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type.find("integer") != std::string::npos || type.find("float") != std::string::npos ||
                   type == "number") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "identifier") {
            TSNode parent = ts_node_parent(node);
            std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);
            if (parentType == "function_item") {
                addSpan(spans, node, TokenCategory::Function);
            } else {
                addSpan(spans, node, TokenCategory::Identifier);
            }
            descend = false;
        } else if (type == "type_identifier" || type == "primitive_type") {
            addSpan(spans, node, TokenCategory::Type);
            descend = false;
        } else if (!ts_node_is_named(node)) {
            if (isRustKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text == "(" || text == ")" || text == "[" || text == "]" ||
                       text == "{" || text == "}" || text == ";" || text == "," ||
                       text == ":" ) {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else {
                addSpan(spans, node, TokenCategory::Operator);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkRust(ts_node_child(node, i), source, spans);
            }
        }
    }

    // --- Go -------------------------------------------------------------

    static bool isGoKeyword(const std::string& text) {
        static const char* keywords[] = {
            "func", "package", "import", "return", "if", "else", "for",
            "switch", "case", "break", "continue", "struct", "interface",
            "type", "var", "const", "go", "defer", "range", nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static void walkGo(TSNode node, const std::string& source,
                       std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        if (type.find("comment") != std::string::npos) {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type == "interpreted_string_literal" || type == "raw_string_literal" ||
                   type == "rune_literal") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type.find("int") != std::string::npos || type.find("float") != std::string::npos ||
                   type == "number") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "identifier") {
            TSNode parent = ts_node_parent(node);
            std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);
            if (parentType == "function_declaration" || parentType == "method_declaration") {
                addSpan(spans, node, TokenCategory::Function);
            } else {
                addSpan(spans, node, TokenCategory::Identifier);
            }
            descend = false;
        } else if (type == "type_identifier" || type == "primitive_type") {
            addSpan(spans, node, TokenCategory::Type);
            descend = false;
        } else if (!ts_node_is_named(node)) {
            if (isGoKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text == "(" || text == ")" || text == "[" || text == "]" ||
                       text == "{" || text == "}" || text == ";" || text == "," ||
                       text == ":" ) {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else {
                addSpan(spans, node, TokenCategory::Operator);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkGo(ts_node_child(node, i), source, spans);
            }
        }
    }

    static void walkCpp(TSNode node, const std::string& source,
                        std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        if (type == "comment") {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type == "string_literal" || type == "raw_string_literal" ||
                   type == "char_literal" || type == "string_content") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type == "number_literal") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "primitive_type" || type == "sized_type_specifier" ||
                   type == "type_identifier") {
            addSpan(spans, node, TokenCategory::Type);
            descend = false;
        } else if (type == "identifier") {
            TSNode parent = ts_node_parent(node);
            std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);

            if (parentType == "function_declarator") {
                TSNode declField = ts_node_child_by_field_name(parent, "declarator", 10);
                if (!ts_node_is_null(declField) &&
                    ts_node_start_byte(declField) == ts_node_start_byte(node)) {
                    addSpan(spans, node, TokenCategory::Function);
                } else {
                    addSpan(spans, node, TokenCategory::Identifier);
                }
            } else if (parentType == "call_expression") {
                TSNode funcField = ts_node_child_by_field_name(parent, "function", 8);
                if (!ts_node_is_null(funcField) &&
                    ts_node_start_byte(funcField) == ts_node_start_byte(node)) {
                    addSpan(spans, node, TokenCategory::Function);
                } else {
                    addSpan(spans, node, TokenCategory::Identifier);
                }
            } else if (parentType == "parameter_declaration") {
                TSNode declField = ts_node_child_by_field_name(parent, "declarator", 10);
                if (!ts_node_is_null(declField) &&
                    ts_node_start_byte(declField) == ts_node_start_byte(node)) {
                    addSpan(spans, node, TokenCategory::Parameter);
                } else if (isCppType(text)) {
                    addSpan(spans, node, TokenCategory::Type);
                } else {
                    addSpan(spans, node, TokenCategory::Identifier);
                }
            } else if (isCppType(text)) {
                addSpan(spans, node, TokenCategory::Type);
            } else {
                addSpan(spans, node, TokenCategory::Identifier);
            }
            descend = false;
        } else if (type == "true" || type == "false" || type == "nullptr") {
            addSpan(spans, node, TokenCategory::Builtin);
            descend = false;
        } else if (type == "preproc_include" || type == "preproc_def" ||
                   type == "preproc_ifdef" || type == "preproc_else" ||
                   type == "preproc_endif" || type == "preproc_call") {
            addSpan(spans, node, TokenCategory::Keyword);
            descend = false;
        } else if (type == "system_lib_string") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (!ts_node_is_named(node)) {
            if (isCppKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text == "(" || text == ")" || text == "[" || text == "]" ||
                       text == "{" || text == "}" || text == ";" || text == "," ||
                       text == "." || text == "::" || text == ":" || text == "->") {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else if (text == "+" || text == "-" || text == "*" || text == "/" ||
                       text == "%" || text == "=" || text == "==" || text == "!=" ||
                       text == "<" || text == ">" || text == "<=" || text == ">=" ||
                       text == "+=" || text == "-=" || text == "*=" || text == "/=" ||
                       text == "&&" || text == "||" || text == "!" || text == "&" ||
                       text == "|" || text == "^" || text == "~" || text == "<<" ||
                       text == ">>" || text == "++" || text == "--") {
                addSpan(spans, node, TokenCategory::Operator);
            } else if (isCppType(text)) {
                addSpan(spans, node, TokenCategory::Type);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkCpp(ts_node_child(node, i), source, spans);
            }
        }
    }

    // --- Elisp ---------------------------------------------------------

    static bool isElispKeyword(const std::string& text) {
        static const char* keywords[] = {
            "defun", "defvar", "defconst", "defmacro", "defcustom",
            "let", "let*", "if", "when", "unless", "cond", "while",
            "dolist", "dotimes", "progn", "prog1", "prog2",
            "lambda", "setq", "setf", "require", "provide",
            "interactive", "save-excursion", "save-restriction",
            "condition-case", "unwind-protect", "catch", "throw",
            nullptr
        };
        for (const char** k = keywords; *k; ++k) {
            if (text == *k) return true;
        }
        return false;
    }

    static void walkElisp(TSNode node, const std::string& source,
                          std::vector<HighlightSpan>& spans) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        std::string text = nodeText(node, source);

        bool descend = true;

        if (type == "comment") {
            addSpan(spans, node, TokenCategory::Comment);
            descend = false;
        } else if (type == "string") {
            addSpan(spans, node, TokenCategory::String);
            descend = false;
        } else if (type == "integer" || type == "float") {
            addSpan(spans, node, TokenCategory::Number);
            descend = false;
        } else if (type == "symbol") {
            if (isElispKeyword(text)) {
                addSpan(spans, node, TokenCategory::Keyword);
            } else if (text.size() > 0 && text[0] == ':') {
                // keyword symbol like :test
                addSpan(spans, node, TokenCategory::Builtin);
            } else if (text == "t" || text == "nil") {
                addSpan(spans, node, TokenCategory::Builtin);
            } else {
                TSNode parent = ts_node_parent(node);
                std::string parentType = ts_node_is_null(parent) ? "" : nodeType(parent);
                if (parentType == "function_definition" || parentType == "special_form") {
                    // Check if this is the name position (second element)
                    TSNode firstSibling = ts_node_named_child(parent, 0);
                    TSNode secondSibling = ts_node_named_child(parent, 1);
                    if (!ts_node_is_null(firstSibling) &&
                        isElispKeyword(nodeText(firstSibling, source)) &&
                        !ts_node_is_null(secondSibling) &&
                        ts_node_start_byte(secondSibling) == ts_node_start_byte(node)) {
                        addSpan(spans, node, TokenCategory::Function);
                    } else {
                        addSpan(spans, node, TokenCategory::Identifier);
                    }
                } else {
                    addSpan(spans, node, TokenCategory::Identifier);
                }
            }
            descend = false;
        } else if (!ts_node_is_named(node)) {
            if (text == "(" || text == ")" || text == "[" || text == "]") {
                addSpan(spans, node, TokenCategory::Punctuation);
            } else if (text == "'" || text == "`" || text == ",") {
                addSpan(spans, node, TokenCategory::Operator);
            }
            descend = false;
        }

        if (descend) {
            uint32_t count = ts_node_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                walkElisp(ts_node_child(node, i), source, spans);
            }
        }
    }

    // --- Org -----------------------------------------------------------

    static void walkOrgSimple(const std::string& source,
                              std::vector<HighlightSpan>& spans) {
        size_t start = 0;
        while (start < source.size()) {
            size_t end = source.find('\n', start);
            if (end == std::string::npos) end = source.size();
            std::string line = source.substr(start, end - start);
            std::string trimmed = line;
            while (!trimmed.empty() && (trimmed.back() == '\r' || trimmed.back() == '\n')) {
                trimmed.pop_back();
            }
            if (!trimmed.empty() && trimmed[0] == '*') {
                spans.push_back({(uint32_t)start, (uint32_t)end, TokenCategory::Keyword});
            } else if (trimmed.rfind("#+begin_src", 0) == 0 ||
                       trimmed.rfind("#+end_src", 0) == 0 ||
                       trimmed.rfind("#+", 0) == 0) {
                spans.push_back({(uint32_t)start, (uint32_t)end, TokenCategory::Comment});
            }
            start = end + 1;
        }
    }
};
