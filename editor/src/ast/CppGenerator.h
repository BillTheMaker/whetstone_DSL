#pragma once
#include "ProjectionGenerator.h"
#include "../TypeAwareMappings.h"

class CppGenerator : public ProjectionGenerator {
public:
    std::string generate(const ASTNode* node) override {
        return dispatchGenerate(this, node, "// Unknown concept: ");
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;

        // Generate namespace wrapper
        oss << "namespace " << module->name << " {\n\n";

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

        oss << "\n}  // namespace " << module->name << "\n";
        return oss.str();
    }

    std::string visitFunction(const Function* function) override {
        std::ostringstream oss;

        // Process annotations first (these are in the "annotations" role)
        auto annotations = function->getChildren("annotations");
        for (const auto* annotation : annotations) {
            std::string annotationCode = generate(annotation);
            if (!annotationCode.empty() && annotationCode != "// Unknown concept: Annotation") {
                oss << annotationCode << "\n";
            }
        }

        // Generate function return type
        std::string returnTypeStr = "void";  // Default
        auto returnType = function->getChild("returnType");
        if (returnType) {
            returnTypeStr = generate(returnType);
        }

        // Generate function signature
        oss << returnTypeStr << " " << function->name << "(";

        auto parameters = function->getChildren("parameters");
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) oss << ", ";
            oss << visitParameter(static_cast<const Parameter*>(parameters[i]));
        }

        oss << ") {\n";

        // Process function body
        auto body = function->getChildren("body");
        if (body.empty()) {
            oss << "    // Function body is empty\n";
            oss << "    return;";
            if (returnTypeStr != "void") {
                oss << " // TODO: return appropriate value";
            }
            oss << "\n";
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

        oss << "}\n";

        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;

        auto type = variable->getChild("type");
        std::string typeStr = "auto";  // Default
        if (type) {
            typeStr = generate(type);
        }

        auto initializer = variable->getChild("initializer");
        if (!type && initializer && initializer->conceptType == "FunctionCall") {
            auto* call = static_cast<const FunctionCall*>(initializer);
            std::string mapped = mapTypeForFunctionCall("cpp", call->functionName);
            if (!mapped.empty()) typeStr = mapped;
        }

        // Check enclosing function for memory annotations that affect variable types
        std::string wrapper = getMemoryTypeWrapper(variable);
        if (!wrapper.empty()) {
            oss << wrapper << "<" << typeStr << "> " << variable->name;
        } else {
            oss << typeStr << " " << variable->name;
        }
        if (initializer) {
            oss << " = " << generate(initializer);
        }

        oss << ";";

        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        std::ostringstream oss;

        auto type = parameter->getChild("type");
        std::string typeStr = "auto";  // Default
        if (type) {
            typeStr = generate(type);
        }

        oss << typeStr << " " << parameter->name;

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
        } else {
            oss << "/* missing target */ ";
        }

        if (value) {
            oss << generate(value);
        } else {
            oss << "/* missing value */";
        }

        oss << ";";

        return oss.str();
    }

    std::string visitReturn(const Return* ret) override {
        std::ostringstream oss;
        oss << "return ";

        auto value = ret->getChild("value");
        if (value) {
            oss << generate(value);
        } else {
            // For void returns
            oss << ";";
            return oss.str();
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
        // In C++, strings need to be enclosed in double quotes
        return "\"" + lit->value + "\"";
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "true" : "false";
    }

    std::string visitNullLiteral(const NullLiteral* lit) override {
        return "nullptr";
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
            oss << "true";  // fallback
        }
        oss << ") {\n";

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
        oss << "}";

        // Else branch
        if (!elseBranch.empty()) {
            oss << " else {\n";
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
            oss << "true";  // fallback to infinite loop
        }
        oss << ") {\n";

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

        oss << "}";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;

        auto iterable = loop->getChild("iterable");
        auto body = loop->getChildren("body");

        std::string iterator = loop->iteratorName.empty() ? "item" : loop->iteratorName;

        oss << "for (auto& " << iterator << " : ";
        if (iterable) {
            oss << generate(iterable);
        } else {
            oss << "/* missing iterable */";  // fallback
        }
        oss << ") {\n";

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

        oss << "}";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        std::ostringstream oss;

        auto expr = stmt->getChild("expression");
        if (expr) {
            oss << generate(expr) << ";";
        } else {
            oss << ";";  // Empty statement
        }

        return oss.str();
    }

    std::string visitUnaryOperation(const UnaryOperation* unOp) override {
        std::ostringstream oss;

        auto operand = unOp->getChild("operand");
        std::string op = unOp->op;

        oss << op << " ";
        if (operand) {
            oss << generate(operand);
        } else {
            oss << "/* missing operand */";
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
        if (statements.empty()) {
            oss << "{}";
        } else {
            oss << "{\n";
            for (size_t i = 0; i < statements.size(); ++i) {
                std::string stmtCode = generate(statements[i]);
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
                if (i < statements.size() - 1) oss << "\n";
            }
            oss << "}";
        }

        return oss.str();
    }

    std::string visitListLiteral(const ListLiteral* lit) override {
        std::ostringstream oss;
        oss << "{";

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
            oss << "/* missing index */";
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
        // Map common types to C++ equivalents
        if (kind == "int") return "int";
        if (kind == "float") return "float";
        if (kind == "string") return "std::string";
        if (kind == "bool") return "bool";
        if (kind == "char") return "char";
        if (kind == "double") return "double";
        if (kind == "long") return "long";
        if (kind == "short") return "short";
        if (kind == "byte") return "char";  // C++ doesn't have byte, use char
        if (kind == "void") return "void";

        return kind;  // Return as-is if not a common type
    }

#include "ast/CppGeneratorTypes.h"
