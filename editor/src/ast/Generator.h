#pragma once
#include <string>
#include <sstream>
#include <map>
#include <vector>
#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Statement.h"
#include "Expression.h"
#include "Type.h"
#include "Annotation.h"

class ProjectionGenerator {
public:
    virtual ~ProjectionGenerator() = default;
    
    virtual std::string generate(const ASTNode* node) = 0;
    virtual std::string visitModule(const Module* module) = 0;
    virtual std::string visitFunction(const Function* function) = 0;
    virtual std::string visitVariable(const Variable* variable) = 0;
    virtual std::string visitParameter(const Parameter* parameter) = 0;
    virtual std::string visitAssignment(const Assignment* assignment) = 0;
    virtual std::string visitReturn(const Return* ret) = 0;
    virtual std::string visitBinaryOperation(const BinaryOperation* binOp) = 0;
    virtual std::string visitVariableReference(const VariableReference* varRef) = 0;
    virtual std::string visitIntegerLiteral(const IntegerLiteral* lit) = 0;
    virtual std::string visitFloatLiteral(const FloatLiteral* lit) = 0;
    virtual std::string visitStringLiteral(const StringLiteral* lit) = 0;
    virtual std::string visitBooleanLiteral(const BooleanLiteral* lit) = 0;
    virtual std::string visitNullLiteral(const NullLiteral* lit) = 0;
    virtual std::string visitIfStatement(const IfStatement* stmt) = 0;
    virtual std::string visitWhileLoop(const WhileLoop* loop) = 0;
    virtual std::string visitForLoop(const ForLoop* loop) = 0;
    virtual std::string visitExpressionStatement(const ExpressionStatement* stmt) = 0;
    virtual std::string visitUnaryOperation(const UnaryOperation* unOp) = 0;
    virtual std::string visitFunctionCall(const FunctionCall* call) = 0;
    virtual std::string visitBlock(const Block* block) = 0;
    virtual std::string visitListLiteral(const ListLiteral* lit) = 0;
    virtual std::string visitIndexAccess(const IndexAccess* access) = 0;
    virtual std::string visitMemberAccess(const MemberAccess* access) = 0;
    virtual std::string visitPrimitiveType(const PrimitiveType* type) = 0;
    virtual std::string visitListType(const ListType* type) = 0;
    virtual std::string visitSetType(const SetType* type) = 0;
    virtual std::string visitMapType(const MapType* type) = 0;
    virtual std::string visitTupleType(const TupleType* type) = 0;
    virtual std::string visitArrayType(const ArrayType* type) = 0;
    virtual std::string visitOptionalType(const OptionalType* type) = 0;
    virtual std::string visitCustomType(const CustomType* type) = 0;
    virtual std::string visitDerefStrategy(const DerefStrategy* annotation) = 0;
    virtual std::string visitOptimizationLock(const OptimizationLock* annotation) = 0;
    virtual std::string visitLangSpecific(const LangSpecific* annotation) = 0;
};

class PythonGenerator : public ProjectionGenerator {
public:
    std::string generate(const ASTNode* node) override {
        if (!node) return "";
        
        if (node->conceptType == "Module") {
            return visitModule(static_cast<const Module*>(node));
        } else if (node->conceptType == "Function") {
            return visitFunction(static_cast<const Function*>(node));
        } else if (node->conceptType == "Variable") {
            return visitVariable(static_cast<const Variable*>(node));
        } else if (node->conceptType == "Parameter") {
            return visitParameter(static_cast<const Parameter*>(node));
        } else if (node->conceptType == "Assignment") {
            return visitAssignment(static_cast<const Assignment*>(node));
        } else if (node->conceptType == "Return") {
            return visitReturn(static_cast<const Return*>(node));
        } else if (node->conceptType == "BinaryOperation") {
            return visitBinaryOperation(static_cast<const BinaryOperation*>(node));
        } else if (node->conceptType == "VariableReference") {
            return visitVariableReference(static_cast<const VariableReference*>(node));
        } else if (node->conceptType == "IntegerLiteral") {
            return visitIntegerLiteral(static_cast<const IntegerLiteral*>(node));
        } else if (node->conceptType == "FloatLiteral") {
            return visitFloatLiteral(static_cast<const FloatLiteral*>(node));
        } else if (node->conceptType == "StringLiteral") {
            return visitStringLiteral(static_cast<const StringLiteral*>(node));
        } else if (node->conceptType == "BooleanLiteral") {
            return visitBooleanLiteral(static_cast<const BooleanLiteral*>(node));
        } else if (node->conceptType == "NullLiteral") {
            return visitNullLiteral(static_cast<const NullLiteral*>(node));
        } else if (node->conceptType == "IfStatement") {
            return visitIfStatement(static_cast<const IfStatement*>(node));
        } else if (node->conceptType == "WhileLoop") {
            return visitWhileLoop(static_cast<const WhileLoop*>(node));
        } else if (node->conceptType == "ForLoop") {
            return visitForLoop(static_cast<const ForLoop*>(node));
        } else if (node->conceptType == "ExpressionStatement") {
            return visitExpressionStatement(static_cast<const ExpressionStatement*>(node));
        } else if (node->conceptType == "UnaryOperation") {
            return visitUnaryOperation(static_cast<const UnaryOperation*>(node));
        } else if (node->conceptType == "FunctionCall") {
            return visitFunctionCall(static_cast<const FunctionCall*>(node));
        } else if (node->conceptType == "Block") {
            return visitBlock(static_cast<const Block*>(node));
        } else if (node->conceptType == "ListLiteral") {
            return visitListLiteral(static_cast<const ListLiteral*>(node));
        } else if (node->conceptType == "IndexAccess") {
            return visitIndexAccess(static_cast<const IndexAccess*>(node));
        } else if (node->conceptType == "MemberAccess") {
            return visitMemberAccess(static_cast<const MemberAccess*>(node));
        } else if (node->conceptType == "PrimitiveType") {
            return visitPrimitiveType(static_cast<const PrimitiveType*>(node));
        } else if (node->conceptType == "ListType") {
            return visitListType(static_cast<const ListType*>(node));
        } else if (node->conceptType == "SetType") {
            return visitSetType(static_cast<const SetType*>(node));
        } else if (node->conceptType == "MapType") {
            return visitMapType(static_cast<const MapType*>(node));
        } else if (node->conceptType == "TupleType") {
            return visitTupleType(static_cast<const TupleType*>(node));
        } else if (node->conceptType == "ArrayType") {
            return visitArrayType(static_cast<const ArrayType*>(node));
        } else if (node->conceptType == "OptionalType") {
            return visitOptionalType(static_cast<const OptionalType*>(node));
        } else if (node->conceptType == "CustomType") {
            return visitCustomType(static_cast<const CustomType*>(node));
        } else if (node->conceptType == "DerefStrategy") {
            return visitDerefStrategy(static_cast<const DerefStrategy*>(node));
        } else if (node->conceptType == "OptimizationLock") {
            return visitOptimizationLock(static_cast<const OptimizationLock*>(node));
        } else if (node->conceptType == "LangSpecific") {
            return visitLangSpecific(static_cast<const LangSpecific*>(node));
        }
        
        return "// Unknown concept: " + node->conceptType;
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
};

class ElispGenerator : public ProjectionGenerator {
public:
    std::string generate(const ASTNode* node) override {
        if (!node) return "";
        
        if (node->conceptType == "Module") {
            return visitModule(static_cast<const Module*>(node));
        } else if (node->conceptType == "Function") {
            return visitFunction(static_cast<const Function*>(node));
        } else if (node->conceptType == "Variable") {
            return visitVariable(static_cast<const Variable*>(node));
        } else if (node->conceptType == "Parameter") {
            return visitParameter(static_cast<const Parameter*>(node));
        } else if (node->conceptType == "Assignment") {
            return visitAssignment(static_cast<const Assignment*>(node));
        } else if (node->conceptType == "Return") {
            return visitReturn(static_cast<const Return*>(node));
        } else if (node->conceptType == "BinaryOperation") {
            return visitBinaryOperation(static_cast<const BinaryOperation*>(node));
        } else if (node->conceptType == "VariableReference") {
            return visitVariableReference(static_cast<const VariableReference*>(node));
        } else if (node->conceptType == "IntegerLiteral") {
            return visitIntegerLiteral(static_cast<const IntegerLiteral*>(node));
        } else if (node->conceptType == "FloatLiteral") {
            return visitFloatLiteral(static_cast<const FloatLiteral*>(node));
        } else if (node->conceptType == "StringLiteral") {
            return visitStringLiteral(static_cast<const StringLiteral*>(node));
        } else if (node->conceptType == "BooleanLiteral") {
            return visitBooleanLiteral(static_cast<const BooleanLiteral*>(node));
        } else if (node->conceptType == "NullLiteral") {
            return visitNullLiteral(static_cast<const NullLiteral*>(node));
        } else if (node->conceptType == "IfStatement") {
            return visitIfStatement(static_cast<const IfStatement*>(node));
        } else if (node->conceptType == "WhileLoop") {
            return visitWhileLoop(static_cast<const WhileLoop*>(node));
        } else if (node->conceptType == "ForLoop") {
            return visitForLoop(static_cast<const ForLoop*>(node));
        } else if (node->conceptType == "ExpressionStatement") {
            return visitExpressionStatement(static_cast<const ExpressionStatement*>(node));
        } else if (node->conceptType == "UnaryOperation") {
            return visitUnaryOperation(static_cast<const UnaryOperation*>(node));
        } else if (node->conceptType == "FunctionCall") {
            return visitFunctionCall(static_cast<const FunctionCall*>(node));
        } else if (node->conceptType == "Block") {
            return visitBlock(static_cast<const Block*>(node));
        } else if (node->conceptType == "ListLiteral") {
            return visitListLiteral(static_cast<const ListLiteral*>(node));
        } else if (node->conceptType == "IndexAccess") {
            return visitIndexAccess(static_cast<const IndexAccess*>(node));
        } else if (node->conceptType == "MemberAccess") {
            return visitMemberAccess(static_cast<const MemberAccess*>(node));
        } else if (node->conceptType == "PrimitiveType") {
            return visitPrimitiveType(static_cast<const PrimitiveType*>(node));
        } else if (node->conceptType == "ListType") {
            return visitListType(static_cast<const ListType*>(node));
        } else if (node->conceptType == "SetType") {
            return visitSetType(static_cast<const SetType*>(node));
        } else if (node->conceptType == "MapType") {
            return visitMapType(static_cast<const MapType*>(node));
        } else if (node->conceptType == "TupleType") {
            return visitTupleType(static_cast<const TupleType*>(node));
        } else if (node->conceptType == "ArrayType") {
            return visitArrayType(static_cast<const ArrayType*>(node));
        } else if (node->conceptType == "OptionalType") {
            return visitOptionalType(static_cast<const OptionalType*>(node));
        } else if (node->conceptType == "CustomType") {
            return visitCustomType(static_cast<const CustomType*>(node));
        } else if (node->conceptType == "DerefStrategy") {
            return visitDerefStrategy(static_cast<const DerefStrategy*>(node));
        } else if (node->conceptType == "OptimizationLock") {
            return visitOptimizationLock(static_cast<const OptimizationLock*>(node));
        } else if (node->conceptType == "LangSpecific") {
            return visitLangSpecific(static_cast<const LangSpecific*>(node));
        }
        
        return "; Unknown concept: " + node->conceptType;
    }

    std::string visitModule(const Module* module) override {
        std::ostringstream oss;
        
        oss << "; Module: " << module->name << "\n\n";
        
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
        
        // Process annotations first
        auto annotations = function->getChildren("annotations");
        for (const auto* annotation : annotations) {
            std::string annotationCode = generate(annotation);
            if (!annotationCode.empty()) {
                oss << annotationCode << "\n";
            }
        }
        
        // Generate function definition in Elisp
        oss << "(defun " << function->name << " (";
        
        auto parameters = function->getChildren("parameters");
        for (size_t i = 0; i < parameters.size(); ++i) {
            if (i > 0) oss << " ";
            oss << visitParameter(static_cast<const Parameter*>(parameters[i]));
        }
        
        oss << ")\n";
        
        // Process function body
        auto body = function->getChildren("body");
        if (body.empty()) {
            oss << "  nil)\n";  // Return nil if no body
        } else {
            for (const auto* stmt : body) {
                std::string stmtCode = generate(stmt);
                // Indent each line of the statement
                size_t pos = 0;
                while (pos < stmtCode.length()) {
                    size_t newlinePos = stmtCode.find('\n', pos);
                    if (newlinePos == std::string::npos) {
                        oss << "  " << stmtCode.substr(pos) << "\n";
                        break;
                    } else {
                        oss << "  " << stmtCode.substr(pos, newlinePos - pos) << "\n";
                        pos = newlinePos + 1;
                        if (pos >= stmtCode.length()) break;
                    }
                }
            }
            oss << ")\n";  // Close the function
        }
        
        return oss.str();
    }

    std::string visitVariable(const Variable* variable) override {
        std::ostringstream oss;
        oss << "(defvar " << variable->name << " ";
        
        auto initializer = variable->getChild("initializer");
        if (initializer) {
            oss << generate(initializer);
        } else {
            oss << "nil";
        }
        
        oss << ")  ; Variable declaration\n";
        return oss.str();
    }

    std::string visitParameter(const Parameter* parameter) override {
        // In Elisp, parameters are just symbols
        return parameter->name;
    }

    std::string visitAssignment(const Assignment* assignment) override {
        std::ostringstream oss;
        
        auto target = assignment->getChild("target");
        auto value = assignment->getChild("value");
        
        if (target && value) {
            oss << "(setq " << generate(target) << " " << generate(value) << ")";
        } else if (target) {
            oss << "(setq " << generate(target) << " nil)";
        } else {
            oss << "nil";
        }
        
        return oss.str();
    }

    std::string visitReturn(const Return* ret) override {
        // In Elisp, the last expression in a function is the return value
        // So we just return the value expression
        auto value = ret->getChild("value");
        if (value) {
            return generate(value);
        } else {
            return "nil";
        }
    }

    std::string visitBinaryOperation(const BinaryOperation* binOp) override {
        std::ostringstream oss;
        
        auto left = binOp->getChild("left");
        auto right = binOp->getChild("right");
        
        // Map operators to Elisp equivalents
        std::string op = binOp->op;
        if (op == "+") op = "+";
        else if (op == "-") op = "-";
        else if (op == "*") op = "*";
        else if (op == "/") op = "/";
        else if (op == "==") op = "=";
        else if (op == "!=") op = "/=";
        else if (op == "<") op = "<";
        else if (op == ">") op = ">";
        else if (op == "<=") op = "<=";
        else if (op == ">=") op = ">=";
        else if (op == "and") op = "and";
        else if (op == "or") op = "or";
        
        oss << "(" << op << " ";
        if (left) {
            oss << generate(left);
        } else {
            oss << "nil";
        }
        oss << " ";
        if (right) {
            oss << generate(right);
        } else {
            oss << "nil";
        }
        oss << ")";
        
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
        // In Elisp, strings are enclosed in double quotes
        return "\"" + lit->value + "\"";
    }

    std::string visitBooleanLiteral(const BooleanLiteral* lit) override {
        return lit->value ? "t" : "nil";
    }

    std::string visitNullLiteral(const NullLiteral* lit) override {
        return "nil";
    }

    std::string visitIfStatement(const IfStatement* stmt) override {
        std::ostringstream oss;
        
        auto condition = stmt->getChild("condition");
        auto thenBranch = stmt->getChildren("thenBranch");
        auto elseBranch = stmt->getChildren("elseBranch");
        
        oss << "(if ";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "nil";  // fallback
        }
        oss << "\n";
        
        // Then branch
        if (!thenBranch.empty()) {
            for (size_t i = 0; i < thenBranch.size(); ++i) {
                if (i > 0) oss << "\n";
                oss << "    " << generate(thenBranch[i]);
            }
        } else {
            oss << "    nil";
        }
        
        // Else branch
        if (!elseBranch.empty()) {
            oss << "\n";
            for (size_t i = 0; i < elseBranch.size(); ++i) {
                if (i > 0) oss << "\n";
                oss << "    " << generate(elseBranch[i]);
            }
        } else {
            oss << "\n    nil";
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitWhileLoop(const WhileLoop* loop) override {
        std::ostringstream oss;
        
        auto condition = loop->getChild("condition");
        auto body = loop->getChildren("body");
        
        oss << "(while ";
        if (condition) {
            oss << generate(condition);
        } else {
            oss << "t";  // fallback to infinite loop
        }
        oss << "\n";
        
        for (size_t i = 0; i < body.size(); ++i) {
            if (i > 0) oss << "\n";
            oss << "  " << generate(body[i]);
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitForLoop(const ForLoop* loop) override {
        std::ostringstream oss;
        
        auto iterable = loop->getChild("iterable");
        auto body = loop->getChildren("body");
        
        std::string iterator = loop->iteratorName.empty() ? "item" : loop->iteratorName;
        
        oss << "(dolist (" << iterator << " ";
        if (iterable) {
            oss << generate(iterable);
        } else {
            oss << "nil";  // fallback
        }
        oss << ")\n";
        
        for (size_t i = 0; i < body.size(); ++i) {
            if (i > 0) oss << "\n";
            oss << "  " << generate(body[i]);
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitExpressionStatement(const ExpressionStatement* stmt) override {
        std::ostringstream oss;
        
        auto expr = stmt->getChild("expression");
        if (expr) {
            oss << generate(expr);
        } else {
            oss << "nil";
        }
        
        return oss.str();
    }

    std::string visitUnaryOperation(const UnaryOperation* unOp) override {
        std::ostringstream oss;
        
        auto operand = unOp->getChild("operand");
        std::string op = unOp->op;
        
        // Map unary operators to Elisp equivalents
        if (op == "!") op = "not";
        else if (op == "-") op = "-";
        else if (op == "+") op = "+";
        
        oss << "(" << op << " ";
        if (operand) {
            oss << generate(operand);
        } else {
            oss << "nil";
        }
        oss << ")";
        
        return oss.str();
    }

    std::string visitFunctionCall(const FunctionCall* call) override {
        std::ostringstream oss;
        oss << "(" << call->functionName;
        
        auto arguments = call->getChildren("arguments");
        for (const auto* arg : arguments) {
            oss << " " << generate(arg);
        }
        
        oss << ")";
        
        return oss.str();
    }

    std::string visitBlock(const Block* block) override {
        std::ostringstream oss;
        
        auto statements = block->getChildren("statements");
        if (statements.empty()) {
            oss << "nil";
        } else {
            oss << "(progn\n";
            for (size_t i = 0; i < statements.size(); ++i) {
                oss << "  " << generate(statements[i]);
                if (i < statements.size() - 1) oss << "\n";
            }
            oss << ")";
        }
        
        return oss.str();
    }

    std::string visitListLiteral(const ListLiteral* lit) override {
        std::ostringstream oss;
        oss << "(list";
        
        auto elements = lit->getChildren("elements");
        for (const auto* elem : elements) {
            oss << " " << generate(elem);
        }
        
        oss << ")";
        
        return oss.str();
    }

    std::string visitIndexAccess(const IndexAccess* access) override {
        std::ostringstream oss;
        
        auto target = access->getChild("target");
        auto index = access->getChild("index");
        
        if (target && index) {
            // In Elisp, accessing list/array elements
            oss << "(nth " << generate(index) << " " << generate(target) << ")";
        } else if (target) {
            oss << generate(target);
        } else {
            oss << "nil";
        }
        
        return oss.str();
    }

    std::string visitMemberAccess(const MemberAccess* access) override {
        std::ostringstream oss;
        
        auto target = access->getChild("target");
        if (target) {
            // In Elisp, member access depends on the data structure
            // For now, we'll represent it as a function call
            oss << "(." << access->memberName << " " << generate(target) << ")";
        } else {
            oss << access->memberName;
        }
        
        return oss.str();
    }

    std::string visitPrimitiveType(const PrimitiveType* type) override {
        // In Elisp, types are more dynamic, but we can represent them as symbols
        std::string kind = type->kind;
        if (kind == "int") return "integer";
        if (kind == "float") return "float";
        if (kind == "string") return "string";
        if (kind == "bool") return "boolean";
        if (kind == "char") return "character";
        if (kind == "double") return "float";
        if (kind == "long") return "integer";
        if (kind == "short") return "integer";
        if (kind == "byte") return "integer";
        if (kind == "void") return "null";
        
        return kind;  // Return as-is if not a common type
    }

    std::string visitListType(const ListType* type) override {
        std::ostringstream oss;
        oss << "(list ";
        
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "t";  // t means any type in Elisp
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitSetType(const SetType* type) override {
        std::ostringstream oss;
        oss << "(hash-table :test 'equal)";
        return oss.str();  // Represent sets as hash tables with equal test
    }

    std::string visitMapType(const MapType* type) override {
        std::ostringstream oss;
        oss << "(hash-table :test 'equal)";
        return oss.str();  // Represent maps as hash tables
    }

    std::string visitTupleType(const TupleType* type) override {
        std::ostringstream oss;
        oss << "(vector";  // Represent tuples as vectors
        
        auto elementTypes = type->getChildren("elementTypes");
        for (const auto* elemType : elementTypes) {
            oss << " " << generate(elemType);
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitArrayType(const ArrayType* type) override {
        std::ostringstream oss;
        oss << "(vector ";
        
        auto elementType = type->getChild("elementType");
        if (elementType) {
            oss << generate(elementType);
        } else {
            oss << "t";
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitOptionalType(const OptionalType* type) override {
        std::ostringstream oss;
        oss << "(or null ";
        
        auto innerType = type->getChild("innerType");
        if (innerType) {
            oss << generate(innerType);
        } else {
            oss << "t";
        }
        
        oss << ")";
        return oss.str();
    }

    std::string visitCustomType(const CustomType* type) override {
        return type->typeName;
    }

    std::string visitDerefStrategy(const DerefStrategy* annotation) override {
        // Convert deref strategy to Elisp annotation/comment
        if (annotation->strategy == "batched") {
            return "; @deref(batched) - Process in batches for efficiency";
        } else if (annotation->strategy == "streamed") {
            return "; @deref(streamed) - Stream processing";
        } else if (annotation->strategy == "manual") {
            return "; @deref(manual) - Manual memory management";
        } else {
            return "; @deref(" + annotation->strategy + ") - Memory dereference strategy";
        }
    }

    std::string visitOptimizationLock(const OptimizationLock* annotation) override {
        return "; @lock(" + annotation->lockedBy + ") - Optimization locked by " + annotation->lockedBy;
    }

    std::string visitLangSpecific(const LangSpecific* annotation) override {
        return "; @lang_specific(" + annotation->language + ", " + annotation->idiomType + ")";
    }
};