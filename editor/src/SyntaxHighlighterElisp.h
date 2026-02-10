#pragma once
// SyntaxHighlighterElisp.h helpers for SyntaxHighlighter.

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
