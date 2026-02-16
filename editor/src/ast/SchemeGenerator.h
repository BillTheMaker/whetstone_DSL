#pragma once

#include "CommonLispGenerator.h"

class SchemeGenerator : public CommonLispGenerator {
public:
    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "; Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        for (auto* var : module->getChildren("variables")) {
            oss << generate(var) << "\n";
        }
        if (!module->getChildren("variables").empty()) oss << "\n";

        for (auto* cls : module->getChildren("classes")) {
            oss << generate(cls) << "\n\n";
        }

        for (auto* fn : module->getChildren("functions")) {
            oss << generate(fn) << "\n\n";
        }
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        emitAnnotations(oss, function);

        oss << "(define (" << function->name;
        for (auto* pNode : function->getChildren("parameters")) {
            auto* p = static_cast<const Parameter*>(pNode);
            oss << " " << p->name;
        }
        oss << ")";

        auto body = function->getChildren("body");
        if (body.empty()) {
            oss << " '())";
            return oss.str();
        }

        for (auto* stmt : body) {
            oss << " " << generate(stmt);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << "(define " << variable->name;
        auto* val = variable->getChild("value");
        if (val) oss << " " << generate(val);
        else oss << " '()";
        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::vector<const Variable*> vars;
        std::vector<ASTNode*> body;
        for (auto* stmt : block->getChildren("statements")) {
            if (stmt->conceptType == "Variable") vars.push_back(static_cast<const Variable*>(stmt));
            else body.push_back(stmt);
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

        oss << "(begin";
        for (auto* stmt : body) oss << " " << generate(stmt);
        oss << ")";
        return oss.str();
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "#t" : "#f";
    }

    std::string visitNullLiteral(const NullLiteral* lit) override {
        (void)lit;
        return "'()";
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        bool continuation = false;
        for (auto* anno : call->getChildren("annotations")) {
            if (anno->conceptType == "ExecAnnotation") {
                auto* ex = static_cast<const ExecAnnotation*>(anno);
                if (ex->mode == "continuation") continuation = true;
            }
        }

        std::string fnName = call->functionName;
        if (continuation || fnName == "call-with-current-continuation") {
            fnName = "call/cc";
        }

        std::ostringstream oss;
        oss << "(" << fnName;
        for (auto* arg : call->getChildren("arguments")) {
            oss << " " << generate(arg);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        oss << "(define-record-type " << cls->name << " (fields";
        for (auto* f : cls->getChildren("fields")) {
            oss << " " << static_cast<const Variable*>(f)->name;
        }
        oss << "))";
        return oss.str();
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        return "; @owner(strategy=" + annotation->strategy + ")";
    }

    std::string visitTailCallAnnotation(const TailCallAnnotation* annotation) override {
        (void)annotation;
        return "; @tailcall()";
    }

    std::string visitExecAnnotation(const ExecAnnotation* annotation) override {
        std::ostringstream oss;
        oss << "; @exec(mode=" << annotation->mode;
        if (!annotation->runtimeHint.empty()) oss << ",runtimeHint=" << annotation->runtimeHint;
        oss << ")";
        return oss.str();
    }

private:
    void emitAnnotations(std::ostringstream& oss, const ASTNode* node) {
        for (auto* anno : node->getChildren("annotations")) {
            std::string text = generate(anno);
            if (!text.empty()) oss << text << "\n";
        }
    }
};
