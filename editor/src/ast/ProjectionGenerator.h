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
    virtual std::string visitDeallocateAnnotation(const DeallocateAnnotation* annotation) = 0;
    virtual std::string visitLifetimeAnnotation(const LifetimeAnnotation* annotation) = 0;
    virtual std::string visitReclaimAnnotation(const ReclaimAnnotation* annotation) = 0;
    virtual std::string visitOwnerAnnotation(const OwnerAnnotation* annotation) = 0;
    virtual std::string visitAllocateAnnotation(const AllocateAnnotation* annotation) = 0;
    virtual std::string visitHotColdAnnotation(const HotColdAnnotation* annotation) = 0;
    virtual std::string visitInlineAnnotation(const InlineAnnotation* annotation) = 0;
    virtual std::string visitPureAnnotation(const PureAnnotation* annotation) = 0;
    virtual std::string visitConstExprAnnotation(const ConstExprAnnotation* annotation) = 0;
};

// Shared dispatch: maps conceptType string to the appropriate visit call.
// Each generator's generate() calls this instead of duplicating the dispatch chain.
template<typename Gen>
std::string dispatchGenerate(Gen* gen, const ASTNode* node, const std::string& unknownPrefix) {
    if (!node) return "";

    if (node->conceptType == "Module") {
        return gen->visitModule(static_cast<const Module*>(node));
    } else if (node->conceptType == "Function") {
        return gen->visitFunction(static_cast<const Function*>(node));
    } else if (node->conceptType == "Variable") {
        return gen->visitVariable(static_cast<const Variable*>(node));
    } else if (node->conceptType == "Parameter") {
        return gen->visitParameter(static_cast<const Parameter*>(node));
    } else if (node->conceptType == "Assignment") {
        return gen->visitAssignment(static_cast<const Assignment*>(node));
    } else if (node->conceptType == "Return") {
        return gen->visitReturn(static_cast<const Return*>(node));
    } else if (node->conceptType == "BinaryOperation") {
        return gen->visitBinaryOperation(static_cast<const BinaryOperation*>(node));
    } else if (node->conceptType == "VariableReference") {
        return gen->visitVariableReference(static_cast<const VariableReference*>(node));
    } else if (node->conceptType == "IntegerLiteral") {
        return gen->visitIntegerLiteral(static_cast<const IntegerLiteral*>(node));
    } else if (node->conceptType == "FloatLiteral") {
        return gen->visitFloatLiteral(static_cast<const FloatLiteral*>(node));
    } else if (node->conceptType == "StringLiteral") {
        return gen->visitStringLiteral(static_cast<const StringLiteral*>(node));
    } else if (node->conceptType == "BooleanLiteral") {
        return gen->visitBooleanLiteral(static_cast<const BooleanLiteral*>(node));
    } else if (node->conceptType == "NullLiteral") {
        return gen->visitNullLiteral(static_cast<const NullLiteral*>(node));
    } else if (node->conceptType == "IfStatement") {
        return gen->visitIfStatement(static_cast<const IfStatement*>(node));
    } else if (node->conceptType == "WhileLoop") {
        return gen->visitWhileLoop(static_cast<const WhileLoop*>(node));
    } else if (node->conceptType == "ForLoop") {
        return gen->visitForLoop(static_cast<const ForLoop*>(node));
    } else if (node->conceptType == "ExpressionStatement") {
        return gen->visitExpressionStatement(static_cast<const ExpressionStatement*>(node));
    } else if (node->conceptType == "UnaryOperation") {
        return gen->visitUnaryOperation(static_cast<const UnaryOperation*>(node));
    } else if (node->conceptType == "FunctionCall") {
        return gen->visitFunctionCall(static_cast<const FunctionCall*>(node));
    } else if (node->conceptType == "Block") {
        return gen->visitBlock(static_cast<const Block*>(node));
    } else if (node->conceptType == "ListLiteral") {
        return gen->visitListLiteral(static_cast<const ListLiteral*>(node));
    } else if (node->conceptType == "IndexAccess") {
        return gen->visitIndexAccess(static_cast<const IndexAccess*>(node));
    } else if (node->conceptType == "MemberAccess") {
        return gen->visitMemberAccess(static_cast<const MemberAccess*>(node));
    } else if (node->conceptType == "PrimitiveType") {
        return gen->visitPrimitiveType(static_cast<const PrimitiveType*>(node));
    } else if (node->conceptType == "ListType") {
        return gen->visitListType(static_cast<const ListType*>(node));
    } else if (node->conceptType == "SetType") {
        return gen->visitSetType(static_cast<const SetType*>(node));
    } else if (node->conceptType == "MapType") {
        return gen->visitMapType(static_cast<const MapType*>(node));
    } else if (node->conceptType == "TupleType") {
        return gen->visitTupleType(static_cast<const TupleType*>(node));
    } else if (node->conceptType == "ArrayType") {
        return gen->visitArrayType(static_cast<const ArrayType*>(node));
    } else if (node->conceptType == "OptionalType") {
        return gen->visitOptionalType(static_cast<const OptionalType*>(node));
    } else if (node->conceptType == "CustomType") {
        return gen->visitCustomType(static_cast<const CustomType*>(node));
    } else if (node->conceptType == "DerefStrategy") {
        return gen->visitDerefStrategy(static_cast<const DerefStrategy*>(node));
    } else if (node->conceptType == "OptimizationLock") {
        return gen->visitOptimizationLock(static_cast<const OptimizationLock*>(node));
    } else if (node->conceptType == "LangSpecific") {
        return gen->visitLangSpecific(static_cast<const LangSpecific*>(node));
    } else if (node->conceptType == "DeallocateAnnotation") {
        return gen->visitDeallocateAnnotation(static_cast<const DeallocateAnnotation*>(node));
    } else if (node->conceptType == "LifetimeAnnotation") {
        return gen->visitLifetimeAnnotation(static_cast<const LifetimeAnnotation*>(node));
    } else if (node->conceptType == "ReclaimAnnotation") {
        return gen->visitReclaimAnnotation(static_cast<const ReclaimAnnotation*>(node));
    } else if (node->conceptType == "OwnerAnnotation") {
        return gen->visitOwnerAnnotation(static_cast<const OwnerAnnotation*>(node));
    } else if (node->conceptType == "AllocateAnnotation") {
        return gen->visitAllocateAnnotation(static_cast<const AllocateAnnotation*>(node));
    } else if (node->conceptType == "HotColdAnnotation") {
        return gen->visitHotColdAnnotation(static_cast<const HotColdAnnotation*>(node));
    } else if (node->conceptType == "InlineAnnotation") {
        return gen->visitInlineAnnotation(static_cast<const InlineAnnotation*>(node));
    } else if (node->conceptType == "PureAnnotation") {
        return gen->visitPureAnnotation(static_cast<const PureAnnotation*>(node));
    } else if (node->conceptType == "ConstExprAnnotation") {
        return gen->visitConstExprAnnotation(static_cast<const ConstExprAnnotation*>(node));
    }

    return unknownPrefix + node->conceptType;
}
