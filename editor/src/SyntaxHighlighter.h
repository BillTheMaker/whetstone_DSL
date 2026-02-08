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
#include <cstring>
#include <tree_sitter/api.h>

extern "C" {
    const TSLanguage* tree_sitter_python();
    const TSLanguage* tree_sitter_cpp();
    const TSLanguage* tree_sitter_elisp();
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
};
