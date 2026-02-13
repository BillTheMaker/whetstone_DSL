#pragma once
#include "ProjectionGenerator.h"
#include "Import.h"
#include "../SemannoAnnotationImpl.h"

class GoGenerator : public ProjectionGenerator, public SemannoAnnotationImpl<GoGenerator> {
public:
    std::string commentPrefix() const { return "// "; }

    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;

        std::string pkgName = module->name.empty() ? "main" : module->name;
        oss << "package " << pkgName << "\n\n";

        auto imports = module->getChildren("imports");
        if (!imports.empty()) {
            oss << "import (\n";
            for (const auto* impNode : imports) {
                if (impNode->conceptType != "Import") continue;
                oss << "    " << emitImport(static_cast<const Import*>(impNode)) << "\n";
            }
            oss << ")\n\n";
        }

        auto variables = module->getChildren("variables");
        for (const auto* var : variables) {
            oss << visitVariable(static_cast<const Variable*>(var)) << "\n";
        }
        if (!variables.empty()) oss << "\n";

        auto functions = module->getChildren("functions");
        for (size_t i = 0; i < functions.size(); ++i) {
            if (i > 0) oss << "\n";
            oss << visitFunction(static_cast<const Function*>(functions[i]));
        }
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;
        emitAnnotations(oss, function->getChildren("annotations"), "");

        std::string name = function->name;
        std::string receiver;
        auto pos = name.find('.');
        if (pos != std::string::npos && pos > 0 && pos + 1 < name.size()) {
            receiver = name.substr(0, pos);
            name = name.substr(pos + 1);
        }

        oss << "func ";
        if (!receiver.empty()) {
            oss << "(" << receiverVar(receiver) << " *" << receiver << ") ";
        }
        oss << name << "(";
        emitParameters(oss, function->getChildren("parameters"));
        oss << ")";
        auto retType = function->getChild("returnType");
        if (retType) {
            oss << " " << generate(retType);
        }
        oss << " {\n";
        emitBody(oss, function->getChildren("body"), "    ");
        oss << "}\n";
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        emitAnnotations(oss, variable->getChildren("annotations"), "");
        std::string typeStr;
        auto type = variable->getChild("type");
        if (type) typeStr = generate(type);
        oss << "var " << variable->name;
        if (!typeStr.empty()) oss << " " << typeStr;
        auto initializer = variable->getChild("initializer");
        if (initializer) {
            oss << " = " << generate(initializer);
        }
        oss << "";
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        std::ostringstream oss;
        std::string typeStr;
        auto type = parameter->getChild("type");
        if (type) typeStr = generate(type);
        oss << parameter->name;
        if (!typeStr.empty()) {
            oss << " " << typeStr;
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
            oss << "nil";
        }
        return oss.str();
    }

    std::string visitReturn(const Return* ret) override {
        std::ostringstream oss;
        oss << "return";
        auto value = ret->getChild("value");
        if (value) {
            oss << " " << generate(value);
        }
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
        return "nil";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;
        auto condition = stmt->getChild("condition");
        auto thenBranch = stmt->getChildren("thenBranch");
        auto elseBranch = stmt->getChildren("elseBranch");

        oss << "if ";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "true";
        }
        oss << " {\n";
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
        oss << "for ";
        if (condition) {
            oss << generate(condition);
        }
        oss << " {\n";
        emitBody(oss, body, "    ");
        oss << "}";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;
        auto iterable = loop->getChild("iterable");
        auto body = loop->getChildren("body");
        std::string iterName = loop->iteratorName.empty() ? "item" : loop->iteratorName;
        if (iterable) {
            oss << "for " << iterName << " := range " << generate(iterable) << " {\n";
        } else {
            oss << "for {\n";
        }
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
            oss << "nil";
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
        oss << "[]interface{}{";
        auto elements = lit->getChildren("elements");
        for (size_t i = 0; i < elements.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elements[i]);
        }
        oss << "}";
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
        if (kind == "int" || kind == "long" || kind == "short") return "int";
        if (kind == "byte") return "byte";
        if (kind == "float" || kind == "double") return "float64";
        if (kind == "string" || kind == "char") return "string";
        if (kind == "bool") return "bool";
        if (kind == "void") return "";
        return kind;
    }

    std::string visitListType(const ListType* type) override {
        std::ostringstream oss;
        oss << "[]";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "interface{}";
        }
        return oss.str();
    }

    std::string visitSetType(const SetType* type) override {
        std::ostringstream oss;
        oss << "map[";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "interface{}";
        }
        oss << "]struct{}";
        return oss.str();
    }

    std::string visitMapType(const MapType* type) override {
        std::ostringstream oss;
        oss << "map[";
        auto keyType = type->getChild("keyType");
        auto valueType = type->getChild("valueType");
        if (keyType) {
            oss << generate(keyType);
        } else {
            oss << "interface{}";
        }
        oss << "]";
        if (valueType) {
            oss << generate(valueType);
        } else {
            oss << "interface{}";
        }
        return oss.str();
    }

    std::string visitTupleType(const TupleType* type) override {
        std::ostringstream oss;
        oss << "[]interface{}{ ";
        auto elementTypes = type->getChildren("elementTypes");
        for (size_t i = 0; i < elementTypes.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << generate(elementTypes[i]);
        }
        oss << " }";
        return oss.str();
    }

    std::string visitArrayType(const ArrayType* type) override {
        std::ostringstream oss;
        oss << "[]";
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "interface{}";
        }
        return oss.str();
    }

    std::string visitOptionalType(const OptionalType* type) override {
        std::ostringstream oss;
        oss << "*";
        auto inner = type->getChild("innerType");
        if (inner) {
            oss << generate(inner);
        } else {
            oss << "interface{}";
        }
        return oss.str();
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
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
        if (annotation->strategy == "Explicit") {
            return "// @dealloc(Explicit) - Close/Release";
        }
        return "// @dealloc(" + annotation->strategy + ")";
    }

    std::string visitLifetimeAnnotation(const LifetimeAnnotation* annotation) override {
        return "// @lifetime(" + annotation->strategy + ")";
    }

    std::string visitReclaimAnnotation(const ReclaimAnnotation* annotation) override {
        if (annotation->strategy == "Escape") {
            return "// @reclaim(Escape) - escape analysis";
        }
        return "// @reclaim(" + annotation->strategy + ")";
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

private:
    static std::string emitImport(const Import* imp) {
        std::string moduleName = imp->moduleName.empty() ? "" : imp->moduleName;
        if (!moduleName.empty() && (moduleName.front() != '"' || moduleName.back() != '"')) {
            moduleName = "\"" + moduleName + "\"";
        }
        return moduleName.empty() ? "\"\"" : moduleName;
    }

    static void emitAnnotations(std::ostringstream& oss,
                                const std::vector<ASTNode*>& annotations,
                                const std::string& indent) {
        for (const auto* annotation : annotations) {
            std::string code;
            if (annotation) {
                GoGenerator gen;
                code = gen.generate(annotation);
            }
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

    static std::string receiverVar(const std::string& receiver) {
        if (receiver.empty()) return "r";
        char c = receiver[0];
        if (std::isalpha(static_cast<unsigned char>(c))) {
            return std::string(1, (char)std::tolower(c));
        }
        return "r";
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
};
