#pragma once

#include "ProjectionGenerator.h"
#include "ClassDeclaration.h"
#include "AsyncNodes.h"
#include "EnumNamespaceNodes.h"
#include "../SemannoAnnotationImpl.h"

class FSharpGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<FSharpGenerator> {
public:
    std::string commentPrefix() const { return "// "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "// Module: " << module->name << "\n\n";

        for (const auto* stmt : module->getChildren("statements")) {
            oss << generate(stmt) << "\n";
        }
        if (!module->getChildren("statements").empty()) oss << "\n";

        for (const auto* var : module->getChildren("variables")) {
            oss << generate(var) << "\n";
        }
        if (!module->getChildren("variables").empty()) oss << "\n";

        for (const auto* fn : module->getChildren("functions")) {
            oss << generate(fn) << "\n";
        }

        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        for (const auto* anno : function->getChildren("annotations")) {
            oss << generate(anno) << "\n";
        }

        oss << "let " << function->name;
        auto params = function->getChildren("parameters");
        for (const auto* p : params) {
            oss << " " << static_cast<const Parameter*>(p)->name;
        }
        oss << " =";

        auto body = function->getChildren("body");
        if (body.empty()) {
            oss << " ()";
            return oss.str();
        }

        if (body.size() == 1) {
            oss << " " << generate(body[0]);
            return oss.str();
        }

        oss << "\n";
        for (const auto* stmt : body) {
            oss << "    " << generate(stmt) << "\n";
        }
        return trimTrailingNewline(oss.str());
    }

    std::string visitAsyncFunction(const ASTNode* node) override {
        auto* fn = static_cast<const AsyncFunction*>(node);
        std::ostringstream oss;
        for (const auto* anno : fn->getChildren("annotations")) {
            oss << generate(anno) << "\n";
        }
        oss << "let " << fn->name;
        auto params = fn->getChildren("parameters");
        for (const auto* p : params) {
            oss << " " << static_cast<const Parameter*>(p)->name;
        }
        oss << " = async {\n";
        auto body = fn->getChildren("body");
        for (const auto* stmt : body) {
            oss << "    " << generate(stmt) << "\n";
        }
        oss << "}";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        bool isMutable = false;
        for (const auto* anno : variable->getChildren("annotations")) {
            if (anno->conceptType == "MutAnnotation") {
                isMutable = true;
                break;
            }
        }

        std::ostringstream oss;
        oss << "let ";
        if (isMutable) oss << "mutable ";
        oss << variable->name;
        auto init = variable->getChild("initializer");
        if (init) oss << " = " << generate(init);
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        return parameter->name;
    }

    std::string visitReturn(const Return* r) override {
        auto value = r->getChild("value");
        return value ? generate(value) : "()";
    }

    std::string visitAssignment(const Assignment* a) override {
        auto target = a->getChild("target");
        auto value = a->getChild("value");
        return (target ? generate(target) : "_") + " <- " + (value ? generate(value) : "()");
    }

    std::string visitIfStatement(const IfStatement* s) override {
        std::ostringstream oss;
        auto cond = s->getChild("condition");
        oss << "if " << (cond ? generate(cond) : "true") << " then";
        auto thenBranch = s->getChildren("thenBranch");
        auto elseBranch = s->getChildren("elseBranch");
        if (!thenBranch.empty()) oss << " " << generate(thenBranch.front());
        else oss << " ()";
        if (!elseBranch.empty()) oss << " else " << generate(elseBranch.front());
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* s) override {
        auto expr = s->getChild("expression");
        return expr ? generate(expr) : "()";
    }

    std::string visitBinaryOperation(const BinaryOperation* b) override {
        auto left = b->getChild("left");
        auto right = b->getChild("right");
        return (left ? generate(left) : "") + " " + b->op + " " + (right ? generate(right) : "");
    }

    std::string visitUnaryOperation(const UnaryOperation* u) override {
        auto operand = u->getChild("operand");
        return u->op + (operand ? generate(operand) : "");
    }

    std::string visitFunctionCall(const FunctionCall* c) override {
        // Parser uses this marker for F# pipe chains.
        if (c->functionName == "pipeline") return "|> pipeline";

        std::ostringstream oss;
        oss << c->functionName;
        auto args = c->getChildren("arguments");
        for (const auto* arg : args) oss << " " << generate(arg);
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* l) override {
        std::ostringstream oss;
        auto cond = l->getChild("condition");
        oss << "while " << (cond ? generate(cond) : "true") << " do";
        auto body = l->getChildren("body");
        if (body.empty()) return oss.str() + " ()";
        oss << "\n";
        for (const auto* s : body) oss << "    " << generate(s) << "\n";
        return trimTrailingNewline(oss.str());
    }

    std::string visitForLoop(const ForLoop* l) override {
        std::ostringstream oss;
        auto iter = l->getChild("iterable");
        oss << "for " << (l->iteratorName.empty() ? "item" : l->iteratorName)
            << " in " << (iter ? generate(iter) : "[]") << " do";
        auto body = l->getChildren("body");
        if (body.empty()) return oss.str() + " ()";
        oss << "\n";
        for (const auto* s : body) oss << "    " << generate(s) << "\n";
        return trimTrailingNewline(oss.str());
    }

    std::string visitBlock(const Block* b) override {
        std::ostringstream oss;
        for (const auto* s : b->getChildren("statements")) {
            oss << generate(s) << "\n";
        }
        return trimTrailingNewline(oss.str());
    }

    std::string visitListLiteral(const ListLiteral* l) override {
        std::ostringstream oss;
        oss << "[";
        auto elems = l->getChildren("elements");
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) oss << "; ";
            oss << generate(elems[i]);
        }
        oss << "]";
        return oss.str();
    }

    std::string visitIndexAccess(const IndexAccess* a) override {
        auto* target = a->getChild("target");
        auto* idx = a->getChild("index");
        return (target ? generate(target) : "") + "[" + (idx ? generate(idx) : "") + "]";
    }

    std::string visitMemberAccess(const MemberAccess* a) override {
        auto* target = a->getChild("target");
        return (target ? generate(target) : "") + "." + a->memberName;
    }

    std::string visitVariableReference(const VariableReference* v) override {
        return v->variableName;
    }
    std::string visitIntegerLiteral(const IntegerLiteral* l) override { return std::to_string(l->value); }
    std::string visitFloatLiteral(const FloatLiteral* l) override { return l->value; }
    std::string visitStringLiteral(const StringLiteral* l) override { return "\"" + l->value + "\""; }
    std::string visitBooleanLiteral(const BooleanLiteral* l) override { return l->value ? "true" : "false"; }
    std::string visitNullLiteral(const NullLiteral*) override { return "null"; }

    std::string visitPrimitiveType(const PrimitiveType* t) override {
        if (t->kind == "int") return "int";
        if (t->kind == "float" || t->kind == "double") return "float";
        if (t->kind == "string") return "string";
        if (t->kind == "bool") return "bool";
        if (t->kind == "void") return "unit";
        return t->kind;
    }
    std::string visitListType(const ListType* t) override {
        auto* e = t->getChild("elementType");
        return (e ? generate(e) : "obj") + " list";
    }
    std::string visitSetType(const SetType* t) override {
        auto* e = t->getChild("elementType");
        return (e ? generate(e) : "obj") + " seq";
    }
    std::string visitMapType(const MapType* t) override {
        auto* k = t->getChild("keyType");
        auto* v = t->getChild("valueType");
        return "(" + std::string(k ? generate(k) : "obj") + " * " +
               std::string(v ? generate(v) : "obj") + ") list";
    }
    std::string visitTupleType(const TupleType* t) override {
        std::ostringstream oss;
        auto elems = t->getChildren("elementTypes");
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) oss << " * ";
            oss << generate(elems[i]);
        }
        return oss.str();
    }
    std::string visitArrayType(const ArrayType* t) override {
        auto* e = t->getChild("elementType");
        return (e ? generate(e) : "obj") + " array";
    }
    std::string visitOptionalType(const OptionalType* t) override {
        auto* inner = t->getChild("innerType");
        return (inner ? generate(inner) : "obj") + " option";
    }
    std::string visitCustomType(const CustomType* t) override {
        if (t->typeName == "result") return "result";
        return t->typeName;
    }

    // Subject 1 annotations not covered by SemannoAnnotationImpl.
    std::string visitDerefStrategy(const DerefStrategy* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitOptimizationLock(const OptimizationLock* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitLangSpecific(const LangSpecific* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitDeallocateAnnotation(const DeallocateAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitLifetimeAnnotation(const LifetimeAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitReclaimAnnotation(const ReclaimAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitOwnerAnnotation(const OwnerAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitAllocateAnnotation(const AllocateAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitHotColdAnnotation(const HotColdAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitInlineAnnotation(const InlineAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitPureAnnotation(const PureAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }
    std::string visitConstExprAnnotation(const ConstExprAnnotation* a) override {
        return commentPrefix() + SemannoEmitter::emit(a);
    }

    std::string visitNamespaceDeclaration(const ASTNode* node) override {
        auto* ns = static_cast<const NamespaceDeclaration*>(node);
        return "module " + ns->name;
    }

    std::string visitEnumDeclaration(const ASTNode* node) override {
        auto* en = static_cast<const EnumDeclaration*>(node);
        std::ostringstream oss;
        oss << "type " << en->name << " =";
        auto members = en->getChildren("members");
        if (members.empty()) {
            oss << " | Unknown";
            return oss.str();
        }
        for (const auto* m : members) {
            auto* em = static_cast<const EnumMember*>(m);
            oss << "\n    | " << em->name;
        }
        return oss.str();
    }

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        oss << "type " << cls->name << " = {";
        auto fields = cls->getChildren("fields");
        if (!fields.empty()) oss << " ";
        for (size_t i = 0; i < fields.size(); ++i) {
            auto* v = static_cast<const Variable*>(fields[i]);
            if (i > 0) oss << "; ";
            oss << v->name << ": obj";
        }
        if (!fields.empty()) oss << " ";
        oss << "}";
        return oss.str();
    }

private:
    static std::string trimTrailingNewline(std::string s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
        return s;
    }
};
