#pragma once
#include "ProjectionGenerator.h"
#include "Import.h"
#include "ClassDeclaration.h"
#include "GenericType.h"
#include "AsyncNodes.h"
#include "../SemannoAnnotationImpl.h"
#include <unordered_map>
#include <unordered_set>

class JavaScriptGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<JavaScriptGenerator> {
public:
    std::string commentPrefix() const { return "// "; }

    explicit JavaScriptGenerator(bool includeTypes = false)
        : includeTypes_(includeTypes) {}

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;

        bool needsFinalizer = hasReclaimAnnotation(module);
        if (needsFinalizer) {
            oss << "// Whetstone: tracing reclamation uses WeakRef/FinalizationRegistry\n";
            oss << "const __whetstoneFinalizer = new FinalizationRegistry(() => {});\n\n";
        }

        auto imports = module->getChildren("imports");
        for (const auto* impNode : imports) {
            if (impNode->conceptType != "Import") continue;
            oss << emitImport(static_cast<const Import*>(impNode)) << "\n";
        }
        if (!imports.empty()) {
            oss << "\n";
        }

        // Partition functions into top-level and class methods.
        std::unordered_map<std::string, std::vector<const Function*>> classes;
        std::vector<const Function*> topLevel;
        auto functions = module->getChildren("functions");
        for (const auto* fnNode : functions) {
            if (fnNode->conceptType != "Function") continue;
            const Function* fn = static_cast<const Function*>(fnNode);
            auto pos = fn->name.find('.');
            if (pos != std::string::npos && pos > 0 && pos + 1 < fn->name.size()) {
                std::string className = fn->name.substr(0, pos);
                classes[className].push_back(fn);
            } else {
                topLevel.push_back(fn);
            }
        }

        // Variables
        auto variables = module->getChildren("variables");
        for (const auto* var : variables) {
            oss << visitVariable(static_cast<const Variable*>(var)) << "\n";
        }
        if (!variables.empty()) {
            oss << "\n";
        }

        // Top-level functions
        for (size_t i = 0; i < topLevel.size(); ++i) {
            if (i > 0) oss << "\n";
            oss << visitFunction(topLevel[i]);
        }

        if (!topLevel.empty() && !classes.empty()) {
            oss << "\n";
        }

        // Classes
        for (const auto& [className, methods] : classes) {
            oss << "class " << className << " {\n";
            for (size_t i = 0; i < methods.size(); ++i) {
                std::string methodName = methodNameFromQualified(methods[i]->name, className);
                emitMethod(oss, methods[i], methodName, "    ");
                if (i + 1 < methods.size()) oss << "\n";
            }
            oss << "}\n";
        }

        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        emitAnnotations(oss, function->getChildren("annotations"), "");
        oss << "function " << function->name << "(";
        emitParameters(oss, function->getChildren("parameters"));
        oss << ")";
        if (includeTypes_) {
            auto retType = function->getChild("returnType");
            if (retType) {
                oss << ": " << generate(retType);
            }
        }
        oss << " {\n";
        emitBody(oss, function->getChildren("body"), "    ");
        oss << "}\n";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << "let " << variable->name;
        if (includeTypes_) {
            auto type = variable->getChild("type");
            if (type) oss << ": " << generate(type);
        }
        auto initializer = variable->getChild("initializer");
        if (initializer) {
            oss << " = " << generate(initializer);
        }
        oss << ";";
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        std::ostringstream oss;
        oss << parameter->name;
        if (includeTypes_) {
            auto type = parameter->getChild("type");
            if (type) {
                oss << ": " << generate(type);
            }
        }
        auto defaultValue = parameter->getChild("defaultValue");
        if (defaultValue) {
            oss << " = " << generate(defaultValue);
        }
        return oss.str();
    }

    std::string visitAssignment(const Assignment* assignment) override {
        std::ostringstream oss;
        auto target = assignment->getChild("target");
        auto value = assignment->getChild("value");
        if (target) {
            oss << generate(target);
        } else {
            oss << "/* missing target */";
        }
        oss << " = ";
        if (value) {
            oss << generate(value);
        } else {
            oss << "undefined";
        }
        oss << ";";
        return oss.str();
    }

    std::string visitReturn(const Return* ret) override {
        std::ostringstream oss;
        oss << "return";
        auto value = ret->getChild("value");
        if (value) {
            oss << " " << generate(value);
        }
        oss << ";";
        return oss.str();
    }

    std::string visitBinaryOperation(const BinaryOperation* binOp) override {
        std::ostringstream oss;
        auto left = binOp->getChild("left");
        auto right = binOp->getChild("right");
        if (left) {
            oss << generate(left);
        } else {
            oss << "/* missing left */";
        }
        oss << " " << binOp->op << " ";
        if (right) {
            oss << generate(right);
        } else {
            oss << "/* missing right */";
        }
        return oss.str();
    }

    std::string visitVariableReference(const VariableReference* varRef) override {
        return varRef->variableName;
    }

    std::string visitIntegerLiteral(const IntegerLiteral* lit) override {
        return std::to_string(lit->value);
    }

    std::string visitFloatLiteral(const FloatLiteral* lit) override {
        return lit->value;
    }

    std::string visitStringLiteral(const StringLiteral* lit) override {
        if (isQuotedLiteral(lit->value)) return lit->value;
        return "\"" + lit->value + "\"";
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "true" : "false";
    }

    std::string visitNullLiteral(const NullLiteral*) override {
        return "null";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;
        auto condition = stmt->getChild("condition");
        auto thenBranch = stmt->getChildren("thenBranch");
        auto elseBranch = stmt->getChildren("elseBranch");

        oss << "if (";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "true";
        }
        oss << ") {\n";
        emitBody(oss, thenBranch, "    ");
        oss << "}";
        if (!elseBranch.empty()) {
            oss << " else {\n";
            emitBody(oss, elseBranch, "    ");
            oss << "}";
        }
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* loop) override {
        std::ostringstream oss;
        auto condition = loop->getChild("condition");
        auto body = loop->getChildren("body");
        oss << "while (";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "true";
        }
        oss << ") {\n";
        emitBody(oss, body, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;
        auto iterable = loop->getChild("iterable");
        auto body = loop->getChildren("body");
        std::string iterName = loop->iteratorName.empty() ? "item" : loop->iteratorName;
        oss << "for (const " << iterName << " of ";
        if (iterable) {
            oss << generate(iterable);
        } else {
            oss << "[]";
        }
        oss << ") {\n";
        emitBody(oss, body, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        std::ostringstream oss;
        auto expr = stmt->getChild("expression");
        if (expr) {
            oss << generate(expr);
        }
        oss << ";";
        return oss.str();
    }

    std::string visitUnaryOperation(const UnaryOperation* unOp) override {
        std::ostringstream oss;
        std::string op = unOp->op;
        bool wordOp = !op.empty() && std::isalpha(static_cast<unsigned char>(op[0]));
        oss << op;
        if (wordOp) oss << " ";
        auto operand = unOp->getChild("operand");
        if (operand) {
            oss << generate(operand);
        } else {
            oss << "undefined";
        }
        return oss.str();
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        oss << call->functionName << "(";
        auto arguments = call->getChildren("arguments");
        for (size_t i = 0; i < arguments.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(arguments[i]);
        }
        oss << ")";
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::ostringstream oss;
        auto statements = block->getChildren("statements");
        oss << "{\n";
        emitBody(oss, statements, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitListLiteral(const ListLiteral* lit) override {
        std::ostringstream oss;
        oss << "[";
        auto elements = lit->getChildren("elements");
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elements[i]);
        }
        oss << "]";
        return oss.str();
    }

    std::string visitIndexAccess(const IndexAccess* access) override {
        std::ostringstream oss;
        auto target = access->getChild("target");
        auto index = access->getChild("index");
        if (target) {
            oss << generate(target);
        } else {
            oss << "/* missing target */";
        }
        oss << "[";
        if (index) {
            oss << generate(index);
        } else {
            oss << "0";
        }
        oss << "]";
        return oss.str();
    }

    std::string visitMemberAccess(const MemberAccess* access) override {
        std::ostringstream oss;
        auto target = access->getChild("target");
        if (target) {
            oss << generate(target);
        } else {
            oss << "/* missing target */";
        }
        oss << "." << access->memberName;
        return oss.str();
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        std::string kind = type->kind;
        if (kind == "int" || kind == "float" || kind == "double" ||
            kind == "long" || kind == "short" || kind == "byte") return "number";
        if (kind == "string" || kind == "char") return "string";
        if (kind == "bool") return "boolean";
        if (kind == "void") return "void";
        return kind;
    }

    std::string visitListType(const ListType* type) override {
        std::ostringstream oss;
        oss << "Array<";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "any";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitSetType(const SetType* type) override {
        std::ostringstream oss;
        oss << "Set<";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "any";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitMapType(const MapType* type) override {
        std::ostringstream oss;
        oss << "Record<";
        auto keyType = type->getChild("keyType");
        auto valueType = type->getChild("valueType");
        if (keyType) {
            oss << generate(keyType);
        } else {
            oss << "string";
        }
        oss << ", ";
        if (valueType) {
            oss << generate(valueType);
        } else {
            oss << "any";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitTupleType(const TupleType* type) override {
        std::ostringstream oss;
        oss << "[";
        auto elementTypes = type->getChildren("elementTypes");
        for (size_t i = 0; i < elementTypes.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elementTypes[i]);
        }
        oss << "]";
        return oss.str();
    }

    std::string visitArrayType(const ArrayType* type) override {
        std::ostringstream oss;
        oss << "Array<";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "any";
        }
        oss << ">";
        return oss.str();
    }

    std::string visitOptionalType(const OptionalType* type) override {
        std::ostringstream oss;
        auto inner = type->getChild("innerType");
        if (inner) {
            oss << generate(inner) << " | null";
        } else {
            oss << "any | null";
        }
        return oss.str();
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
    }

    // --- New AST node visitors (Step 308) ---

    std::string visitClassDeclaration(const ASTNode* node) override {
        auto* cls = static_cast<const ClassDeclaration*>(node);
        std::ostringstream oss;
        auto annotations = cls->getChildren("annotations");
        for (const auto* a : annotations) oss << generate(a) << "\n";
        oss << "class " << cls->name;
        if (!cls->superClass.empty()) oss << " extends " << cls->superClass;
        oss << " {\n";
        auto fields = cls->getChildren("fields");
        for (const auto* f : fields)
            oss << "    " << generate(f) << "\n";
        auto methods = cls->getChildren("methods");
        for (const auto* m : methods) oss << generate(m);
        oss << "}\n";
        return oss.str();
    }

    std::string visitInterfaceDeclaration(const ASTNode* node) override {
        auto* iface = static_cast<const InterfaceDeclaration*>(node);
        std::ostringstream oss;
        oss << "class " << iface->name << " {\n";
        auto methods = iface->getChildren("methods");
        for (const auto* m : methods) {
            auto* meth = static_cast<const MethodDeclaration*>(m);
            oss << "    " << meth->name << "() { throw new Error('not implemented'); }\n";
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
        if (meth->isStatic) oss << "static ";
        oss << meth->name << "(";
        emitParameters(oss, meth->getChildren("parameters"));
        oss << ")";
        if (includeTypes_) {
            auto retType = meth->getChild("returnType");
            if (retType) oss << ": " << generate(retType);
        }
        auto body = meth->getChildren("body");
        if (body.empty()) {
            oss << " {}\n";
        } else {
            oss << " {\n";
            emitBody(oss, body, "        ");
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
        oss << "async function " << af->name << "(";
        emitParameters(oss, af->getChildren("parameters"));
        oss << ")";
        if (includeTypes_) {
            auto retType = af->getChild("returnType");
            if (retType) oss << ": Promise<" << generate(retType) << ">";
        }
        oss << " {\n";
        emitBody(oss, af->getChildren("body"), "    ");
        oss << "}\n";
        return oss.str();
    }

    std::string visitAwaitExpression(const ASTNode* node) override {
        auto* aw = static_cast<const AwaitExpression*>(node);
        auto* expr = aw->getChild("expression");
        return "await " + (expr ? generate(expr) : "undefined");
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
            oss << "{}";
        } else {
            oss << "{ ";
            for (const auto* s : body) oss << generate(s) << "; ";
            oss << "}";
        }
        return oss.str();
    }

    std::string visitDecoratorAnnotation(const ASTNode* node) override {
        auto* dec = static_cast<const DecoratorAnnotation*>(node);
        return "// @" + dec->name;
    }

    std::string visitDerefStrategy(const DerefStrategy* annotation) override {
        return "// @deref(" + annotation->strategy + ")";
    }

    std::string visitOptimizationLock(const OptimizationLock* annotation) override {
        return "// @lock(" + annotation->lockedBy + ") - " + annotation->lockReason;
    }

    std::string visitLangSpecific(const LangSpecific* annotation) override {
        return "// @lang_specific(" + annotation->language + ", " + annotation->idiomType + ")";
    }

    std::string visitDeallocateAnnotation(const DeallocateAnnotation* annotation) override {
        return "// @dealloc(" + annotation->strategy + ")";
    }

    std::string visitLifetimeAnnotation(const LifetimeAnnotation* annotation) override {
        return "// @lifetime(" + annotation->strategy + ")";
    }

    std::string visitReclaimAnnotation(const ReclaimAnnotation* annotation) override {
        return "// @reclaim(" + annotation->strategy + ") -> WeakRef/FinalizationRegistry";
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        return "// @owner(" + annotation->strategy + ")";
    }

    std::string visitAllocateAnnotation(const AllocateAnnotation* annotation) override {
        return "// @allocate(" + annotation->strategy + ")";
    }

    std::string visitHotColdAnnotation(const HotColdAnnotation* annotation) override {
        return "// @" + annotation->hint;
    }

    std::string visitInlineAnnotation(const InlineAnnotation* annotation) override {
        return "// @inline(" + annotation->mode + ")";
    }

    std::string visitPureAnnotation(const PureAnnotation*) override {
        return "// @pure";
    }

    std::string visitConstExprAnnotation(const ConstExprAnnotation*) override {
        return "// @constexpr";
    }

protected:
    bool includeTypes_ = false;

private:
    void emitAnnotations(std::ostringstream& oss,
                         const std::vector<ASTNode*>& annotations,
                         const std::string& indent) {
        for (const auto* annotation : annotations) {
            std::string code = generate(annotation);
            if (!code.empty()) {
                oss << indent << code << "\n";
            }
        }
    }

    void emitParameters(std::ostringstream& oss,
                        const std::vector<ASTNode*>& params) {
        for (size_t i = 0; i < params.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << visitParameter(static_cast<const Parameter*>(params[i]));
        }
    }

    void emitBody(std::ostringstream& oss,
                  const std::vector<ASTNode*>& body,
                  const std::string& indent) {
        if (body.empty()) {
            oss << indent << "// TODO: implement\n";
            return;
        }
        for (const auto* stmt : body) {
            std::string stmtCode = generate(stmt);
            appendIndented(oss, stmtCode, indent);
            oss << "\n";
        }
    }

    void emitMethod(std::ostringstream& oss,
                    const Function* function,
                    const std::string& methodName,
                    const std::string& indent) {
        emitAnnotations(oss, function->getChildren("annotations"), indent);
        oss << indent << methodName << "(";
        emitParameters(oss, function->getChildren("parameters"));
        oss << ")";
        if (includeTypes_) {
            auto retType = function->getChild("returnType");
            if (retType) {
                oss << ": " << generate(retType);
            }
        }
        oss << " {\n";
        emitBody(oss, function->getChildren("body"), indent + "    ");
        oss << indent << "}\n";
    }

    static void appendIndented(std::ostringstream& oss,
                               const std::string& code,
                               const std::string& indent) {
        size_t pos = 0;
        while (pos < code.length()) {
            size_t newlinePos = code.find('\n', pos);
            if (newlinePos == std::string::npos) {
                oss << indent << code.substr(pos);
                break;
            }
            oss << indent << code.substr(pos, newlinePos - pos) << "\n";
            pos = newlinePos + 1;
            if (pos >= code.length()) break;
        }
    }

    bool hasReclaimAnnotation(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "ReclaimAnnotation") return true;
        for (auto* child : node->allChildren()) {
            if (hasReclaimAnnotation(child)) return true;
        }
        return false;
    }

    static bool isQuotedLiteral(const std::string& text) {
        if (text.size() < 2) return false;
        char first = text.front();
        char last = text.back();
        if ((first == '\'' && last == '\'') || (first == '"' && last == '"') ||
            (first == '`' && last == '`')) {
            return true;
        }
        return false;
    }

    static std::string methodNameFromQualified(const std::string& name,
                                               const std::string& className) {
        if (name.rfind(className + ".", 0) == 0) {
            return name.substr(className.size() + 1);
        }
        auto pos = name.find('.');
        if (pos != std::string::npos && pos + 1 < name.size()) {
            return name.substr(pos + 1);
        }
        return name;
    }

    static std::string emitImport(const Import* imp) {
        std::string moduleName = imp->moduleName.empty() ? "module" : imp->moduleName;
        std::string alias = imp->alias.empty() ? moduleName : imp->alias;
        if (imp->importKind == "require") {
            return "const " + alias + " = require('" + moduleName + "');";
        }
        if (imp->importKind == "include") {
            return "// include '" + moduleName + "'";
        }
        if (imp->alias.empty()) {
            return "import * as " + moduleName + " from '" + moduleName + "';";
        }
        return "import * as " + alias + " from '" + moduleName + "';";
    }
};

class TypeScriptGenerator : public JavaScriptGenerator {
public:
    TypeScriptGenerator() : JavaScriptGenerator(true) {}
};
