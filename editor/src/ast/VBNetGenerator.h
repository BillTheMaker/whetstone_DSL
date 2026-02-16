#pragma once

#include "ProjectionGenerator.h"
#include "ClassDeclaration.h"
#include "EnumNamespaceNodes.h"
#include "../SemannoAnnotationImpl.h"

class VBNetGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<VBNetGenerator> {
public:
    std::string commentPrefix() const { return "' "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "' Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "' Module: " << module->name << "\n\n";
        for (const auto* stmt : module->getChildren("statements")) oss << generate(stmt) << "\n";
        if (!module->getChildren("statements").empty()) oss << "\n";
        for (const auto* var : module->getChildren("variables")) oss << generate(var) << "\n";
        if (!module->getChildren("variables").empty()) oss << "\n";
        for (const auto* fn : module->getChildren("functions")) oss << generate(fn) << "\n";
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        for (const auto* anno : function->getChildren("annotations")) oss << generate(anno) << "\n";

        auto* retType = function->getChild("returnType");
        const bool hasReturn = retType != nullptr;
        oss << (hasReturn ? "Function " : "Sub ") << function->name << "(";
        auto params = function->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")";
        if (hasReturn) oss << " As " << generate(retType);
        oss << "\n";

        for (const auto* stmt : function->getChildren("body")) {
            oss << "    " << generate(stmt) << "\n";
        }
        oss << (hasReturn ? "End Function" : "End Sub");
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << "Dim " << variable->name;
        auto* type = variable->getChild("type");
        if (type) oss << " As " << generate(type);
        auto* init = variable->getChild("initializer");
        if (init) oss << " = " << generate(init);
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        auto* type = parameter->getChild("type");
        if (!type) return parameter->name;
        return parameter->name + " As " + generate(type);
    }

    std::string visitAssignment(const Assignment* a) override {
        auto* t = a->getChild("target");
        auto* v = a->getChild("value");
        return (t ? generate(t) : "") + " = " + (v ? generate(v) : "Nothing");
    }

    std::string visitReturn(const Return* r) override {
        auto* v = r->getChild("value");
        return v ? "Return " + generate(v) : "Return";
    }

    std::string visitBinaryOperation(const BinaryOperation* b) override {
        auto* l = b->getChild("left");
        auto* r = b->getChild("right");
        return (l ? generate(l) : "") + " " + b->op + " " + (r ? generate(r) : "");
    }

    std::string visitVariableReference(const VariableReference* v) override { return v->variableName; }
    std::string visitIntegerLiteral(const IntegerLiteral* l) override { return std::to_string(l->value); }
    std::string visitFloatLiteral(const FloatLiteral* l) override { return l->value; }
    std::string visitStringLiteral(const StringLiteral* l) override { return "\"" + l->value + "\""; }
    std::string visitBooleanLiteral(const BooleanLiteral* l) override { return l->value ? "True" : "False"; }
    std::string visitNullLiteral(const NullLiteral*) override { return "Nothing"; }

    std::string visitIfStatement(const IfStatement* s) override {
        std::ostringstream oss;
        auto* cond = s->getChild("condition");
        oss << "If " << (cond ? generate(cond) : "True") << " Then\n";
        for (const auto* t : s->getChildren("thenBranch")) oss << "    " << generate(t) << "\n";
        auto elseBranch = s->getChildren("elseBranch");
        if (!elseBranch.empty()) {
            oss << "Else\n";
            for (const auto* e : elseBranch) oss << "    " << generate(e) << "\n";
        }
        oss << "End If";
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* l) override {
        std::ostringstream oss;
        auto* cond = l->getChild("condition");
        oss << "While " << (cond ? generate(cond) : "True") << "\n";
        for (const auto* s : l->getChildren("body")) oss << "    " << generate(s) << "\n";
        oss << "End While";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* l) override {
        std::ostringstream oss;
        auto* iter = l->getChild("iterable");
        oss << "For Each " << (l->iteratorName.empty() ? "item" : l->iteratorName)
            << " In " << (iter ? generate(iter) : "collection") << "\n";
        for (const auto* s : l->getChildren("body")) oss << "    " << generate(s) << "\n";
        oss << "Next";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* s) override {
        auto* expr = s->getChild("expression");
        return expr ? generate(expr) : "";
    }

    std::string visitUnaryOperation(const UnaryOperation* u) override {
        auto* o = u->getChild("operand");
        return u->op + (o ? generate(o) : "");
    }

    std::string visitFunctionCall(const FunctionCall* c) override {
        std::ostringstream oss;
        oss << c->functionName << "(";
        auto args = c->getChildren("arguments");
        for (size_t i = 0; i < args.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(args[i]);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* b) override {
        std::ostringstream oss;
        for (const auto* s : b->getChildren("statements")) oss << generate(s) << "\n";
        return trimTrailingNewline(oss.str());
    }

    std::string visitListLiteral(const ListLiteral* l) override {
        std::ostringstream oss;
        oss << "{";
        auto elems = l->getChildren("elements");
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elems[i]);
        }
        oss << "}";
        return oss.str();
    }

    std::string visitIndexAccess(const IndexAccess* a) override {
        auto* t = a->getChild("target");
        auto* i = a->getChild("index");
        return (t ? generate(t) : "") + "(" + (i ? generate(i) : "") + ")";
    }

    std::string visitMemberAccess(const MemberAccess* a) override {
        auto* t = a->getChild("target");
        return (t ? generate(t) : "") + "." + a->memberName;
    }

    std::string visitPrimitiveType(const PrimitiveType* t) override {
        if (t->kind == "int") return "Integer";
        if (t->kind == "float" || t->kind == "double") return "Double";
        if (t->kind == "string") return "String";
        if (t->kind == "bool") return "Boolean";
        if (t->kind == "void") return "Object";
        return t->kind;
    }
    std::string visitListType(const ListType* t) override {
        auto* e = t->getChild("elementType");
        return "List(Of " + std::string(e ? generate(e) : "Object") + ")";
    }
    std::string visitSetType(const SetType* t) override {
        auto* e = t->getChild("elementType");
        return "HashSet(Of " + std::string(e ? generate(e) : "Object") + ")";
    }
    std::string visitMapType(const MapType* t) override {
        auto* k = t->getChild("keyType");
        auto* v = t->getChild("valueType");
        return "Dictionary(Of " + std::string(k ? generate(k) : "Object") + ", " +
               std::string(v ? generate(v) : "Object") + ")";
    }
    std::string visitTupleType(const TupleType* t) override {
        std::ostringstream oss;
        oss << "Tuple(Of ";
        auto elems = t->getChildren("elementTypes");
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elems[i]);
        }
        oss << ")";
        return oss.str();
    }
    std::string visitArrayType(const ArrayType* t) override {
        auto* e = t->getChild("elementType");
        return (e ? generate(e) : "Object") + "()";
    }
    std::string visitOptionalType(const OptionalType* t) override {
        auto* inner = t->getChild("innerType");
        return "Nullable(Of " + std::string(inner ? generate(inner) : "Object") + ")";
    }
    std::string visitCustomType(const CustomType* t) override { return t->typeName; }

    // Subject 1 annotations not covered by SemannoAnnotationImpl.
    std::string visitDerefStrategy(const DerefStrategy* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitOptimizationLock(const OptimizationLock* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitLangSpecific(const LangSpecific* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitDeallocateAnnotation(const DeallocateAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitLifetimeAnnotation(const LifetimeAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitReclaimAnnotation(const ReclaimAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitOwnerAnnotation(const OwnerAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitAllocateAnnotation(const AllocateAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitHotColdAnnotation(const HotColdAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitInlineAnnotation(const InlineAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitPureAnnotation(const PureAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }
    std::string visitConstExprAnnotation(const ConstExprAnnotation* a) override { return commentPrefix() + SemannoEmitter::emit(a); }

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        oss << "Class " << cls->name << "\n";
        for (const auto* f : cls->getChildren("fields")) {
            oss << "    " << generate(f) << "\n";
        }
        oss << "End Class";
        return oss.str();
    }

    std::string visitInterfaceDeclaration(const ASTNode* node) override {
        auto* iface = static_cast<const InterfaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "Interface " << iface->name << "\nEnd Interface";
        return oss.str();
    }

    std::string visitNamespaceDeclaration(const ASTNode* node) override {
        auto* ns = static_cast<const NamespaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "Module " << ns->name << "\nEnd Module";
        return oss.str();
    }

private:
    static std::string trimTrailingNewline(std::string s) {
        while (!s.empty() && (s.back() == '\n' || s.back() == '\r')) s.pop_back();
        return s;
    }
};
