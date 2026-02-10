#pragma once
// SyntaxHighlighterCpp.h helpers for SyntaxHighlighter.

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
