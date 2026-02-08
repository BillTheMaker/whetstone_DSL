#pragma once
#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
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

        convertElispSourceFile(root, source, result.module.get());
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
        fn->name = nodeText(nameNode, source);

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
                fn->addChild("parameters", param);
            } else if (type == "default_parameter") {
                // def f(x=10) → Parameter with defaultValue
                TSNode nameN = childByFieldName(child, "name");
                TSNode valueN = childByFieldName(child, "value");
                if (!ts_node_is_null(nameN)) {
                    auto* param = new Parameter(IdGenerator::next("param"), nodeText(nameN, source));
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
            return ref;
        } else if (type == "integer") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            auto* lit = new IntegerLiteral(IdGenerator::next("int"), val);
            return lit;
        } else if (type == "string" || type == "concatenated_string") {
            auto* lit = new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
            return lit;
        } else if (type == "comparison_operator" || type == "boolean_operator") {
            // Treat like binary op
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
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

        // parameter_declaration has type and declarator fields
        TSNode typeNode = childByFieldName(paramNode, "type");
        TSNode declNode = childByFieldName(paramNode, "declarator");

        if (!ts_node_is_null(typeNode)) {
            std::string typeText = nodeText(typeNode, source);
            auto* primType = new PrimitiveType(IdGenerator::next("type"), typeText);
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
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* val = convertCppExpression(ts_node_named_child(node, 0), source);
                if (val) ret->setChild("value", val);
            }
            return ret;
        } else if (type == "expression_statement") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) {
                ASTNode* expr = convertCppExpression(ts_node_named_child(node, 0), source);
                if (expr) exprStmt->setChild("expression", expr);
            }
            return exprStmt;
        } else if (type == "declaration") {
            auto* exprStmt = new ExpressionStatement();
            exprStmt->id = IdGenerator::next("exprstmt");
            return exprStmt;
        } else if (type == "if_statement") {
            auto* ifStmt = new IfStatement();
            ifStmt->id = IdGenerator::next("if");
            return ifStmt;
        }
        return nullptr;
    }

    static ASTNode* convertCppExpression(TSNode node, const std::string& source) {
        std::string type = nodeType(node);
        if (type == "binary_expression") {
            auto* binOp = new BinaryOperation();
            binOp->id = IdGenerator::next("binop");
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
            return new VariableReference(IdGenerator::next("var"), nodeText(node, source));
        } else if (type == "number_literal") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            return new IntegerLiteral(IdGenerator::next("int"), val);
        } else if (type == "string_literal" || type == "raw_string_literal") {
            return new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
        } else if (type == "parenthesized_expression") {
            uint32_t count = ts_node_named_child_count(node);
            if (count > 0) return convertCppExpression(ts_node_named_child(node, 0), source);
        }
        // Fallback
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            return new VariableReference(IdGenerator::next("var"), text);
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
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                } else {
                    auto* exprStmt = new ExpressionStatement();
                    exprStmt->id = IdGenerator::next("exprstmt");
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
                    exprStmt->setChild("expression", expr);
                    fn->addChild("body", exprStmt);
                }
            }
        } else {
            ASTNode* expr = convertElispExpression(bodyNode, source);
            if (expr) {
                auto* exprStmt = new ExpressionStatement();
                exprStmt->id = IdGenerator::next("exprstmt");
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
                TSNode funcName = ts_node_named_child(node, 0);
                call->functionName = nodeText(funcName, source);
                for (uint32_t i = 1; i < count; ++i) {
                    ASTNode* arg = convertElispExpression(ts_node_named_child(node, i), source);
                    if (arg) call->addChild("arguments", arg);
                }
                return call;
            }
        } else if (type == "symbol" || type == "identifier") {
            return new VariableReference(IdGenerator::next("var"), nodeText(node, source));
        } else if (type == "integer" || type == "number") {
            std::string text = nodeText(node, source);
            int val = 0;
            try { val = std::stoi(text); } catch (...) {}
            return new IntegerLiteral(IdGenerator::next("int"), val);
        } else if (type == "string") {
            return new StringLiteral(IdGenerator::next("str"), nodeText(node, source));
        } else if (type == "special_form") {
            // Could be (if ...), (let ...), etc.
            return convertElispSpecialForm(node, source);
        }

        // Fallback
        std::string text = nodeText(node, source);
        if (!text.empty()) {
            return new VariableReference(IdGenerator::next("var"), text);
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
            ASTNode* cond = convertElispExpression(ts_node_named_child(node, 1), source);
            if (cond) ifStmt->setChild("condition", cond);
            return ifStmt;
        }

        // Generic: treat as function call
        auto* call = new FunctionCall();
        call->id = IdGenerator::next("call");
        call->functionName = formName;
        for (uint32_t i = 1; i < count; ++i) {
            ASTNode* arg = convertElispExpression(ts_node_named_child(node, i), source);
            if (arg) call->addChild("arguments", arg);
        }
        return call;
    }
};
