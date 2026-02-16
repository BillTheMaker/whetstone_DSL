#pragma once

#include "CppGenerator.h"
#include "Import.h"

class WatGenerator : public CppGenerator {
public:
    std::string commentPrefix() const { return ";; "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, ";; Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "(module\n";

        for (auto* imp : module->getChildren("imports")) {
            if (imp->conceptType == "Import") {
                oss << "  " << emitImportWat(static_cast<const Import*>(imp)) << "\n";
            }
        }

        for (auto* stmt : module->getChildren("statements")) {
            oss << "  " << generate(stmt) << "\n";
        }

        for (auto* var : module->getChildren("variables")) {
            oss << "  " << "(global $" << static_cast<Variable*>(var)->name
                << " (mut i32) (i32.const 0))\n";
        }

        for (auto* fn : module->getChildren("functions")) {
            oss << "  " << generate(fn) << "\n";
        }

        oss << ")";
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        oss << "(func $" << sanitizeName(function->name);

        for (auto* pNode : function->getChildren("parameters")) {
            auto* p = static_cast<const Parameter*>(pNode);
            auto* t = p->getChild("type");
            oss << " (param $" << sanitizeName(p->name) << " "
                << watType(t) << ")";
        }

        auto* ret = function->getChild("returnType");
        if (ret && watType(ret) != "") {
            oss << " (result " << watType(ret) << ")";
        }

        auto annos = function->getChildren("annotations");
        if (!annos.empty()) {
            for (auto* a : annos) {
                oss << " " << generate(a);
            }
        }

        auto body = function->getChildren("body");
        if (!body.empty()) {
            for (auto* s : body) {
                oss << " " << generate(s);
            }
        }

        oss << ")";
        return oss.str();
    }

    std::string visitBinaryOperation(const BinaryOperation* binOp) override {
        std::string op = "i32.add";
        if (binOp->op == "-" || binOp->op == "sub") op = "i32.sub";
        else if (binOp->op == "*" || binOp->op == "mul") op = "i32.mul";

        std::ostringstream oss;
        emitOperand(oss, binOp->getChild("left"));
        oss << " ";
        emitOperand(oss, binOp->getChild("right"));
        oss << " " << op;
        return oss.str();
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        for (auto* arg : call->getChildren("arguments")) {
            emitOperand(oss, arg);
            oss << " ";
        }
        oss << "call $" << sanitizeName(call->functionName);
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        auto* expr = stmt->getChild("expression");
        return expr ? generate(expr) : "nop";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;
        oss << "(if";
        if (auto* cond = stmt->getChild("condition")) {
            oss << " (" << generate(cond) << ")";
        }

        oss << " (then";
        for (auto* t : stmt->getChildren("thenBranch")) {
            oss << " " << generate(t);
        }
        oss << ")";

        auto elseBranch = stmt->getChildren("elseBranch");
        if (!elseBranch.empty()) {
            oss << " (else";
            for (auto* e : elseBranch) oss << " " << generate(e);
            oss << ")";
        }

        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::ostringstream oss;
        oss << "(block";
        for (auto* s : block->getChildren("statements")) {
            oss << " " << generate(s);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitVariableReference(const VariableReference* ref) override {
        return "local.get $" + sanitizeName(ref->variableName);
    }

    std::string visitIntegerLiteral(const IntegerLiteral* lit) override {
        return "i32.const " + std::to_string(lit->value);
    }

    std::string visitFloatLiteral(const FloatLiteral* lit) override {
        return "f32.const " + lit->value;
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return std::string("i32.const ") + (lit->value ? "1" : "0");
    }

    std::string visitPragmaDirective(const ASTNode* node) override {
        auto* p = static_cast<const PragmaDirective*>(node);
        if (p->directive == "export") return "(export \"f\" (func $f))";
        if (p->directive == "memory") return "(memory 1)";
        if (p->directive == "table") return "(table 1 funcref)";
        return "(; pragma " + p->directive + " ;)";
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        return ";; @owner(" + annotation->strategy + ")";
    }

private:
    static std::string emitImportWat(const Import* imp) {
        std::string moduleName = imp->moduleName;
        std::string importName = imp->alias.empty() ? imp->moduleName : imp->alias;
        auto dot = moduleName.find('.');
        if (dot != std::string::npos) {
            importName = moduleName.substr(dot + 1);
            moduleName = moduleName.substr(0, dot);
        }
        return "(import \"" + moduleName + "\" \"" + importName + "\" (func $" +
               sanitizeName(importName) + "))";
    }

    static std::string sanitizeName(const std::string& name) {
        if (name.empty()) return "fn";
        std::string out = name;
        for (auto& c : out) {
            if (c == '$' || c == '-' || c == ':') c = '_';
        }
        return out;
    }

    static std::string watType(const ASTNode* typeNode) {
        if (!typeNode) return "";
        std::string kind;
        if (typeNode->conceptType == "PrimitiveType") {
            kind = static_cast<const PrimitiveType*>(typeNode)->kind;
        } else if (typeNode->conceptType == "CustomType") {
            kind = static_cast<const CustomType*>(typeNode)->typeName;
        }

        if (kind == "int" || kind == "bool" || kind == "i32") return "i32";
        if (kind == "long" || kind == "i64") return "i64";
        if (kind == "float" || kind == "f32") return "f32";
        if (kind == "double" || kind == "f64") return "f64";
        if (kind == "void") return "";
        return kind.empty() ? "i32" : kind;
    }

    static void emitOperand(std::ostringstream& oss, const ASTNode* operand) {
        if (!operand) {
            oss << "i32.const 0";
            return;
        }
        if (operand->conceptType == "VariableReference") {
            oss << "local.get $" << sanitizeName(static_cast<const VariableReference*>(operand)->variableName);
        } else if (operand->conceptType == "IntegerLiteral") {
            oss << "i32.const " << static_cast<const IntegerLiteral*>(operand)->value;
        } else if (operand->conceptType == "FloatLiteral") {
            oss << "f32.const " << static_cast<const FloatLiteral*>(operand)->value;
        } else {
            oss << "i32.const 0";
        }
    }
};
