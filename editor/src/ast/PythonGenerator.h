#pragma once

#include "ProjectionGenerator.h"

class PythonGenerator : public ProjectionGenerator {
public:
    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "# Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;

        // Add module docstring and basic info
        oss << "\"\"\"Module: " << module->name << "\"\"\"\n\n";

        // Process variables first
        auto variables = module->getChildren("variables");
        for (const auto* var : variables) {
            oss << visitVariable(static_cast<const Variable*>(var)) << "\n";
        }

        if (!variables.empty()) {
            oss << "\n";
        }

        // Process functions
        auto functions = module->getChildren("functions");
        for (size_t i = 0; i < functions.size(); ++i) {
            if (i > 0) oss << "\n";  // Add blank line between functions
            oss << visitFunction(static_cast<const Function*>(functions[i]));
        }

        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;

        // Process annotations first (these are in the "annotations" role)
        auto annotations = function->getChildren("annotations");
        for (const auto* annotation : annotations) {
            oss << generate(annotation) << "\n";
        }

        // Generate function signature
        oss << "def " << function->name << "(";

        auto parameters = function->getChildren("parameters");
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << visitParameter(static_cast<const Parameter*>(parameters[i]));
        }

        oss << "):\n";

        // Process function body
        auto body = function->getChildren("body");
        if (body.empty()) {
            oss << "    pass\n";
        } else {
            for (const auto* stmt : body) {
                std::string stmtCode = generate(stmt);
                // Indent each line of the statement
                size_t pos = 0;
                while (pos < stmtCode.length()) {
                    size_t newlinePos = stmtCode.find('\n', pos);
                    if (newlinePos == std::string::npos) {
                        oss << "    " << stmtCode.substr(pos) << "\n";
                        break;
                    } else {
                        oss << "    " << stmtCode.substr(pos, newlinePos - pos) << "\n";
                        pos = newlinePos + 1;
                        if (pos >= stmtCode.length()) break;
                    }
                }
            }
        }

        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << variable->name << " = ";

        auto initializer = variable->getChild("initializer");
        if (initializer) {
            oss << generate(initializer);
        } else {
            oss << "None";
        }

        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        std::ostringstream oss;
        oss << parameter->name;

        // Add type annotation if present
        auto type = parameter->getChild("type");
        if (type) {
            oss << ": " << generate(type);
        }

        // Add default value if present
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
            oss << generate(target) << " = ";
        }

        if (value) {
            oss << generate(value);
        } else {
            oss << "None";
        }

        return oss.str();
    }

    std::string visitReturn(const Return* ret) override {
        std::ostringstream oss;
        oss << "return ";

        auto value = ret->getChild("value");
        if (value) {
            oss << generate(value);
        } else {
            oss << "None";
        }

        return oss.str();
    }

    std::string visitBinaryOperation(const BinaryOperation* binOp) override {
        std::ostringstream oss;

        auto left = binOp->getChild("left");
        auto right = binOp->getChild("right");

        if (left) {
            oss << generate(left);
        }

        oss << " " << binOp->op << " ";

        if (right) {
            oss << generate(right);
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
        return "\"" + lit->value + "\"";
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "True" : "False";
    }

    std::string visitNullLiteral(const NullLiteral* lit) override {
        return "None";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;

        auto condition = stmt->getChild("condition");
        auto thenBranch = stmt->getChildren("thenBranch");
        auto elseBranch = stmt->getChildren("elseBranch");

        if (condition) {
            oss << "if " << generate(condition) << ":\n";
        } else {
            oss << "if True:\n";  // fallback
        }

        // Then branch
        for (const auto* thenStmt : thenBranch) {
            std::string stmtCode = generate(thenStmt);
            size_t pos = 0;
            while (pos < stmtCode.length()) {
                size_t newlinePos = stmtCode.find('\n', pos);
                if (newlinePos == std::string::npos) {
                    oss << "    " << stmtCode.substr(pos) << "\n";
                    break;
                } else {
                    oss << "    " << stmtCode.substr(pos, newlinePos - pos) << "\n";
                    pos = newlinePos + 1;
                    if (pos >= stmtCode.length()) break;
                }
            }
        }

        // Else branch
        if (!elseBranch.empty()) {
            oss << "else:\n";
            for (const auto* elseStmt : elseBranch) {
                std::string stmtCode = generate(elseStmt);
                size_t pos = 0;
                while (pos < stmtCode.length()) {
                    size_t newlinePos = stmtCode.find('\n', pos);
                    if (newlinePos == std::string::npos) {
                        oss << "    " << stmtCode.substr(pos) << "\n";
                        break;
                    } else {
                        oss << "    " << stmtCode.substr(pos, newlinePos - pos) << "\n";
                        pos = newlinePos + 1;
                        if (pos >= stmtCode.length()) break;
                    }
                }
            }
        }

        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* loop) override {
        std::ostringstream oss;

        auto condition = loop->getChild("condition");
        auto body = loop->getChildren("body");

        if (condition) {
            oss << "while " << generate(condition) << ":\n";
        } else {
            oss << "while True:\n";  // fallback
        }

        for (const auto* stmt : body) {
            std::string stmtCode = generate(stmt);
            size_t pos = 0;
            while (pos < stmtCode.length()) {
                size_t newlinePos = stmtCode.find('\n', pos);
                if (newlinePos == std::string::npos) {
                    oss << "    " << stmtCode.substr(pos) << "\n";
                    break;
                } else {
                    oss << "    " << stmtCode.substr(pos, newlinePos - pos) << "\n";
                    pos = newlinePos + 1;
                    if (pos >= stmtCode.length()) break;
                }
            }
        }

        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;

        auto iterable = loop->getChild("iterable");
        auto body = loop->getChildren("body");

        if (loop->iteratorName.empty()) {
            oss << "for item";
        } else {
            oss << "for " << loop->iteratorName;
        }

        if (iterable) {
            oss << " in " << generate(iterable) << ":\n";
        } else {
            oss << " in []:\n";  // fallback
        }

        for (const auto* stmt : body) {
            std::string stmtCode = generate(stmt);
            size_t pos = 0;
            while (pos < stmtCode.length()) {
                size_t newlinePos = stmtCode.find('\n', pos);
                if (newlinePos == std::string::npos) {
                    oss << "    " << stmtCode.substr(pos) << "\n";
                    break;
                } else {
                    oss << "    " << stmtCode.substr(pos, newlinePos - pos) << "\n";
                    pos = newlinePos + 1;
                    if (pos >= stmtCode.length()) break;
                }
            }
        }

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
        oss << unOp->op << " ";

        auto operand = unOp->getChild("operand");
        if (operand) {
            oss << generate(operand);
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
        for (size_t i = 0; i < statements.size(); ++i) {
            if (i > 0) oss << "\n";
            oss << generate(statements[i]);
        }

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
        }

        oss << "[";
        if (index) {
            oss << generate(index);
        }
        oss << "]";

        return oss.str();
    }

    std::string visitMemberAccess(const MemberAccess* access) override {
        std::ostringstream oss;

        auto target = access->getChild("target");
        if (target) {
            oss << generate(target);
        }

        oss << "." << access->memberName;

        return oss.str();
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        std::string kind = type->kind;
        // Map common types to Python equivalents
        if (kind == "int") return "int";
        if (kind == "float") return "float";
        if (kind == "string") return "str";
        if (kind == "bool") return "bool";
        if (kind == "char") return "str";  // Python doesn't have char, use str
        if (kind == "double") return "float";
        if (kind == "long") return "int";
        if (kind == "short") return "int";
        if (kind == "byte") return "int";
        if (kind == "void") return "None";

        return kind;  // Return as-is if not a common type
    }

    std::string visitListType(const ListType* type) override {
        std::ostringstream oss;
        oss << "List[";

        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "Any";
        }

        oss << "]";
        return oss.str();
    }

    std::string visitSetType(const SetType* type) override {
        std::ostringstream oss;
        oss << "Set[";

        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "Any";
        }

        oss << "]";
        return oss.str();
    }

    std::string visitMapType(const MapType* type) override {
        std::ostringstream oss;
        oss << "Dict[";

        auto keyType = type->getChild("keyType");
        auto valueType = type->getChild("valueType");

        if (keyType) {
            oss << generate(keyType);
        } else {
            oss << "Any";
        }

        oss << ", ";

        if (valueType) {
            oss << generate(valueType);
        } else {
            oss << "Any";
        }

        oss << "]";
        return oss.str();
    }

    std::string visitTupleType(const TupleType* type) override {
        std::ostringstream oss;
        oss << "Tuple[";

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
        oss << "List[";

        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "Any";
        }

        oss << "]";
        return oss.str();
    }

    std::string visitOptionalType(const OptionalType* type) override {
        std::ostringstream oss;
        oss << "Optional[";

        auto innerType = type->getChild("innerType");
        if (innerType) {
            oss << generate(innerType);
        } else {
            oss << "Any";
        }

        oss << "]";
        return oss.str();
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
    }

    std::string visitDerefStrategy(const DerefStrategy* annotation) override {
        return "# @deref(" + annotation->strategy + ")";
    }

    std::string visitOptimizationLock(const OptimizationLock* annotation) override {
        return "# @lock(" + annotation->lockedBy + ")";
    }

    std::string visitLangSpecific(const LangSpecific* annotation) override {
        return "# @lang_specific(" + annotation->language + ", " + annotation->idiomType + ")";
    }

    std::string visitDeallocateAnnotation(const DeallocateAnnotation* annotation) override {
        return "# @dealloc(" + annotation->strategy + ") - Memory deallocation strategy";
    }

    std::string visitLifetimeAnnotation(const LifetimeAnnotation* annotation) override {
        return "# @lifetime(" + annotation->strategy + ") - Object lifetime management";
    }

    std::string visitReclaimAnnotation(const ReclaimAnnotation* annotation) override {
        return "# @reclaim(" + annotation->strategy + ") - Memory reclamation strategy";
    }

    std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) override {
        return "# @owner(" + annotation->strategy + ") - Ownership management strategy";
    }

    std::string visitAllocateAnnotation(const AllocateAnnotation* annotation) override {
        return "# @allocate(" + annotation->strategy + ") - Memory allocation strategy";
    }

    std::string visitHotColdAnnotation(const HotColdAnnotation* annotation) override {
        return "# @" + annotation->hint + " - Optimization hint";
    }

    std::string visitInlineAnnotation(const InlineAnnotation* annotation) override {
        return "# @inline(" + annotation->mode + ")";
    }

    std::string visitPureAnnotation(const PureAnnotation*) override {
        return "# @pure - No side effects";
    }

    std::string visitConstExprAnnotation(const ConstExprAnnotation*) override {
        return "# @constexpr - Compile-time evaluable";
    }
};
