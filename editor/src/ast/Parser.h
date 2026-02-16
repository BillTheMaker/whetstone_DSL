#pragma once
#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Import.h"
#include "Statement.h"
#include "Expression.h"
#include "Type.h"
#include "Annotation.h"
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "PreprocessorNodes.h"
#include "EnumNamespaceNodes.h"
#include <string>
#include <memory>
#include <vector>
#include <atomic>
#include <cstring>

#include <tree_sitter/api.h>

// Grammar language functions (defined in the grammar static libraries)
extern "C" {
    const TSLanguage* tree_sitter_python();
    const TSLanguage* tree_sitter_cpp();
    const TSLanguage* tree_sitter_elisp();
    const TSLanguage* tree_sitter_javascript();
    const TSLanguage* tree_sitter_typescript();
    const TSLanguage* tree_sitter_java();
    const TSLanguage* tree_sitter_rust();
    const TSLanguage* tree_sitter_go();
}

// Unique ID generator for AST nodes
class IdGenerator {
public:
    static std::string next(const std::string& prefix = "node") {
        static std::atomic<int> counter{0};
        return prefix + "_" + std::to_string(counter.fetch_add(1));
    }
};

// Diagnostic for parse errors
struct ParseDiagnostic {
    int line;
    int column;
    std::string message;
    std::string severity; // "error", "warning"
};

// Result type wrapping a module + diagnostics
struct ParseResult {
    std::unique_ptr<Module> module;
    std::vector<ParseDiagnostic> diagnostics;
    bool hasErrors() const {
        for (const auto& d : diagnostics) {
            if (d.severity == "error") return true;
        }
        return false;
    }
};

class TreeSitterParser {
private:
    // ---------------------------------------------------------------
    //  Helpers
    // ---------------------------------------------------------------
    static std::string nodeText(TSNode node, const std::string& source) {
        uint32_t start = ts_node_start_byte(node);
        uint32_t end = ts_node_end_byte(node);
        if (start >= source.size() || end > source.size()) return "";
        return source.substr(start, end - start);
    }

    static std::string nodeType(TSNode node) {
        return ts_node_type(node);
    }

    static void applySpan(ASTNode* node, TSNode tsNode) {
        if (!node || ts_node_is_null(tsNode)) return;
        TSPoint start = ts_node_start_point(tsNode);
        TSPoint end = ts_node_end_point(tsNode);
        node->setSpan((int)start.row, (int)start.column, (int)end.row, (int)end.column);
    }

    static bool isNamed(TSNode node) {
        return ts_node_is_named(node);
    }

    static TSNode childByFieldName(TSNode node, const char* field) {
        return ts_node_child_by_field_name(node, field, (uint32_t)strlen(field));
    }

    // Walk all ERROR nodes in a tree and collect diagnostics
    static void collectDiagnostics(TSNode node, const std::string& source,
                                   std::vector<ParseDiagnostic>& diags) {
        if (ts_node_is_null(node)) return;
        std::string type = nodeType(node);
        if (type == "ERROR" || ts_node_is_missing(node)) {
            TSPoint start = ts_node_start_point(node);
            ParseDiagnostic d;
            d.line = (int)start.row + 1;
            d.column = (int)start.column + 1;
            d.severity = "error";
            d.message = "Syntax error at line " + std::to_string(d.line) +
                        ", column " + std::to_string(d.column);
            diags.push_back(d);
        }
        uint32_t count = ts_node_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            collectDiagnostics(ts_node_child(node, i), source, diags);
        }
    }

    static TSNode findDescendantByType(TSNode node, const std::string& typeName) {
        if (nodeType(node) == typeName) return node;
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            TSNode found = findDescendantByType(child, typeName);
            if (!ts_node_is_null(found)) return found;
        }
        return TSNode{};
    }

    static TSNode findDescendantByField(TSNode node, const std::string& fieldName) {
        TSNode direct = childByFieldName(node, fieldName.c_str());
        if (!ts_node_is_null(direct)) return direct;
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            TSNode found = findDescendantByField(child, fieldName);
            if (!ts_node_is_null(found)) return found;
        }
        return TSNode{};
    }

    static std::string trimPointerPrefix(const std::string& text) {
        if (text.empty()) return text;
        std::string out = text;
        size_t pos = out.find('*');
        if (pos != std::string::npos) {
            out.erase(pos, 1);
        }
        return out;
    }

    // ---------------------------------------------------------------
public:
#include "ast/PythonParser.h"
#include "ast/CppParser.h"
#include "ast/ElispParser.h"
#include "ast/JavaScriptParser.h"
#include "ast/TypeScriptParser.h"
#include "ast/JavaParser.h"
#include "ast/RustParser.h"
#include "ast/GoParser.h"
private:
};

// Standalone parsers (not tree-sitter fragment includes)
#include "ast/KotlinParser.h"
#include "ast/CSharpParser.h"
