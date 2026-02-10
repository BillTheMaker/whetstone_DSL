#pragma once
// SyntaxHighlighterRust.h helpers for SyntaxHighlighter.

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
