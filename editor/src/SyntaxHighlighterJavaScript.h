#pragma once
// SyntaxHighlighterJavaScript.h helpers for SyntaxHighlighter.

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
