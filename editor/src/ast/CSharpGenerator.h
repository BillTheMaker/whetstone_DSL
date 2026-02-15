#pragma once
#include "ProjectionGenerator.h"
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "../SemannoAnnotationImpl.h"

class CSharpGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<CSharpGenerator> {
public:
    std::string commentPrefix() const { return "// "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "// Module: " << module->name << "\n\n";
        auto variables = module->getChildren("variables");
        for (const auto* var : variables) oss << generate(var) << "\n";
        if (!variables.empty()) oss << "\n";
        auto functions = module->getChildren("functions");
        for (size_t i = 0; i < functions.size(); ++i) {
            if (i > 0) oss << "\n";
            oss << generate(functions[i]);
        }
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        auto annotations = function->getChildren("annotations");
        for (const auto* anno : annotations) oss << generate(anno) << "\n";
        oss << "public void " << function->name << "(";
        auto params = function->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")\n{\n";
        auto body = function->getChildren("body");
        for (const auto* stmt : body)
            oss << "    " << generate(stmt) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitVariable(const Variable* v) override {
        std::ostringstream oss;
        auto type = v->getChild("type");
        oss << (type ? generate(type) : "var") << " " << v->name;
        auto init = v->getChild("initializer");
        if (init) oss << " = " << generate(init);
        oss << ";";
        return oss.str();
    }

    std::string visitParameter(const Parameter* p) override {
        auto type = p->getChild("type");
        return (type ? generate(type) : "object") + " " + p->name;
    }

    std::string visitAssignment(const Assignment* a) override {
        auto t = a->getChild("target"); auto v = a->getChild("value");
        return (t ? generate(t) : "") + " = " + (v ? generate(v) : "null") + ";";
    }
    std::string visitReturn(const Return* r) override {
        auto v = r->getChild("value");
        return v ? "return " + generate(v) + ";" : "return;";
    }
    std::string visitBinaryOperation(const BinaryOperation* b) override {
        auto l = b->getChild("left"); auto r = b->getChild("right");
        return (l ? generate(l) : "") + " " + b->op + " " + (r ? generate(r) : "");
    }
    std::string visitVariableReference(const VariableReference* v) override { return v->variableName; }
    std::string visitIntegerLiteral(const IntegerLiteral* l) override { return std::to_string(l->value); }
    std::string visitFloatLiteral(const FloatLiteral* l) override { return l->value; }
    std::string visitStringLiteral(const StringLiteral* l) override { return "\"" + l->value + "\""; }
    std::string visitBooleanLiteral(const BooleanLiteral* l) override { return l->value ? "true" : "false"; }
    std::string visitNullLiteral(const NullLiteral*) override { return "null"; }
    std::string visitIfStatement(const IfStatement* s) override {
        std::ostringstream oss;
        auto c = s->getChild("condition");
        oss << "if (" << (c ? generate(c) : "true") << ")\n{\n";
        for (const auto* st : s->getChildren("thenBranch"))
            oss << "    " << generate(st) << "\n";
        oss << "}";
        auto el = s->getChildren("elseBranch");
        if (!el.empty()) {
            oss << "\nelse\n{\n";
            for (const auto* st : el) oss << "    " << generate(st) << "\n";
            oss << "}";
        }
        oss << "\n";
        return oss.str();
    }
    std::string visitWhileLoop(const WhileLoop* l) override {
        std::ostringstream oss;
        auto c = l->getChild("condition");
        oss << "while (" << (c ? generate(c) : "true") << ")\n{\n";
        for (const auto* s : l->getChildren("body")) oss << "    " << generate(s) << "\n";
        oss << "}\n";
        return oss.str();
    }
    std::string visitForLoop(const ForLoop* l) override {
        std::ostringstream oss;
        auto iter = l->getChild("iterable");
        oss << "foreach (var " << (l->iteratorName.empty() ? "item" : l->iteratorName);
        oss << " in " << (iter ? generate(iter) : "Array.Empty<object>()") << ")\n{\n";
        for (const auto* s : l->getChildren("body")) oss << "    " << generate(s) << "\n";
        oss << "}\n";
        return oss.str();
    }
    std::string visitExpressionStatement(const ExpressionStatement* s) override {
        auto e = s->getChild("expression");
        return e ? generate(e) + ";" : "";
    }
    std::string visitUnaryOperation(const UnaryOperation* u) override {
        auto o = u->getChild("operand");
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
        return oss.str();
    }
    std::string visitListLiteral(const ListLiteral* l) override {
        std::ostringstream oss;
        oss << "new List<object> { ";
        auto e = l->getChildren("elements");
        for (size_t i = 0; i < e.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(e[i]);
        }
        oss << " }";
        return oss.str();
    }
    std::string visitIndexAccess(const IndexAccess* a) override {
        auto t = a->getChild("target"); auto i = a->getChild("index");
        return (t ? generate(t) : "") + "[" + (i ? generate(i) : "") + "]";
    }
    std::string visitMemberAccess(const MemberAccess* a) override {
        auto t = a->getChild("target");
        return (t ? generate(t) : "") + "." + a->memberName;
    }
    std::string visitPrimitiveType(const PrimitiveType* t) override {
        if (t->kind == "int") return "int";
        if (t->kind == "float") return "float";
        if (t->kind == "double") return "double";
        if (t->kind == "string") return "string";
        if (t->kind == "bool") return "bool";
        if (t->kind == "void") return "void";
        return t->kind;
    }
    std::string visitListType(const ListType* t) override {
        auto e = t->getChild("elementType");
        return "List<" + (e ? generate(e) : "object") + ">";
    }
    std::string visitSetType(const SetType* t) override {
        auto e = t->getChild("elementType");
        return "HashSet<" + (e ? generate(e) : "object") + ">";
    }
    std::string visitMapType(const MapType* t) override {
        auto k = t->getChild("keyType"); auto v = t->getChild("valueType");
        return "Dictionary<" + (k ? generate(k) : "object") + ", " + (v ? generate(v) : "object") + ">";
    }
    std::string visitTupleType(const TupleType* t) override {
        std::ostringstream oss;
        oss << "(";
        auto types = t->getChildren("elementTypes");
        for (size_t i = 0; i < types.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(types[i]);
        }
        oss << ")";
        return oss.str();
    }
    std::string visitArrayType(const ArrayType* t) override {
        auto e = t->getChild("elementType");
        return (e ? generate(e) : "object") + "[]";
    }
    std::string visitOptionalType(const OptionalType* t) override {
        auto i = t->getChild("innerType");
        return (i ? generate(i) : "object") + "?";
    }
    std::string visitCustomType(const CustomType* t) override { return t->typeName; }

    // --- New AST node visitors (Step 312) ---

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = cls->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        if (cls->isAbstract) oss << "abstract ";
        oss << "class " << cls->name;
        if (!cls->superClass.empty()) oss << " : " << cls->superClass;
        oss << "\n{\n";
        auto fields = cls->getChildren("fields");
        for (const auto* f : fields) oss << "    " << generate(f) << "\n";
        if (!fields.empty() && !cls->getChildren("methods").empty()) oss << "\n";
        auto methods = cls->getChildren("methods");
        for (const auto* m : methods) oss << generate(m) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitInterfaceDeclaration(const ASTNode* node) override {
        auto* iface = static_cast<const InterfaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "interface " << iface->name << "\n{\n";
        auto methods = iface->getChildren("methods");
        for (const auto* m : methods) {
            auto* meth = static_cast<const MethodDeclaration*>(m);
            oss << "    void " << meth->name << "();\n";
        }
        oss << "}\n";
        return oss.str();
    }

    std::string visitMethodDeclaration(const ASTNode* node) override {
        auto* meth = static_cast<const MethodDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = meth->getChildren("annotations");
        for (const auto* a : annotations) oss << "    " << generate(a) << "\n";
        oss << "    ";
        if (!meth->visibility.empty()) oss << meth->visibility << " ";
        if (meth->isStatic) oss << "static ";
        if (meth->isVirtual) oss << "virtual ";
        if (meth->isOverride) oss << "override ";
        std::string returnType = "void";
        auto retType = meth->getChild("returnType");
        if (retType) returnType = generate(retType);
        oss << returnType << " " << meth->name << "(";
        auto params = meth->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")\n";
        auto body = meth->getChildren("body");
        if (body.empty()) {
            oss << "    {\n    }\n";
        } else {
            oss << "    {\n";
            for (const auto* s : body) oss << "        " << generate(s) << "\n";
            oss << "    }\n";
        }
        return oss.str();
    }

    std::string visitGenericType(const ASTNode* node) override {
        auto* gen = static_cast<const GenericType*>(node);
        std::ostringstream oss;
        oss << gen->baseName << "<";
        auto params = gen->getChildren("typeParameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ">";
        return oss.str();
    }

    std::string visitTypeParameter(const ASTNode* node) override {
        auto* tp = static_cast<const TypeParameter*>(node);
        return tp->name;
    }

    std::string visitAsyncFunction(const ASTNode* node) override {
        auto* af = static_cast<const AsyncFunction*>(node);
        std::ostringstream oss;
        auto annotations = af->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        oss << "public async Task " << af->name << "(";
        auto params = af->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")\n{\n";
        auto body = af->getChildren("body");
        for (const auto* s : body) oss << "    " << generate(s) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitAwaitExpression(const ASTNode* node) override {
        auto* aw = static_cast<const AwaitExpression*>(node);
        auto* expr = aw->getChild("expression");
        return "await " + (expr ? generate(expr) : "Task.CompletedTask");
    }

    std::string visitLambdaExpression(const ASTNode* node) override {
        auto* lam = static_cast<const LambdaExpression*>(node);
        std::ostringstream oss;
        oss << "(";
        auto params = lam->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << static_cast<const Parameter*>(params[i])->name;
        }
        oss << ") => ";
        auto body = lam->getChildren("body");
        if (body.size() == 1) {
            oss << generate(body[0]);
        } else if (body.empty()) {
            oss << "{ }";
        } else {
            oss << "{ ";
            for (const auto* s : body) oss << generate(s) << "; ";
            oss << "}";
        }
        return oss.str();
    }

    std::string visitDecoratorAnnotation(const ASTNode* node) override {
        auto* dec = static_cast<const DecoratorAnnotation*>(node);
        std::ostringstream oss;
        oss << "[" << dec->name;
        auto args = dec->getChildren("arguments");
        if (!args.empty()) {
            oss << "(";
            for (size_t i = 0; i < args.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << generate(args[i]);
            }
            oss << ")";
        }
        oss << "]";
        return oss.str();
    }

    std::string visitDerefStrategy(const DerefStrategy* a) override { return "// @deref(" + a->strategy + ")"; }
    std::string visitOptimizationLock(const OptimizationLock* a) override { return "// @lock(" + a->lockedBy + ")"; }
    std::string visitLangSpecific(const LangSpecific* a) override { return "// @lang_specific(" + a->language + ")"; }
    std::string visitDeallocateAnnotation(const DeallocateAnnotation* a) override { return semanno(a); }
    std::string visitLifetimeAnnotation(const LifetimeAnnotation* a) override { return semanno(a); }
    std::string visitReclaimAnnotation(const ReclaimAnnotation* a) override { return semanno(a); }
    std::string visitOwnerAnnotation(const OwnerAnnotation* a) override { return semanno(a); }
    std::string visitAllocateAnnotation(const AllocateAnnotation* a) override { return semanno(a); }
    std::string visitHotColdAnnotation(const HotColdAnnotation* a) override { return semanno(a); }
    std::string visitInlineAnnotation(const InlineAnnotation* a) override { return semanno(a); }
    std::string visitPureAnnotation(const PureAnnotation* a) override { return semanno(a); }
    std::string visitConstExprAnnotation(const ConstExprAnnotation* a) override { return semanno(a); }
};
