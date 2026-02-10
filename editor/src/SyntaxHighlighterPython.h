#pragma once
// SyntaxHighlighterPython.h helpers for SyntaxHighlighter.

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
