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
public:
    // ---------------------------------------------------------------
    //  Python
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parsePython(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_python());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_python_module";
        module->targetLanguage = "python";
        applySpan(module.get(), root);

        convertPythonModule(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parsePythonWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_python());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_python_module";
        result.module->targetLanguage = "python";
        applySpan(result.module.get(), root);

        convertPythonModule(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
    //  C++
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseCpp(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_cpp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_cpp_module";
        module->targetLanguage = "cpp";
        applySpan(module.get(), root);

        convertCppTranslationUnit(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseCppWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_cpp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_cpp_module";
        result.module->targetLanguage = "cpp";
        applySpan(result.module.get(), root);

        convertCppTranslationUnit(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
    //  Elisp
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseElisp(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_elisp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_elisp_module";
        module->targetLanguage = "elisp";
        applySpan(module.get(), root);

        convertElispSourceFile(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseElispWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_elisp());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_elisp_module";
        result.module->targetLanguage = "elisp";
        applySpan(result.module.get(), root);

        convertElispSourceFile(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
    //  JavaScript
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseJavaScript(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_javascript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_js_module";
        module->targetLanguage = "javascript";
        applySpan(module.get(), root);

        convertJavaScriptModule(root, source, module.get(), "javascript");

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseJavaScriptWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_javascript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_js_module";
        result.module->targetLanguage = "javascript";
        applySpan(result.module.get(), root);

        convertJavaScriptModule(root, source, result.module.get(), "javascript");
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
    //  TypeScript
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseTypeScript(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_typescript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_ts_module";
        module->targetLanguage = "typescript";
        applySpan(module.get(), root);

        convertJavaScriptModule(root, source, module.get(), "typescript");

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseTypeScriptWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_typescript());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_ts_module";
        result.module->targetLanguage = "typescript";
        applySpan(result.module.get(), root);

        convertJavaScriptModule(root, source, result.module.get(), "typescript");
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
    //  Java
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseJava(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_java());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_java_module";
        module->targetLanguage = "java";
        applySpan(module.get(), root);

        convertJavaCompilationUnit(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseJavaWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_java());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_java_module";
        result.module->targetLanguage = "java";
        applySpan(result.module.get(), root);

        convertJavaCompilationUnit(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

    // ---------------------------------------------------------------
    //  Rust
    // ---------------------------------------------------------------
    static std::unique_ptr<Module> parseRust(const std::string& source) {
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_rust());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        auto module = std::make_unique<Module>();
        module->id = IdGenerator::next("mod");
        module->name = "parsed_rust_module";
        module->targetLanguage = "rust";
        applySpan(module.get(), root);

        convertRustCrate(root, source, module.get());

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return module;
    }

    static ParseResult parseRustWithDiagnostics(const std::string& source) {
        ParseResult result;
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_rust());
        TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), (uint32_t)source.size());
        TSNode root = ts_tree_root_node(tree);

        result.module = std::make_unique<Module>();
        result.module->id = IdGenerator::next("mod");
        result.module->name = "parsed_rust_module";
        result.module->targetLanguage = "rust";
        applySpan(result.module.get(), root);

        convertRustCrate(root, source, result.module.get());
        collectDiagnostics(root, source, result.diagnostics);

        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return result;
    }

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

    // ---------------------------------------------------------------
    //  Python CST → AST
    // ---------------------------------------------------------------
    static void convertPythonModule(TSNode root, const std::string& source, Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_definition") {
                auto* fn = convertPythonFunction(child, source);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertPythonFunction(TSNode node, const std::string& source) {
        // Get function name
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;

        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        applySpan(fn, node);
        fn->name = nodeText(nameNode, source);
        applySpan(fn, node);

        // Parameters
        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertPythonParameters(paramsNode, source, fn);
        }

        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertPythonBody(bodyNode, source, fn);
        }

        // Auto-annotate: Python uses tracing GC
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    static void convertPythonParameters(TSNode paramsNode, const std::string& source, Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "identifier") {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(child, source));
                applySpan(param, child);
                fn->addChild("parameters", param);
            } else if (type == "default_parameter") {
                // def f(x=10) → Parameter with defaultValue
                TSNode nameN = childByFieldName(child, "name");
                TSNode valueN = childByFieldName(child, "value");
                if (!ts_node_is_null(nameN)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(nameN, source));
                    applySpan(param, child);
                    if (!ts_node_is_null(valueN)) {
                        ASTNode* defVal = convertPythonExpression(valueN, source);
                        if (defVal) param->setChild("defaultValue", defVal);
                    }
                    fn->addChild("parameters", param);
                }
            }
        }
    }

    static void convertPythonBody(TSNode bodyNode, const std::string& source, Function* fn) {
        // bodyNode is typically a "block" node
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            ASTNode* stmt = convertPythonStatement(child, source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertPythonStatement(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            // The return value is the first named child (if any)
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                TSNode valNode = ts_node_named_child(node, 0);
                ASTNode* val = convertPythonExpression(valNode, source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertPythonExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode conseq = childByFieldName(node, "consequence");
            if (!ts_node_is_null(conseq)) {
                uint32_t cc = ts_node_named_child_count(conseq);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertPythonStatement(ts_node_named_child(conseq, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "for_statement") {
            auto* forLoop = new ForLoop();
            forLoop->id = IdGenerator::next("for");
            applySpan(forLoop, node);
            TSNode leftNode = childByFieldName(node, "left");
            if (!ts_node_is_null(leftNode)) {
                forLoop->iteratorName = nodeText(leftNode, source);
            }
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(rightNode)) {
                ASTNode* iter = convertPythonExpression(rightNode, source);
                if (iter) forLoop->setChild("iterable", iter);
            }
            TSNode body = childByFieldName(node, "body");
            if (!ts_node_is_null(body)) {
                uint32_t cc = ts_node_named_child_count(body);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertPythonStatement(ts_node_named_child(body, i), source);
                    if (s) forLoop->addChild("body", s);
                }
            }
            return forLoop;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* expr = convertPythonExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        }
        // Fallback: wrap as expression statement
        ASTNode* expr = convertPythonExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertPythonExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_operator") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) {
                binOp->op = nodeText(opNode, source);
            }
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertPythonExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertPythonExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string" || type == "concatenated_string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "comparison_operator" || type == "boolean_operator") {
            // Treat like binary op
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count >= 2) {
                ASTNode* left = convertPythonExpression(ts_node_named_child(node, 0), source);
                if (left) binOp->setChild("left", left);
                ASTNode* right = convertPythonExpression(ts_node_named_child(node, count - 1), source);
                if (right) binOp->setChild("right", right);
            }
            // Operator is a non-named child between the named ones
            uint32_t totalCount = ts_node_child_count(node);
            for (uint32_t i = 0; i < totalCount; ++i) {
                TSNode c = ts_node_child(node, i);
                if (!ts_node_is_named(c)) {
                    std::string opText = nodeText(c, source);
                    if (!opText.empty() && opText != "(" && opText != ")") {
                        binOp->op = opText;
                        break;
                    }
                }
            }
            return binOp;
        } else if (type == "call") {
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            applySpan(call, node);
            TSNode funcNode = childByFieldName(node, "function");
            if (!ts_node_is_null(funcNode)) {
                call->functionName = nodeText(funcNode, source);
            }
            TSNode argsNode = childByFieldName(node, "arguments");
            if (!ts_node_is_null(argsNode)) {
                uint32_t count = ts_node_named_child_count(argsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    ASTNode* arg = convertPythonExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "parenthesized_expression") {
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) return convertPythonExpression(ts_node_named_child(node, 0), source);
        } else if (type == "unary_operator") {
            auto* unOp = new UnaryOperation();
            unOp->id = IdGenerator::next("unop");
            applySpan(unOp, node);
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) {
                unOp->op = nodeText(opNode, source);
            }
            TSNode operandNode = childByFieldName(node, "operand");
            if (!ts_node_is_null(operandNode)) {
                ASTNode* operand = convertPythonExpression(operandNode, source);
                if (operand) unOp->setChild("operand", operand);
            }
            return unOp;
        }
        // Fallback: treat as variable reference with raw text
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    // ---------------------------------------------------------------
    //  C++ CST → AST
    // ---------------------------------------------------------------
    static void convertCppTranslationUnit(TSNode root, const std::string& source, Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_definition") {
                auto* fn = convertCppFunction(child, source);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertCppFunction(TSNode node, const std::string& source) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);

        // In C++ grammar, the structure is:
        //   function_definition: type declarator body
        // The declarator contains the function name and parameters.

        // Return type
        TSNode typeNode = childByFieldName(node, "type");
        if (!ts_node_is_null(typeNode)) {
            std::string typeText = nodeText(typeNode, source);
            auto* retType = new PrimitiveType(IdGenerator::next("type"), typeText);
            fn->setChild("returnType", retType);
        }

        // Declarator: function_declarator which has declarator (name) and parameters
        TSNode declaratorNode = childByFieldName(node, "declarator");
        if (!ts_node_is_null(declaratorNode)) {
            extractCppFunctionName(declaratorNode, source, fn);
            extractCppParameters(declaratorNode, source, fn);
        }

        // Body
        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertCppBody(bodyNode, source, fn);
        }

        // Memory pattern detection from source text
        std::string bodySource = "";
        if (!ts_node_is_null(bodyNode)) {
            bodySource = nodeText(bodyNode, source);
        }
        detectCppMemoryPatterns(bodySource, fn);

        return fn;
    }

    static void extractCppFunctionName(TSNode declNode, const std::string& source, Function* fn) {
        std::string type = nodeType(declNode);
        if (type == "function_declarator") {
            TSNode nameNode = childByFieldName(declNode, "declarator");
            if (!ts_node_is_null(nameNode)) {
                // Could be an identifier directly, or nested further
                fn->name = nodeText(nameNode, source);
            }
        } else if (type == "identifier") {
            fn->name = nodeText(declNode, source);
        } else {
            // Try to find function_declarator among children
            uint32_t count = ts_node_named_child_count(declNode);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(declNode, i);
                std::string childType = nodeType(child);
                if (childType == "function_declarator") {
                    extractCppFunctionName(child, source, fn);
                    return;
                }
            }
            // Fallback
            fn->name = nodeText(declNode, source);
        }
    }

    static void extractCppParameters(TSNode declNode, const std::string& source, Function* fn) {
        std::string type = nodeType(declNode);
        if (type == "function_declarator") {
            TSNode paramsNode = childByFieldName(declNode, "parameters");
            if (!ts_node_is_null(paramsNode)) {
                uint32_t count = ts_node_named_child_count(paramsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    TSNode paramChild = ts_node_named_child(paramsNode, i);
                    std::string paramType = nodeType(paramChild);
                    if (paramType == "parameter_declaration") {
                        convertCppParameter(paramChild, source, fn);
                    }
                }
            }
        } else {
            // Search for function_declarator child
            uint32_t count = ts_node_named_child_count(declNode);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(declNode, i);
                if (nodeType(child) == "function_declarator") {
                    extractCppParameters(child, source, fn);
                    return;
                }
            }
        }
    }

    static void convertCppParameter(TSNode paramNode, const std::string& source, Function* fn) {
        auto* param = new Parameter();
        param->id = IdGenerator::next("param");
        applySpan(param, paramNode);

        // parameter_declaration has type and declarator fields
        TSNode typeNode = childByFieldName(paramNode, "type");
        TSNode declNode = childByFieldName(paramNode, "declarator");

        if (!ts_node_is_null(typeNode)) {
            std::string typeText = nodeText(typeNode, source);
            auto* primType = new PrimitiveType(IdGenerator::next("type"), typeText);
            applySpan(primType, typeNode);
            param->setChild("type", primType);
        }

        if (!ts_node_is_null(declNode)) {
            param->name = nodeText(declNode, source);
        }

        fn->addChild("parameters", param);
    }

    static void convertCppBody(TSNode bodyNode, const std::string& source, Function* fn) {
        // bodyNode is a compound_statement { ... }
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            ASTNode* stmt = convertCppStatement(child, source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertCppStatement(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* val = convertCppExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* expr = convertCppExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "declaration") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            return exprStmt;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            return ifStmt;
        }
        return nullptr;
    }

    static ASTNode* convertCppExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) {
                binOp->op = nodeText(opNode, source);
            }
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertCppExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertCppExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "number_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string_literal" || type == "raw_string_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) return convertCppExpression(ts_node_named_child(node, 0), source);
        }
        // Fallback
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static void detectCppMemoryPatterns(const std::string& bodySource, Function* fn) {
        bool hasUnique = bodySource.find("unique_ptr") != std::string::npos ||
                         bodySource.find("make_unique") != std::string::npos;
        bool hasShared = bodySource.find("shared_ptr") != std::string::npos ||
                         bodySource.find("make_shared") != std::string::npos;
        bool hasNew = bodySource.find("new ") != std::string::npos;
        bool hasDelete = bodySource.find("delete ") != std::string::npos ||
                         bodySource.find("delete;") != std::string::npos;

        if (hasUnique) {
            auto* anno = new LifetimeAnnotation(IdGenerator::next("anno"), "RAII");
            fn->addChild("annotations", anno);
        }
        if (hasShared) {
            auto* anno = new OwnerAnnotation(IdGenerator::next("anno"), "Shared_ARC");
            fn->addChild("annotations", anno);
        }
        if (hasNew && hasDelete) {
            auto* anno = new DeallocateAnnotation(IdGenerator::next("anno"), "Explicit");
            fn->addChild("annotations", anno);
        }
    }

    // ---------------------------------------------------------------
    //  Elisp CST → AST
    // ---------------------------------------------------------------
    static void convertElispSourceFile(TSNode root, const std::string& source, Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_definition" || type == "defun") {
                auto* fn = convertElispDefun(child, source);
                if (fn) module->addChild("functions", fn);
            } else if (type == "list") {
                auto* fn = tryConvertElispDefunFromList(child, source);
                if (fn) module->addChild("functions", fn);
            } else if (type == "special_form") {
                auto* fn = tryConvertElispDefunFromSpecialForm(child, source);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertElispDefun(TSNode node, const std::string& source) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);

        // tree-sitter-elisp function_definition has:
        //   field "name" → symbol (function name)
        //   field "parameters" → list (arglist)
        //   remaining named children → body forms (no field name)

        TSNode nameNode = childByFieldName(node, "name");
        if (!ts_node_is_null(nameNode)) {
            fn->name = nodeText(nameNode, source);
        }

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertElispArglist(paramsNode, source, fn);
        }

        // Body: iterate all named children, skip name and parameters
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            // Skip the name symbol and parameters list
            if (!ts_node_is_null(nameNode) &&
                ts_node_start_byte(child) == ts_node_start_byte(nameNode) &&
                ts_node_end_byte(child) == ts_node_end_byte(nameNode))
                continue;
            if (!ts_node_is_null(paramsNode) &&
                ts_node_start_byte(child) == ts_node_start_byte(paramsNode) &&
                ts_node_end_byte(child) == ts_node_end_byte(paramsNode))
                continue;

            // This is a body form
            ASTNode* expr = convertElispExpression(child, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, child);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }

        // Fallback: if no name/params fields, try positional approach
        if (fn->name.empty()) {
            uint32_t nc = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < nc; ++i) {
                TSNode child = ts_node_named_child(node, i);
                std::string childType = nodeType(child);
                if (childType == "symbol" && fn->name.empty()) {
                    fn->name = nodeText(child, source);
                } else if (childType == "list" && fn->getChildren("parameters").empty()) {
                    convertElispArglist(child, source, fn);
                }
            }
        }

        // Auto-annotate: Elisp uses tracing GC
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    // Handle (defun ...) when parsed as a generic list node
    static Function* tryConvertElispDefunFromList(TSNode node, const std::string& source) {
        // Check if first child is "defun" symbol
        uint32_t count = ts_node_named_child_count(node);
        if (count < 3) return nullptr;

        TSNode firstChild = ts_node_named_child(node, 0);
        std::string firstText = nodeText(firstChild, source);
        if (firstText != "defun") return nullptr;

        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");

        // Second child is the name
        TSNode nameChild = ts_node_named_child(node, 1);
        fn->name = nodeText(nameChild, source);

        // Third child is the arglist (a list)
        TSNode arglistChild = ts_node_named_child(node, 2);
        if (nodeType(arglistChild) == "list") {
            convertElispArglist(arglistChild, source, fn);
        }

        // Remaining children are body forms
        for (uint32_t i = 3; i < count; ++i) {
            TSNode bodyChild = ts_node_named_child(node, i);
            ASTNode* expr = convertElispExpression(bodyChild, source);
            if (expr) {
                if (i == count - 1) {
                    // Last form — wrap in ExpressionStatement (implicit return)
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, bodyChild);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                } else {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, bodyChild);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        }

        // Auto-annotate
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    static Function* tryConvertElispDefunFromSpecialForm(TSNode node, const std::string& source) {
        // special_form might contain defun
        uint32_t count = ts_node_named_child_count(node);
        if (count < 3) return nullptr;

        TSNode firstChild = ts_node_named_child(node, 0);
        std::string firstText = nodeText(firstChild, source);
        if (firstText != "defun") return nullptr;

        // Same logic as tryConvertElispDefunFromList
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");

        TSNode nameChild = ts_node_named_child(node, 1);
        fn->name = nodeText(nameChild, source);

        TSNode arglistChild = ts_node_named_child(node, 2);
        std::string argType = nodeType(arglistChild);
        if (argType == "list") {
            convertElispArglist(arglistChild, source, fn);
        }

        for (uint32_t i = 3; i < count; ++i) {
            TSNode bodyChild = ts_node_named_child(node, i);
            ASTNode* expr = convertElispExpression(bodyChild, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, bodyChild);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);

        return fn;
    }

    static void convertElispArglist(TSNode node, const std::string& source, Function* fn) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            std::string type = nodeType(child);
            if (type == "symbol" || type == "identifier") {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(child, source));
                applySpan(param, child);
                fn->addChild("parameters", param);
            }
        }
    }

    static void convertElispBodyField(TSNode bodyNode, const std::string& source, Function* fn) {
        // body might be a single node or we need to iterate children
        uint32_t count = ts_node_named_child_count(bodyNode);
        if (count > 0) {
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(bodyNode, i);
                ASTNode* expr = convertElispExpression(child, source);
                if (expr) {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, child);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        } else {
            ASTNode* expr = convertElispExpression(bodyNode, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, bodyNode);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }
    }

    static void convertElispBodyFromChildren(TSNode node, const std::string& source, Function* fn) {
        // Skip: first named child should be name, second should be arglist, rest is body
        uint32_t count = ts_node_named_child_count(node);
        int bodyStart = -1;
        int arglistSeen = 0;
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            std::string type = nodeType(child);
            if (type == "symbol" && fn->name.empty()) {
                fn->name = nodeText(child, source);
                continue;
            }
            if (type == "list" && fn->getChildren("parameters").empty()) {
                convertElispArglist(child, source, fn);
                arglistSeen = 1;
                continue;
            }
            if (arglistSeen || (int)i >= 2) {
                // Body form
                ASTNode* expr = convertElispExpression(child, source);
                if (expr) {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
                    applySpan(exprStmt, child);
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        }
    }

    static ASTNode* convertElispExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);

        if (type == "list") {
            // Check if operator-form: (+ x 1), (- x 1), (* x y), (/ x y)
            uint32_t count = ts_node_named_child_count(node);
            if (count >= 3) {
                TSNode firstChild = ts_node_named_child(node, 0);
                std::string firstText = nodeText(firstChild, source);
                if (firstText == "+" || firstText == "-" || firstText == "*" || firstText == "/") {
                    auto* binOp = new BinaryOperation();
                    binOp->id = IdGenerator::next("binop");
                    binOp->op = firstText;
                    applySpan(binOp, node);
                    ASTNode* left = convertElispExpression(ts_node_named_child(node, 1), source);
                    ASTNode* right = convertElispExpression(ts_node_named_child(node, 2), source);
                    if (left) binOp->setChild("left", left);
                    if (right) binOp->setChild("right", right);
                    return binOp;
                }
            }
            // Generic list — could be a function call
            if (count >= 1) {
                auto* call = new FunctionCall();
                call->id = IdGenerator::next("call");
                applySpan(call, node);
                TSNode funcName = ts_node_named_child(node, 0);
                call->functionName = nodeText(funcName, source);
                for (uint32_t i = 1; i < count; ++i) {
                    ASTNode* arg = convertElispExpression(ts_node_named_child(node, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
                return call;
            }
        } else if (type == "symbol" || type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer" || type == "number") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "special_form") {
            // Could be (if ...), (let ...), etc.
            return convertElispSpecialForm(node, source);
        }

        // Fallback
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static ASTNode* convertElispSpecialForm(TSNode node, const std::string& source) {
        uint32_t count = ts_node_named_child_count(node);
        if (count < 1) return nullptr;

        TSNode firstChild = ts_node_named_child(node, 0);
        std::string formName = nodeText(firstChild, source);

        if (formName == "if" && count >= 3) {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            ASTNode* cond = convertElispExpression(ts_node_named_child(node, 1), source);
            if (cond) ifStmt->setChild("condition", cond);
            return ifStmt;
        }

        // Generic: treat as function call
        auto* call = new FunctionCall();
        call->id = IdGenerator::next("call");
        applySpan(call, node);
        call->functionName = formName;
        for (uint32_t i = 1; i < count; ++i) {
            ASTNode* arg = convertElispExpression(ts_node_named_child(node, i), source);
            if (arg) call->addChild("arguments", arg);
        }
        return call;
    }

    // ---------------------------------------------------------------
    //  JavaScript / TypeScript CST -> AST
    // ---------------------------------------------------------------
    static void convertJavaScriptModule(TSNode root,
                                        const std::string& source,
                                        Module* module,
                                        const std::string& language) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "function_declaration") {
                auto* fn = convertJavaScriptFunction(child, source, language);
                if (fn) module->addChild("functions", fn);
            } else if (type == "class_declaration") {
                convertJavaScriptClass(child, source, module, language);
            } else if (type == "lexical_declaration" || type == "variable_declaration") {
                convertJavaScriptVariableFunctions(child, source, module, language);
            } else if (type == "export_statement") {
                TSNode decl = ts_node_named_child(child, 0);
                std::string declType = nodeType(decl);
                if (declType == "function_declaration") {
                    auto* fn = convertJavaScriptFunction(decl, source, language);
                    if (fn) module->addChild("functions", fn);
                } else if (declType == "class_declaration") {
                    convertJavaScriptClass(decl, source, module, language);
                } else if (declType == "lexical_declaration" || declType == "variable_declaration") {
                    convertJavaScriptVariableFunctions(decl, source, module, language);
                }
            }
        }
    }

    static void convertJavaScriptClass(TSNode node,
                                       const std::string& source,
                                       Module* module,
                                       const std::string& language) {
        TSNode nameNode = childByFieldName(node, "name");
        std::string className = nodeText(nameNode, source);
        TSNode bodyNode = childByFieldName(node, "body");
        if (ts_node_is_null(bodyNode)) return;
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            std::string type = nodeType(child);
            if (type == "method_definition") {
                auto* fn = convertJavaScriptMethod(child, source, language, className);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static void convertJavaScriptVariableFunctions(TSNode node,
                                                   const std::string& source,
                                                   Module* module,
                                                   const std::string& language) {
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (nodeType(child) != "variable_declarator") continue;
            TSNode nameNode = childByFieldName(child, "name");
            TSNode valueNode = childByFieldName(child, "value");
            if (ts_node_is_null(nameNode) || ts_node_is_null(valueNode)) continue;
            std::string valueType = nodeType(valueNode);
            if (valueType == "arrow_function" || valueType == "function" ||
                valueType == "function_expression") {
                auto* fn = convertJavaScriptFunctionExpression(valueNode, source,
                                                               language, nodeText(nameNode, source));
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertJavaScriptFunction(TSNode node,
                                               const std::string& source,
                                               const std::string& language) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        fn->name = nodeText(nameNode, source);

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaScriptParameters(paramsNode, source, fn, language);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaScriptBody(bodyNode, source, fn, language);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertJavaScriptMethod(TSNode node,
                                             const std::string& source,
                                             const std::string& language,
                                             const std::string& className) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        std::string name = nodeText(nameNode, source);
        fn->name = className.empty() ? name : (className + "." + name);

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaScriptParameters(paramsNode, source, fn, language);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaScriptBody(bodyNode, source, fn, language);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertJavaScriptFunctionExpression(TSNode node,
                                                         const std::string& source,
                                                         const std::string& language,
                                                         const std::string& nameOverride) {
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        fn->name = nameOverride;

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaScriptParameters(paramsNode, source, fn, language);
        } else if (nodeType(node) == "arrow_function") {
            TSNode paramNode = childByFieldName(node, "parameter");
            if (!ts_node_is_null(paramNode)) {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(paramNode, source));
                applySpan(param, paramNode);
                fn->addChild("parameters", param);
            }
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaScriptBody(bodyNode, source, fn, language);
        }

        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static void convertJavaScriptParameters(TSNode paramsNode,
                                            const std::string& source,
                                            Function* fn,
                                            const std::string& language) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "identifier" || type == "pattern" || type == "rest_pattern") {
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(child, source));
                applySpan(param, child);
                fn->addChild("parameters", param);
            } else if (type == "required_parameter" || type == "optional_parameter" ||
                       type == "formal_parameter") {
                TSNode nameNode = childByFieldName(child, "pattern");
                if (ts_node_is_null(nameNode)) nameNode = childByFieldName(child, "name");
                if (!ts_node_is_null(nameNode)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(nameNode, source));
                    applySpan(param, child);
                    TSNode typeNode = childByFieldName(child, "type");
                    if (ts_node_is_null(typeNode)) typeNode = childByFieldName(child, "type_annotation");
                    if (!ts_node_is_null(typeNode) && language == "typescript") {
                        auto* typeAnno = new CustomType();
                        typeAnno->id = IdGenerator::next("type");
                        typeAnno->typeName = nodeText(typeNode, source);
                        param->setChild("type", typeAnno);
                    }
                    fn->addChild("parameters", param);
                }
            } else if (type == "assignment_pattern") {
                TSNode left = childByFieldName(child, "left");
                TSNode right = childByFieldName(child, "right");
                if (!ts_node_is_null(left)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(left, source));
                    applySpan(param, child);
                    if (!ts_node_is_null(right)) {
                        ASTNode* defVal = convertJavaScriptExpression(right, source, language);
                        if (defVal) param->setChild("defaultValue", defVal);
                    }
                    fn->addChild("parameters", param);
                }
            }
        }
    }

    static void convertJavaScriptBody(TSNode bodyNode,
                                      const std::string& source,
                                      Function* fn,
                                      const std::string& language) {
        std::string type = nodeType(bodyNode);
        if (type == "statement_block" || type == "statement_block") {
            uint32_t count = ts_node_named_child_count(bodyNode);
            for (uint32_t i = 0; i < count; ++i) {
                ASTNode* stmt = convertJavaScriptStatement(ts_node_named_child(bodyNode, i), source, language);
                if (stmt) fn->addChild("body", stmt);
            }
        } else {
            ASTNode* expr = convertJavaScriptExpression(bodyNode, source, language);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
                applySpan(exprStmt, bodyNode);
                exprStmt->setChild("expression", expr);
                fn->addChild("body", exprStmt);
            }
        }
    }

    static ASTNode* convertJavaScriptStatement(TSNode node,
                                               const std::string& source,
                                               const std::string& language) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* val = convertJavaScriptExpression(ts_node_named_child(node, 0), source, language);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertJavaScriptExpression(ts_node_named_child(node, 0), source, language);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "lexical_declaration" || type == "variable_declaration") {
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                if (nodeType(child) != "variable_declarator") continue;
                TSNode nameNode = childByFieldName(child, "name");
                TSNode valueNode = childByFieldName(child, "value");
                if (ts_node_is_null(nameNode)) continue;
                auto* assign = new Assignment();
                assign->id = IdGenerator::next("assign");
                applySpan(assign, child);
                auto* target = new VariableReference(IdGenerator::next("var"), nodeText(nameNode, source));
                applySpan(target, nameNode);
                assign->setChild("target", target);
                if (!ts_node_is_null(valueNode)) {
                    ASTNode* val = convertJavaScriptExpression(valueNode, source, language);
                    if (val) assign->setChild("value", val);
                }
                return assign;
            }
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaScriptExpression(condNode, source, language);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertJavaScriptStatement(ts_node_named_child(consNode, i), source, language);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertJavaScriptStatement(ts_node_named_child(altNode, i), source, language);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        }
        ASTNode* expr = convertJavaScriptExpression(node, source, language);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertJavaScriptExpression(TSNode node,
                                                const std::string& source,
                                                const std::string& language) {
        std::string type = nodeType(node);
        if (type == "binary_expression" || type == "logical_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) binOp->op = nodeText(opNode, source);
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertJavaScriptExpression(leftNode, source, language);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertJavaScriptExpression(rightNode, source, language);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "assignment_expression") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertJavaScriptExpression(leftNode, source, language);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertJavaScriptExpression(rightNode, source, language);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "call_expression") {
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            applySpan(call, node);
            TSNode funcNode = childByFieldName(node, "function");
            if (!ts_node_is_null(funcNode)) {
                call->functionName = nodeText(funcNode, source);
            }
            TSNode argsNode = childByFieldName(node, "arguments");
            if (!ts_node_is_null(argsNode)) {
                uint32_t count = ts_node_named_child_count(argsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    ASTNode* arg = convertJavaScriptExpression(ts_node_named_child(argsNode, i), source, language);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "member_expression") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode objNode = childByFieldName(node, "object");
            TSNode propNode = childByFieldName(node, "property");
            if (!ts_node_is_null(propNode)) {
                mem->memberName = nodeText(propNode, source);
            }
            if (!ts_node_is_null(objNode)) {
                ASTNode* target = convertJavaScriptExpression(objNode, source, language);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "subscript_expression") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            TSNode objNode = childByFieldName(node, "object");
            TSNode idxNode = childByFieldName(node, "index");
            if (!ts_node_is_null(objNode)) {
                ASTNode* target = convertJavaScriptExpression(objNode, source, language);
                if (target) access->setChild("target", target);
            }
            if (!ts_node_is_null(idxNode)) {
                ASTNode* idx = convertJavaScriptExpression(idxNode, source, language);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "number") {
            std::string text = nodeText(node, source);
            if (text.find('.') != std::string::npos) {
                auto* lit = new FloatLiteral(IdGenerator::next("float"), text);
                applySpan(lit, node);
                return lit;
            }
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "string" || type == "string_fragment" || type == "template_string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "null") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertJavaScriptExpression(ts_node_named_child(node, 0), source, language);
            }
        }
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    // ---------------------------------------------------------------
    //  Java CST -> AST
    // ---------------------------------------------------------------
    static void convertJavaCompilationUnit(TSNode root,
                                           const std::string& source,
                                           Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "import_declaration") {
                std::string importName;
                uint32_t ic = ts_node_named_child_count(child);
                for (uint32_t j = 0; j < ic; ++j) {
                    TSNode part = ts_node_named_child(child, j);
                    std::string pType = nodeType(part);
                    if (pType == "scoped_identifier" || pType == "identifier") {
                        importName = nodeText(part, source);
                        break;
                    }
                }
                if (importName.empty()) {
                    importName = nodeText(child, source);
                }
                if (!importName.empty()) {
                    auto* imp = new Import(IdGenerator::next("imp"), importName, "module");
                    module->addChild("imports", imp);
                }
            } else if (type == "class_declaration" || type == "interface_declaration" ||
                       type == "enum_declaration" || type == "record_declaration" ||
                       type == "annotation_type_declaration") {
                convertJavaTypeDeclaration(child, source, module);
            }
        }
    }

    static void convertJavaTypeDeclaration(TSNode node,
                                           const std::string& source,
                                           Module* module) {
        TSNode nameNode = childByFieldName(node, "name");
        std::string className = nodeText(nameNode, source);
        TSNode bodyNode = childByFieldName(node, "body");
        if (ts_node_is_null(bodyNode)) return;

        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            std::string type = nodeType(child);
            if (type == "method_declaration") {
                auto* fn = convertJavaMethod(child, source, className);
                if (fn) module->addChild("functions", fn);
            } else if (type == "constructor_declaration" || type == "compact_constructor_declaration") {
                auto* fn = convertJavaConstructor(child, source, className);
                if (fn) module->addChild("functions", fn);
            } else if (type == "field_declaration") {
                convertJavaFieldDeclaration(child, source, module, className);
            } else if (type == "class_declaration" || type == "interface_declaration" ||
                       type == "enum_declaration" || type == "record_declaration" ||
                       type == "annotation_type_declaration") {
                convertJavaTypeDeclaration(child, source, module);
            }
        }
    }

    static void convertJavaFieldDeclaration(TSNode node,
                                            const std::string& source,
                                            Module* module,
                                            const std::string& className) {
        TSNode typeNode = childByFieldName(node, "type");
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (nodeType(child) != "variable_declarator") continue;
            TSNode nameNode = childByFieldName(child, "name");
            if (ts_node_is_null(nameNode)) continue;
            std::string varName = nodeText(nameNode, source);
            if (!className.empty()) varName = className + "." + varName;
            auto* var = new Variable(IdGenerator::next("var"), varName);
            applySpan(var, child);
            if (!ts_node_is_null(typeNode)) {
                if (auto* t = convertJavaType(typeNode, source)) var->setChild("type", t);
            }
            TSNode valueNode = childByFieldName(child, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* init = convertJavaExpression(valueNode, source);
                if (init) var->setChild("initializer", init);
            }
            module->addChild("variables", var);
        }
    }

    static Function* convertJavaMethod(TSNode node,
                                       const std::string& source,
                                       const std::string& className) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        std::string methodName = nodeText(nameNode, source);
        fn->name = className.empty() ? methodName : (className + "." + methodName);

        TSNode typeNode = childByFieldName(node, "type");
        if (!ts_node_is_null(typeNode)) {
            if (auto* t = convertJavaType(typeNode, source)) fn->setChild("returnType", t);
        }

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaParameters(paramsNode, source, fn);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaBlockStatements(bodyNode, source, fn);
        }

        attachJavaAnnotations(node, source, fn);
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static Function* convertJavaConstructor(TSNode node,
                                            const std::string& source,
                                            const std::string& className) {
        if (className.empty()) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        fn->name = className + ".constructor";

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertJavaParameters(paramsNode, source, fn);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertJavaBlockStatements(bodyNode, source, fn);
        }

        attachJavaAnnotations(node, source, fn);
        auto* reclaim = new ReclaimAnnotation(IdGenerator::next("anno"), "Tracing");
        fn->addChild("annotations", reclaim);
        return fn;
    }

    static void convertJavaParameters(TSNode paramsNode,
                                      const std::string& source,
                                      Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "formal_parameter" || type == "spread_parameter") {
                TSNode nameNode = childByFieldName(child, "name");
                if (ts_node_is_null(nameNode)) {
                    nameNode = findDescendantByType(child, "_variable_declarator_id");
                    if (!ts_node_is_null(nameNode)) {
                        TSNode innerName = childByFieldName(nameNode, "name");
                        if (!ts_node_is_null(innerName)) nameNode = innerName;
                    }
                }
                if (ts_node_is_null(nameNode)) continue;
                std::string name = nodeText(nameNode, source);
                if (type == "spread_parameter") name = "..." + name;
                auto* param = new Parameter(IdGenerator::next("param"), name);
                applySpan(param, child);
                TSNode typeNode = childByFieldName(child, "type");
                if (!ts_node_is_null(typeNode)) {
                    if (auto* t = convertJavaType(typeNode, source)) param->setChild("type", t);
                }
                fn->addChild("parameters", param);
            }
        }
    }

    static void convertJavaBlockStatements(TSNode bodyNode,
                                           const std::string& source,
                                           Function* fn) {
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            ASTNode* stmt = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertJavaStatement(TSNode node,
                                         const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_statement") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* val = convertJavaExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertJavaExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "local_variable_declaration") {
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_named_child(node, i);
                if (nodeType(child) != "variable_declarator") continue;
                TSNode nameNode = childByFieldName(child, "name");
                if (ts_node_is_null(nameNode)) continue;
                auto* assign = new Assignment();
                assign->id = IdGenerator::next("assign");
                applySpan(assign, child);
                auto* target = new VariableReference(IdGenerator::next("var"), nodeText(nameNode, source));
                applySpan(target, nameNode);
                assign->setChild("target", target);
                TSNode valueNode = childByFieldName(child, "value");
                if (!ts_node_is_null(valueNode)) {
                    ASTNode* val = convertJavaExpression(valueNode, source);
                    if (val) assign->setChild("value", val);
                }
                return assign;
            }
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(consNode, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(altNode, i), source);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "while_statement") {
            auto* loop = new WhileLoop();
            loop->id = IdGenerator::next("while");
            applySpan(loop, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaExpression(condNode, source);
                if (cond) loop->setChild("condition", cond);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "for_statement") {
            auto* loop = new WhileLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertJavaExpression(condNode, source);
                if (cond) loop->setChild("condition", cond);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "enhanced_for_statement") {
            auto* loop = new ForLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode nameNode = childByFieldName(node, "name");
            if (!ts_node_is_null(nameNode)) {
                loop->iteratorName = nodeText(nameNode, source);
            }
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* iter = convertJavaExpression(valueNode, source);
                if (iter) loop->setChild("iterable", iter);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "block") {
            auto* block = new Block();
            block->id = IdGenerator::next("block");
            applySpan(block, node);
            uint32_t bc = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < bc; ++i) {
                ASTNode* s = convertJavaStatement(ts_node_named_child(node, i), source);
                if (s) block->addChild("statements", s);
            }
            return block;
        } else if (type == "try_statement") {
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                auto* block = new Block();
                block->id = IdGenerator::next("block");
                applySpan(block, bodyNode);
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertJavaStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) block->addChild("statements", s);
                }
                return block;
            }
        }

        ASTNode* expr = convertJavaExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertJavaExpression(TSNode node,
                                          const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) binOp->op = nodeText(opNode, source);
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertJavaExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertJavaExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "assignment_expression") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertJavaExpression(leftNode, source);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertJavaExpression(rightNode, source);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "method_invocation") {
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            applySpan(call, node);
            TSNode nameNode = childByFieldName(node, "name");
            TSNode objectNode = childByFieldName(node, "object");
            if (!ts_node_is_null(objectNode) && !ts_node_is_null(nameNode)) {
                call->functionName = nodeText(objectNode, source) + "." + nodeText(nameNode, source);
            } else if (!ts_node_is_null(nameNode)) {
                call->functionName = nodeText(nameNode, source);
            } else {
                call->functionName = nodeText(node, source);
            }
            TSNode argsNode = childByFieldName(node, "arguments");
            if (!ts_node_is_null(argsNode)) {
                uint32_t count = ts_node_named_child_count(argsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    ASTNode* arg = convertJavaExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "field_access") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode objNode = childByFieldName(node, "object");
            TSNode fieldNode = childByFieldName(node, "field");
            if (!ts_node_is_null(fieldNode)) {
                mem->memberName = nodeText(fieldNode, source);
            }
            if (!ts_node_is_null(objNode)) {
                ASTNode* target = convertJavaExpression(objNode, source);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "array_access") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            TSNode arrayNode = childByFieldName(node, "array");
            TSNode indexNode = childByFieldName(node, "index");
            if (!ts_node_is_null(arrayNode)) {
                ASTNode* target = convertJavaExpression(arrayNode, source);
                if (target) access->setChild("target", target);
            }
            if (!ts_node_is_null(indexNode)) {
                ASTNode* idx = convertJavaExpression(indexNode, source);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "floating_point_literal") {
            auto* lit = new FloatLiteral(IdGenerator::next("float"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "string_literal" || type == "character_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "null_literal") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertJavaExpression(ts_node_named_child(node, 0), source);
            }
        } else if (type == "lambda_expression") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        }

        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static Type* convertJavaType(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "integral_type" || type == "floating_point_type" ||
            type == "boolean_type" || type == "void_type") {
            auto* prim = new PrimitiveType();
            prim->id = IdGenerator::next("type");
            prim->kind = nodeText(node, source);
            return prim;
        } else if (type == "array_type") {
            auto* arr = new ArrayType();
            arr->id = IdGenerator::next("type");
            TSNode element = childByFieldName(node, "element");
            if (!ts_node_is_null(element)) {
                if (auto* et = convertJavaType(element, source)) arr->setChild("elementType", et);
            }
            return arr;
        }
        auto* custom = new CustomType();
        custom->id = IdGenerator::next("type");
        custom->typeName = nodeText(node, source);
        return custom;
    }

    static void attachJavaAnnotations(TSNode node,
                                      const std::string& source,
                                      ASTNode* target) {
        if (!target) return;
        uint32_t count = ts_node_named_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(node, i);
            if (nodeType(child) != "modifiers") continue;
            uint32_t mc = ts_node_named_child_count(child);
            for (uint32_t j = 0; j < mc; ++j) {
                TSNode modChild = ts_node_named_child(child, j);
                std::string mType = nodeType(modChild);
                if (mType == "marker_annotation" || mType == "annotation") {
                    auto* anno = new LangSpecific();
                    anno->id = IdGenerator::next("anno");
                    anno->language = "java";
                    TSNode nameNode = childByFieldName(modChild, "name");
                    anno->idiomType = ts_node_is_null(nameNode) ? "" : nodeText(nameNode, source);
                    anno->rawSyntax = nodeText(modChild, source);
                    target->addChild("annotations", anno);
                }
            }
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

    // ---------------------------------------------------------------
    //  Rust CST -> AST
    // ---------------------------------------------------------------
    static void convertRustCrate(TSNode root,
                                 const std::string& source,
                                 Module* module) {
        uint32_t count = ts_node_named_child_count(root);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(root, i);
            std::string type = nodeType(child);
            if (type == "use_declaration") {
                TSNode arg = childByFieldName(child, "argument");
                std::string importName = nodeText(arg, source);
                if (importName.empty()) importName = nodeText(child, source);
                if (!importName.empty()) {
                    auto* imp = new Import(IdGenerator::next("imp"), importName, "module");
                    module->addChild("imports", imp);
                }
            } else if (type == "function_item") {
                auto* fn = convertRustFunction(child, source, "");
                if (fn) module->addChild("functions", fn);
            } else if (type == "impl_item") {
                convertRustImpl(child, source, module);
            } else if (type == "struct_item" || type == "enum_item" || type == "trait_item") {
                // Record type name as a custom type variable for visibility.
                TSNode nameNode = childByFieldName(child, "name");
                if (!ts_node_is_null(nameNode)) {
                    auto* var = new Variable(IdGenerator::next("var"), nodeText(nameNode, source));
                    module->addChild("variables", var);
                }
            }
        }
    }

    static void convertRustImpl(TSNode node,
                                const std::string& source,
                                Module* module) {
        TSNode typeNode = childByFieldName(node, "type");
        std::string typeName = nodeText(typeNode, source);
        TSNode bodyNode = childByFieldName(node, "body");
        if (ts_node_is_null(bodyNode)) return;
        uint32_t count = ts_node_named_child_count(bodyNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(bodyNode, i);
            if (nodeType(child) == "function_item" || nodeType(child) == "function_signature_item") {
                auto* fn = convertRustFunction(child, source, typeName);
                if (fn) module->addChild("functions", fn);
            }
        }
    }

    static Function* convertRustFunction(TSNode node,
                                         const std::string& source,
                                         const std::string& receiverType) {
        TSNode nameNode = childByFieldName(node, "name");
        if (ts_node_is_null(nameNode)) return nullptr;
        auto* fn = new Function();
        fn->id = IdGenerator::next("fn");
        applySpan(fn, node);
        std::string name = nodeText(nameNode, source);
        if (!receiverType.empty()) {
            fn->name = receiverType + "." + name;
        } else {
            fn->name = name;
        }

        TSNode paramsNode = childByFieldName(node, "parameters");
        if (!ts_node_is_null(paramsNode)) {
            convertRustParameters(paramsNode, source, fn);
        }

        TSNode retNode = childByFieldName(node, "return_type");
        if (!ts_node_is_null(retNode)) {
            if (auto* t = convertRustType(retNode, source)) fn->setChild("returnType", t);
        }

        TSNode bodyNode = childByFieldName(node, "body");
        if (!ts_node_is_null(bodyNode)) {
            convertRustBlock(bodyNode, source, fn);
        }

        auto* lifetime = new LifetimeAnnotation(IdGenerator::next("anno"), "RAII");
        fn->addChild("annotations", lifetime);
        return fn;
    }

    static void convertRustParameters(TSNode paramsNode,
                                      const std::string& source,
                                      Function* fn) {
        uint32_t count = ts_node_named_child_count(paramsNode);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_named_child(paramsNode, i);
            std::string type = nodeType(child);
            if (type == "self_parameter") {
                auto* param = new Parameter(IdGenerator::next("param"), "self");
                applySpan(param, child);
                fn->addChild("parameters", param);
            } else if (type == "parameter") {
                TSNode patternNode = childByFieldName(child, "pattern");
                TSNode typeNode = childByFieldName(child, "type");
                if (ts_node_is_null(patternNode)) continue;
                auto* param = new Parameter(IdGenerator::next("param"), nodeText(patternNode, source));
                applySpan(param, child);
                if (!ts_node_is_null(typeNode)) {
                    if (auto* t = convertRustType(typeNode, source)) param->setChild("type", t);
                }
                fn->addChild("parameters", param);
            }
        }
    }

    static void convertRustBlock(TSNode blockNode,
                                 const std::string& source,
                                 Function* fn) {
        uint32_t count = ts_node_named_child_count(blockNode);
        for (uint32_t i = 0; i < count; ++i) {
            ASTNode* stmt = convertRustStatement(ts_node_named_child(blockNode, i), source);
            if (stmt) fn->addChild("body", stmt);
        }
    }

    static ASTNode* convertRustStatement(TSNode node,
                                         const std::string& source) {
        std::string type = nodeType(node);
        if (type == "return_expression") {
            auto* ret = new Return();
            ret->id = IdGenerator::next("ret");
            applySpan(ret, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* val = convertRustExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "let_declaration") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode patternNode = childByFieldName(node, "pattern");
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(patternNode)) {
                auto* target = new VariableReference(IdGenerator::next("var"), nodeText(patternNode, source));
                assign->setChild("target", target);
            }
            if (!ts_node_is_null(valueNode)) {
                ASTNode* val = convertRustExpression(valueNode, source);
                if (val) assign->setChild("value", val);
            }
            return assign;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            if (ts_node_named_child_count(node) > 0) {
                ASTNode* expr = convertRustExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "if_expression") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            applySpan(ifStmt, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertRustExpression(condNode, source);
                if (cond) ifStmt->setChild("condition", cond);
            }
            TSNode consNode = childByFieldName(node, "consequence");
            if (!ts_node_is_null(consNode)) {
                uint32_t cc = ts_node_named_child_count(consNode);
                for (uint32_t i = 0; i < cc; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(consNode, i), source);
                    if (s) ifStmt->addChild("thenBranch", s);
                }
            }
            TSNode altNode = childByFieldName(node, "alternative");
            if (!ts_node_is_null(altNode)) {
                uint32_t ac = ts_node_named_child_count(altNode);
                for (uint32_t i = 0; i < ac; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(altNode, i), source);
                    if (s) ifStmt->addChild("elseBranch", s);
                }
            }
            return ifStmt;
        } else if (type == "while_expression") {
            auto* loop = new WhileLoop();
            loop->id = IdGenerator::next("while");
            applySpan(loop, node);
            TSNode condNode = childByFieldName(node, "condition");
            if (!ts_node_is_null(condNode)) {
                ASTNode* cond = convertRustExpression(condNode, source);
                if (cond) loop->setChild("condition", cond);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "for_expression") {
            auto* loop = new ForLoop();
            loop->id = IdGenerator::next("for");
            applySpan(loop, node);
            TSNode patternNode = childByFieldName(node, "pattern");
            if (!ts_node_is_null(patternNode)) {
                loop->iteratorName = nodeText(patternNode, source);
            }
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                ASTNode* iter = convertRustExpression(valueNode, source);
                if (iter) loop->setChild("iterable", iter);
            }
            TSNode bodyNode = childByFieldName(node, "body");
            if (!ts_node_is_null(bodyNode)) {
                uint32_t bc = ts_node_named_child_count(bodyNode);
                for (uint32_t i = 0; i < bc; ++i) {
                    ASTNode* s = convertRustStatement(ts_node_named_child(bodyNode, i), source);
                    if (s) loop->addChild("body", s);
                }
            }
            return loop;
        } else if (type == "block") {
            auto* block = new Block();
            block->id = IdGenerator::next("block");
            applySpan(block, node);
            uint32_t bc = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < bc; ++i) {
                ASTNode* s = convertRustStatement(ts_node_named_child(node, i), source);
                if (s) block->addChild("statements", s);
            }
            return block;
        }

        ASTNode* expr = convertRustExpression(node, source);
        if (expr) {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            applySpan(exprStmt, node);
            exprStmt->setChild("expression", expr);
            return exprStmt;
        }
        return nullptr;
    }

    static ASTNode* convertRustExpression(TSNode node,
                                          const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
            applySpan(binOp, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            TSNode opNode = childByFieldName(node, "operator");
            if (!ts_node_is_null(opNode)) binOp->op = nodeText(opNode, source);
            if (!ts_node_is_null(leftNode)) {
                ASTNode* left = convertRustExpression(leftNode, source);
                if (left) binOp->setChild("left", left);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* right = convertRustExpression(rightNode, source);
                if (right) binOp->setChild("right", right);
            }
            return binOp;
        } else if (type == "assignment_expression" || type == "compound_assignment_expr") {
            auto* assign = new Assignment();
            assign->id = IdGenerator::next("assign");
            applySpan(assign, node);
            TSNode leftNode = childByFieldName(node, "left");
            TSNode rightNode = childByFieldName(node, "right");
            if (!ts_node_is_null(leftNode)) {
                ASTNode* target = convertRustExpression(leftNode, source);
                if (target) assign->setChild("target", target);
            }
            if (!ts_node_is_null(rightNode)) {
                ASTNode* value = convertRustExpression(rightNode, source);
                if (value) assign->setChild("value", value);
            }
            return assign;
        } else if (type == "call_expression") {
            auto* call = new FunctionCall();
            call->id = IdGenerator::next("call");
            applySpan(call, node);
            TSNode funcNode = childByFieldName(node, "function");
            if (!ts_node_is_null(funcNode)) {
                call->functionName = nodeText(funcNode, source);
            }
            TSNode argsNode = childByFieldName(node, "arguments");
            if (!ts_node_is_null(argsNode)) {
                uint32_t count = ts_node_named_child_count(argsNode);
                for (uint32_t i = 0; i < count; ++i) {
                    ASTNode* arg = convertRustExpression(ts_node_named_child(argsNode, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
            }
            return call;
        } else if (type == "field_expression") {
            auto* mem = new MemberAccess();
            mem->id = IdGenerator::next("member");
            applySpan(mem, node);
            TSNode valueNode = childByFieldName(node, "value");
            TSNode fieldNode = childByFieldName(node, "field");
            if (!ts_node_is_null(fieldNode)) {
                mem->memberName = nodeText(fieldNode, source);
            }
            if (!ts_node_is_null(valueNode)) {
                ASTNode* target = convertRustExpression(valueNode, source);
                if (target) mem->setChild("target", target);
            }
            return mem;
        } else if (type == "index_expression") {
            auto* access = new IndexAccess();
            access->id = IdGenerator::next("index");
            applySpan(access, node);
            if (ts_node_named_child_count(node) >= 2) {
                ASTNode* target = convertRustExpression(ts_node_named_child(node, 0), source);
                ASTNode* idx = convertRustExpression(ts_node_named_child(node, 1), source);
                if (target) access->setChild("target", target);
                if (idx) access->setChild("index", idx);
            }
            return access;
        } else if (type == "identifier") {
            auto* ref = new VariableReference(IdGenerator::next("var"), nodeText(node, source));
            applySpan(ref, node);
            return ref;
        } else if (type == "integer_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            applySpan(lit, node);
            return lit;
        } else if (type == "float_literal") {
            auto* lit = new FloatLiteral(IdGenerator::next("float"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "string_literal" || type == "char_literal") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            applySpan(lit, node);
            return lit;
        } else if (type == "true" || type == "false") {
            auto* lit = new BooleanLiteral(IdGenerator::next("bool"), type == "true");
            applySpan(lit, node);
            return lit;
        } else if (type == "unit_expression") {
            auto* lit = new NullLiteral();
            lit->id = IdGenerator::next("null");
            applySpan(lit, node);
            return lit;
        } else if (type == "parenthesized_expression") {
            if (ts_node_named_child_count(node) > 0) {
                return convertRustExpression(ts_node_named_child(node, 0), source);
            }
        } else if (type == "array_expression") {
            auto* list = new ListLiteral();
            list->id = IdGenerator::next("list");
            applySpan(list, node);
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                ASTNode* elem = convertRustExpression(ts_node_named_child(node, i), source);
                if (elem) list->addChild("elements", elem);
            }
            return list;
        }

        std::string text = nodeText(node, source);
        if (!text.empty()) {
            auto* ref = new VariableReference(IdGenerator::next("var"), text);
            applySpan(ref, node);
            return ref;
        }
        return nullptr;
    }

    static Type* convertRustType(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "primitive_type") {
            auto* prim = new PrimitiveType();
            prim->id = IdGenerator::next("type");
            prim->kind = nodeText(node, source);
            return prim;
        } else if (type == "reference_type") {
            TSNode valueNode = childByFieldName(node, "value");
            if (!ts_node_is_null(valueNode)) {
                if (auto* inner = convertRustType(valueNode, source)) {
                    auto* opt = new OptionalType();
                    opt->id = IdGenerator::next("type");
                    opt->setChild("innerType", inner);
                    return opt;
                }
            }
        } else if (type == "array_type") {
            auto* arr = new ArrayType();
            arr->id = IdGenerator::next("type");
            TSNode elemNode = childByFieldName(node, "element");
            if (!ts_node_is_null(elemNode)) {
                if (auto* et = convertRustType(elemNode, source)) arr->setChild("elementType", et);
            }
            return arr;
        } else if (type == "tuple_type") {
            auto* tuple = new TupleType();
            tuple->id = IdGenerator::next("type");
            uint32_t count = ts_node_named_child_count(node);
            for (uint32_t i = 0; i < count; ++i) {
                if (auto* t = convertRustType(ts_node_named_child(node, i), source)) {
                    tuple->addChild("elementTypes", t);
                }
            }
            return tuple;
        }
        auto* custom = new CustomType();
        custom->id = IdGenerator::next("type");
        custom->typeName = nodeText(node, source);
        return custom;
    }
};
