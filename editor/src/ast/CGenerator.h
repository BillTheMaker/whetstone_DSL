#pragma once
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "CppGenerator.h"

class CGenerator : public CppGenerator {
public:
    std::string commentPrefix() const { return "// "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "/* Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        for (auto* s : module->getChildren("statements")) oss << generate(s) << "\n";
        if (!module->getChildren("statements").empty()) oss << "\n";
        for (auto* c : module->getChildren("classes")) oss << generate(c) << "\n";
        for (auto* v : module->getChildren("variables")) oss << generate(v) << "\n";
        if (!module->getChildren("variables").empty()) oss << "\n";
        for (auto* f : module->getChildren("functions")) oss << generate(f) << "\n";
        return oss.str();
    }

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        oss << "typedef struct " << cls->name << " {\n";
        for (auto* f : cls->getChildren("fields")) {
            std::string line = generate(f);
            if (!line.empty() && line.back() == ';') line.pop_back();
            oss << "    " << line << ";\n";
        }
        oss << "} " << cls->name << ";";
        return oss.str();
    }

    std::string visitMethodDeclaration(const ASTNode* node) override {
        auto* meth = static_cast<const MethodDeclaration*>(node);
        std::ostringstream oss;
        oss << "void " << meth->className << "_" << meth->name << "(";
        auto params = meth->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << visitParameter(static_cast<const Parameter*>(params[i]));
        }
        oss << ") {\n";
        for (auto* s : meth->getChildren("body")) oss << "    " << generate(s) << "\n";
        oss << "}";
        return oss.str();
    }

    std::string visitNullLiteral(const NullLiteral* /*lit*/) override {
        return "NULL";
    }

    std::string visitStringLiteral(const StringLiteral* lit) override {
        return "\"" + lit->value + "\"";
    }

    std::string visitTypeAlias(const ASTNode* node) override {
        auto* ta = static_cast<const TypeAlias*>(node);
        return "typedef " + ta->targetType + " " + ta->aliasName + ";";
    }

    std::string visitEnumDeclaration(const ASTNode* node) override {
        auto* e = static_cast<const EnumDeclaration*>(node);
        std::ostringstream oss;
        oss << "enum " << e->name << " { ";
        auto members = e->getChildren("members");
        for (size_t i = 0; i < members.size(); ++i) {
            auto* m = static_cast<const EnumMember*>(members[i]);
            if (i > 0) oss << ", ";
            oss << m->name;
            if (!m->value.empty()) oss << " = " << m->value;
        }
        oss << " };";
        return oss.str();
    }

    std::string visitIncludeDirective(const ASTNode* node) override {
        auto* inc = static_cast<const IncludeDirective*>(node);
        if (inc->isSystem) return "#include <" + inc->path + ">";
        return "#include \"" + inc->path + "\"";
    }

    std::string visitPragmaDirective(const ASTNode* node) override {
        auto* p = static_cast<const PragmaDirective*>(node);
        return "#pragma " + p->directive;
    }

    std::string visitMacroDefinition(const ASTNode* node) override {
        auto* m = static_cast<const MacroDefinition*>(node);
        std::ostringstream oss;
        oss << "#define " << m->name;
        if (m->isFunctionLike) {
            oss << "(";
            for (size_t i = 0; i < m->parameters.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << m->parameters[i];
            }
            oss << ")";
        }
        if (!m->body.empty()) oss << " " << m->body;
        return oss.str();
    }
};
