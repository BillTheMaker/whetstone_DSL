#pragma once

#include "ElispGenerator.h"
#include "ClassDeclaration.h"
#include <sstream>
#include <string>
#include <vector>

class CommonLispGenerator : public ElispGenerator {
public:
    std::string commentPrefix() const { return ";; "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, ";; Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        for (auto* var : module->getChildren("variables")) {
            oss << visitVariable(static_cast<const Variable*>(var)) << "\n";
        }
        for (auto* cls : module->getChildren("classes")) {
            oss << generate(cls) << "\n\n";
        }
        for (auto* stmt : module->getChildren("statements")) {
            std::string text = generate(stmt);
            if (!text.empty()) oss << text << "\n";
        }
        for (size_t i = 0; i < module->getChildren("functions").size(); ++i) {
            if (i > 0 || !module->getChildren("variables").empty() || !module->getChildren("classes").empty()) {
                oss << "\n";
            }
            oss << generate(module->getChildren("functions")[i]);
        }
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        emitAnnotations(oss, function);
        oss << "(defun " << function->name << " (";

        auto params = function->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << " ";
            oss << visitParameter(static_cast<const Parameter*>(params[i]));
        }
        oss << ")\n";

        std::vector<std::string> typeDecls;
        for (auto* paramNode : params) {
            auto* p = static_cast<const Parameter*>(paramNode);
            auto* typeNode = p->getChild("type");
            if (!typeNode) continue;
            typeDecls.push_back("(type " + mapTypeName(typeNode) + " " + p->name + ")");
        }
        if (!typeDecls.empty()) {
            oss << "  (declare";
            for (const auto& decl : typeDecls) oss << " " << decl;
            oss << ")\n";
        }

        bool hasException = hasExceptionAnnotation(function);
        if (hasException) {
            oss << "  (handler-case\n";
            oss << "      (progn\n";
        }

        const auto& body = function->getChildren("body");
        if (body.empty()) {
            oss << (hasException ? "        nil\n" : "  nil\n");
        } else {
            for (auto* stmt : body) {
                std::string line = generate(stmt);
                if (line.empty()) continue;
                oss << (hasException ? "        " : "  ") << line << "\n";
            }
        }

        if (hasException) {
            oss << "      )\n";
            oss << "    (error (e) e))";
        } else {
            oss << ")";
        }
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        std::string name = variable->name;
        std::string keyword = isDynamicName(name) ? "defparameter" : "defvar";

        oss << "(" << keyword << " " << name;
        auto* init = variable->getChild("value");
        if (init) oss << " " << generate(init);
        else oss << " nil";
        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::vector<const Variable*> vars;
        std::vector<ASTNode*> body;
        for (auto* stmt : block->getChildren("statements")) {
            if (stmt->conceptType == "Variable") {
                vars.push_back(static_cast<const Variable*>(stmt));
            } else {
                body.push_back(stmt);
            }
        }

        std::ostringstream oss;
        if (!vars.empty()) {
            oss << "(let (";
            for (size_t i = 0; i < vars.size(); ++i) {
                if (i > 0) oss << " ";
                oss << "(" << vars[i]->name;
                auto* init = vars[i]->getChild("value");
                if (init) oss << " " << generate(init);
                oss << ")";
            }
            oss << ")";
            for (auto* stmt : body) oss << " " << generate(stmt);
            oss << ")";
            return oss.str();
        }

        oss << "(progn";
        for (auto* stmt : body) oss << " " << generate(stmt);
        oss << ")";
        return oss.str();
    }

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;

        oss << "(defclass " << cls->name << " (";
        auto bases = cls->getBases();
        for (size_t i = 0; i < bases.size(); ++i) {
            if (i > 0) oss << " ";
            oss << bases[i].name;
        }
        oss << ") (";

        auto fields = cls->getChildren("fields");
        for (size_t i = 0; i < fields.size(); ++i) {
            if (i > 0) oss << " ";
            auto* var = static_cast<const Variable*>(fields[i]);
            oss << "(" << var->name << " :initarg :" << var->name << ")";
        }
        oss << "))";
        return oss.str();
    }

    std::string visitMethodDeclaration(const ASTNode* node) override {
        auto* method = static_cast<const MethodDeclaration*>(node);
        std::ostringstream oss;

        emitAnnotations(oss, method);
        oss << "(defmethod " << method->name << " (";

        auto params = method->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << " ";
            auto* p = static_cast<const Parameter*>(params[i]);
            oss << "(" << p->name << " " << mapTypeName(p->getChild("type")) << ")";
        }
        oss << ")\n";

        if (method->getChildren("body").empty()) {
            oss << "  nil)";
            return oss.str();
        }

        for (auto* stmt : method->getChildren("body")) {
            oss << "  " << generate(stmt) << "\n";
        }
        oss << ")";
        return oss.str();
    }

    std::string visitLambdaExpression(const ASTNode* node) override {
        auto* lambda = static_cast<const LambdaExpression*>(node);
        std::ostringstream oss;
        oss << "(lambda (";

        auto params = lambda->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << " ";
            oss << static_cast<const Parameter*>(params[i])->name;
        }
        oss << ")";

        for (auto* stmt : lambda->getChildren("body")) {
            oss << " " << generate(stmt);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitMacroDefinition(const ASTNode* node) override {
        auto* macro = static_cast<const MacroDefinition*>(node);
        std::ostringstream oss;
        oss << "(defmacro " << macro->name << " (";
        for (size_t i = 0; i < macro->parameters.size(); ++i) {
            if (i > 0) oss << " ";
            oss << macro->parameters[i];
        }
        oss << ") " << (macro->body.empty() ? "nil" : macro->body) << ")";
        return oss.str();
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        oss << "(" << call->functionName;
        for (auto* arg : call->getChildren("arguments")) {
            oss << " " << generate(arg);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        return mapTypeName(type);
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        return ";; @owner(" + annotation->strategy + ")";
    }

private:
    static bool isDynamicName(const std::string& name) {
        return name.size() > 2 && name.front() == '*' && name.back() == '*';
    }

    static std::string mapTypeName(const ASTNode* typeNode) {
        if (!typeNode) return "t";

        std::string t;
        if (typeNode->conceptType == "PrimitiveType") {
            t = static_cast<const PrimitiveType*>(typeNode)->kind;
        } else if (typeNode->conceptType == "CustomType") {
            t = static_cast<const CustomType*>(typeNode)->typeName;
        }

        if (t == "int" || t == "i32") return "fixnum";
        if (t == "long" || t == "i64") return "integer";
        if (t == "float" || t == "f32") return "single-float";
        if (t == "double" || t == "f64") return "double-float";
        if (t == "string") return "string";
        if (t == "bool") return "boolean";
        if (t.empty()) return "t";
        return t;
    }

    bool hasExceptionAnnotation(const Function* function) const {
        for (auto* anno : function->getChildren("annotations")) {
            if (anno->conceptType == "ExceptionAnnotation") return true;
        }
        return false;
    }

    void emitAnnotations(std::ostringstream& oss, const ASTNode* node) {
        for (auto* anno : node->getChildren("annotations")) {
            std::string text = generate(anno);
            if (!text.empty()) oss << text << "\n";
        }
    }
};
