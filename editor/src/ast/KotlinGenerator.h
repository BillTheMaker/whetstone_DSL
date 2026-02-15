#pragma once
#include "ProjectionGenerator.h"
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "../SemannoAnnotationImpl.h"

class KotlinGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<KotlinGenerator> {
public:
    std::string commentPrefix() const { return "// "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        oss << "// Module: " << module->name << "\n\n";
        auto variables = module->getChildren("variables");
        for (const auto* var : variables) {
            oss << generate(var) << "\n";
        }
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
        oss << "fun " << function->name << "(";
        auto params = function->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")";
        auto retType = function->getChild("returnType");
        if (retType) oss << ": " << generate(retType);
        oss << " {\n";
        auto body = function->getChildren("body");
        for (const auto* stmt : body) {
            std::string code = generate(stmt);
            oss << "    " << code << "\n";
        }
        oss << "}\n";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << "val " << variable->name;
        auto type = variable->getChild("type");
        if (type) oss << ": " << generate(type);
        auto init = variable->getChild("initializer");
        if (init) oss << " = " << generate(init);
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        std::ostringstream oss;
        oss << parameter->name;
        auto type = parameter->getChild("type");
        if (type) oss << ": " << generate(type);
        auto def = parameter->getChild("defaultValue");
        if (def) oss << " = " << generate(def);
        return oss.str();
    }

    std::string visitAssignment(const Assignment* a) override {
        std::ostringstream oss;
        auto target = a->getChild("target");
        auto value = a->getChild("value");
        if (target) oss << generate(target);
        oss << " = ";
        if (value) oss << generate(value);
        return oss.str();
    }

    std::string visitReturn(const Return* r) override {
        auto value = r->getChild("value");
        return value ? "return " + generate(value) : "return";
    }

    std::string visitBinaryOperation(const BinaryOperation* b) override {
        std::ostringstream oss;
        auto left = b->getChild("left");
        auto right = b->getChild("right");
        if (left) oss << generate(left);
        oss << " " << b->op << " ";
        if (right) oss << generate(right);
        return oss.str();
    }

    std::string visitVariableReference(const VariableReference* v) override { return v->variableName; }
    std::string visitIntegerLiteral(const IntegerLiteral* l) override { return std::to_string(l->value); }
    std::string visitFloatLiteral(const FloatLiteral* l) override { return l->value; }
    std::string visitStringLiteral(const StringLiteral* l) override { return "\"" + l->value + "\""; }
    std::string visitBooleanLiteral(const BooleanLiteral* l) override { return l->value ? "true" : "false"; }
    std::string visitNullLiteral(const NullLiteral*) override { return "null"; }

    std::string visitIfStatement(const IfStatement* s) override {
        std::ostringstream oss;
        auto cond = s->getChild("condition");
        oss << "if (" << (cond ? generate(cond) : "true") << ") {\n";
        for (const auto* stmt : s->getChildren("thenBranch"))
            oss << "    " << generate(stmt) << "\n";
        oss << "}";
        auto elseBranch = s->getChildren("elseBranch");
        if (!elseBranch.empty()) {
            oss << " else {\n";
            for (const auto* stmt : elseBranch)
                oss << "    " << generate(stmt) << "\n";
            oss << "}";
        }
        oss << "\n";
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* l) override {
        std::ostringstream oss;
        auto cond = l->getChild("condition");
        oss << "while (" << (cond ? generate(cond) : "true") << ") {\n";
        for (const auto* stmt : l->getChildren("body"))
            oss << "    " << generate(stmt) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* l) override {
        std::ostringstream oss;
        auto iter = l->getChild("iterable");
        oss << "for (" << (l->iteratorName.empty() ? "item" : l->iteratorName);
        oss << " in " << (iter ? generate(iter) : "listOf()") << ") {\n";
        for (const auto* stmt : l->getChildren("body"))
            oss << "    " << generate(stmt) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* s) override {
        auto expr = s->getChild("expression");
        return expr ? generate(expr) : "";
    }
    std::string visitUnaryOperation(const UnaryOperation* u) override {
        auto op = u->getChild("operand");
        return u->op + (op ? generate(op) : "");
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
        for (const auto* s : b->getChildren("statements"))
            oss << generate(s) << "\n";
        return oss.str();
    }
    std::string visitListLiteral(const ListLiteral* l) override {
        std::ostringstream oss;
        oss << "listOf(";
        auto elems = l->getChildren("elements");
        for (size_t i = 0; i < elems.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elems[i]);
        }
        oss << ")";
        return oss.str();
    }
    std::string visitIndexAccess(const IndexAccess* a) override {
        auto t = a->getChild("target");
        auto i = a->getChild("index");
        return (t ? generate(t) : "") + "[" + (i ? generate(i) : "") + "]";
    }
    std::string visitMemberAccess(const MemberAccess* a) override {
        auto t = a->getChild("target");
        return (t ? generate(t) : "") + "." + a->memberName;
    }
    std::string visitPrimitiveType(const PrimitiveType* t) override {
        if (t->kind == "int") return "Int";
        if (t->kind == "float" || t->kind == "double") return "Double";
        if (t->kind == "string") return "String";
        if (t->kind == "bool") return "Boolean";
        if (t->kind == "void") return "Unit";
        return t->kind;
    }
    std::string visitListType(const ListType* t) override {
        auto el = t->getChild("elementType");
        return "List<" + (el ? generate(el) : "Any") + ">";
    }
    std::string visitSetType(const SetType* t) override {
        auto el = t->getChild("elementType");
        return "Set<" + (el ? generate(el) : "Any") + ">";
    }
    std::string visitMapType(const MapType* t) override {
        auto k = t->getChild("keyType");
        auto v = t->getChild("valueType");
        return "Map<" + (k ? generate(k) : "Any") + ", " + (v ? generate(v) : "Any") + ">";
    }
    std::string visitTupleType(const TupleType* t) override {
        std::ostringstream oss;
        oss << "Pair<";
        auto types = t->getChildren("elementTypes");
        for (size_t i = 0; i < types.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(types[i]);
        }
        oss << ">";
        return oss.str();
    }
    std::string visitArrayType(const ArrayType* t) override {
        auto el = t->getChild("elementType");
        return "Array<" + (el ? generate(el) : "Any") + ">";
    }
    std::string visitOptionalType(const OptionalType* t) override {
        auto inner = t->getChild("innerType");
        return (inner ? generate(inner) : "Any") + "?";
    }
    std::string visitCustomType(const CustomType* t) override { return t->typeName; }

    // --- New AST node visitors (Step 310) ---

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = cls->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        if (cls->isAbstract) oss << "abstract ";
        oss << "class " << cls->name;
        if (!cls->superClass.empty()) oss << " : " << cls->superClass << "()";
        oss << " {\n";
        auto fields = cls->getChildren("fields");
        for (const auto* f : fields) oss << "    " << generate(f) << "\n";
        auto methods = cls->getChildren("methods");
        for (const auto* m : methods) oss << generate(m);
        oss << "}\n";
        return oss.str();
    }

    std::string visitInterfaceDeclaration(const ASTNode* node) override {
        auto* iface = static_cast<const InterfaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "interface " << iface->name << " {\n";
        auto methods = iface->getChildren("methods");
        for (const auto* m : methods) {
            auto* meth = static_cast<const MethodDeclaration*>(m);
            oss << "    fun " << meth->name << "()\n";
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
        if (meth->isOverride) oss << "override ";
        oss << "fun " << meth->name << "(";
        auto params = meth->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")";
        auto retType = meth->getChild("returnType");
        if (retType) oss << ": " << generate(retType);
        auto body = meth->getChildren("body");
        if (body.empty()) {
            oss << " {}\n";
        } else {
            oss << " {\n";
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
        if (!tp->constraint.empty()) return tp->name + " : " + tp->constraint;
        return tp->name;
    }

    std::string visitAsyncFunction(const ASTNode* node) override {
        auto* af = static_cast<const AsyncFunction*>(node);
        std::ostringstream oss;
        auto annotations = af->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        oss << "suspend fun " << af->name << "(";
        auto params = af->getChildren("parameters");
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(params[i]);
        }
        oss << ")";
        auto retType = af->getChild("returnType");
        if (retType) oss << ": " << generate(retType);
        oss << " {\n";
        auto body = af->getChildren("body");
        for (const auto* s : body) oss << "    " << generate(s) << "\n";
        oss << "}\n";
        return oss.str();
    }

    std::string visitAwaitExpression(const ASTNode* node) override {
        auto* aw = static_cast<const AwaitExpression*>(node);
        auto* expr = aw->getChild("expression");
        return expr ? generate(expr) : "/* missing */";
    }

    std::string visitLambdaExpression(const ASTNode* node) override {
        auto* lam = static_cast<const LambdaExpression*>(node);
        std::ostringstream oss;
        oss << "{ ";
        auto params = lam->getChildren("parameters");
        if (!params.empty()) {
            for (size_t i = 0; i < params.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << static_cast<const Parameter*>(params[i])->name;
            }
            oss << " -> ";
        }
        auto body = lam->getChildren("body");
        for (size_t i = 0; i < body.size(); ++i) {
            if (i > 0) oss << "; ";
            oss << generate(body[i]);
        }
        oss << " }";
        return oss.str();
    }

    std::string visitDecoratorAnnotation(const ASTNode* node) override {
        auto* dec = static_cast<const DecoratorAnnotation*>(node);
        return "@" + dec->name;
    }

    // Legacy annotation visitors
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
