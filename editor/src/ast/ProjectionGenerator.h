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
#include "AnnotationVisitors.h"
#include "HostBoundary.h"

class ProjectionGenerator : public virtual AnnotationVisitorExtended {
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

    // New AST node visitors (Phase 11c)
    virtual std::string visitClassDeclaration(const ASTNode* node) { return ""; }
    virtual std::string visitInterfaceDeclaration(const ASTNode* node) { return ""; }
    virtual std::string visitMethodDeclaration(const ASTNode* node) { return ""; }
    virtual std::string visitGenericType(const ASTNode* node) { return ""; }
    virtual std::string visitTypeParameter(const ASTNode* node) { return ""; }
    virtual std::string visitAsyncFunction(const ASTNode* node) { return ""; }
    virtual std::string visitAwaitExpression(const ASTNode* node) { return ""; }
    virtual std::string visitLambdaExpression(const ASTNode* node) { return ""; }
    virtual std::string visitDecoratorAnnotation(const ASTNode* node) { return ""; }

    // Host boundary visitors
    virtual std::string visitHostCall(const HostCall* node) { return ""; }
    virtual std::string visitScheduleTask(const ScheduleTask* node) { return ""; }
    virtual std::string visitModuleLoad(const ModuleLoad* node) { return ""; }
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
    // Subject 2: Type System (Steps 291-292)
    else if (node->conceptType == "BitWidthAnnotation") {
        return gen->visitBitWidthAnnotation(static_cast<const BitWidthAnnotation*>(node));
    } else if (node->conceptType == "EndianAnnotation") {
        return gen->visitEndianAnnotation(static_cast<const EndianAnnotation*>(node));
    } else if (node->conceptType == "LayoutAnnotation") {
        return gen->visitLayoutAnnotation(static_cast<const LayoutAnnotation*>(node));
    } else if (node->conceptType == "NullabilityAnnotation") {
        return gen->visitNullabilityAnnotation(static_cast<const NullabilityAnnotation*>(node));
    } else if (node->conceptType == "VarianceAnnotation") {
        return gen->visitVarianceAnnotation(static_cast<const VarianceAnnotation*>(node));
    } else if (node->conceptType == "IdentityAnnotation") {
        return gen->visitIdentityAnnotation(static_cast<const IdentityAnnotation*>(node));
    } else if (node->conceptType == "MutAnnotation") {
        return gen->visitMutAnnotation(static_cast<const MutAnnotation*>(node));
    } else if (node->conceptType == "TypeStateAnnotation") {
        return gen->visitTypeStateAnnotation(static_cast<const TypeStateAnnotation*>(node));
    }
    // Subject 3: Concurrency
    else if (node->conceptType == "AtomicAnnotation") {
        return gen->visitAtomicAnnotation(static_cast<const AtomicAnnotation*>(node));
    } else if (node->conceptType == "SyncAnnotation") {
        return gen->visitSyncAnnotation(static_cast<const SyncAnnotation*>(node));
    } else if (node->conceptType == "ThreadModelAnnotation") {
        return gen->visitThreadModelAnnotation(static_cast<const ThreadModelAnnotation*>(node));
    } else if (node->conceptType == "MemoryBarrierAnnotation") {
        return gen->visitMemoryBarrierAnnotation(static_cast<const MemoryBarrierAnnotation*>(node));
    } else if (node->conceptType == "ExecAnnotation") {
        return gen->visitExecAnnotation(static_cast<const ExecAnnotation*>(node));
    } else if (node->conceptType == "BlockingAnnotation") {
        return gen->visitBlockingAnnotation(static_cast<const BlockingAnnotation*>(node));
    } else if (node->conceptType == "ParallelAnnotation") {
        return gen->visitParallelAnnotation(static_cast<const ParallelAnnotation*>(node));
    } else if (node->conceptType == "TrapAnnotation") {
        return gen->visitTrapAnnotation(static_cast<const TrapAnnotation*>(node));
    } else if (node->conceptType == "ExceptionAnnotation") {
        return gen->visitExceptionAnnotation(static_cast<const ExceptionAnnotation*>(node));
    } else if (node->conceptType == "PanicAnnotation") {
        return gen->visitPanicAnnotation(static_cast<const PanicAnnotation*>(node));
    }
    // Subject 4: Scope
    else if (node->conceptType == "BindingAnnotation") {
        return gen->visitBindingAnnotation(static_cast<const BindingAnnotation*>(node));
    } else if (node->conceptType == "LookupAnnotation") {
        return gen->visitLookupAnnotation(static_cast<const LookupAnnotation*>(node));
    } else if (node->conceptType == "CaptureAnnotation") {
        return gen->visitCaptureAnnotation(static_cast<const CaptureAnnotation*>(node));
    } else if (node->conceptType == "VisibilityAnnotation") {
        return gen->visitVisibilityAnnotation(static_cast<const VisibilityAnnotation*>(node));
    } else if (node->conceptType == "NamespaceAnnotation") {
        return gen->visitNamespaceAnnotation(static_cast<const NamespaceAnnotation*>(node));
    } else if (node->conceptType == "ScopeAnnotation") {
        return gen->visitScopeAnnotation(static_cast<const ScopeAnnotation*>(node));
    }
    // Subject 5: Shims & Platform
    else if (node->conceptType == "IntrinsicAnnotation") {
        return gen->visitIntrinsicAnnotation(static_cast<const IntrinsicAnnotation*>(node));
    } else if (node->conceptType == "RawAnnotation") {
        return gen->visitRawAnnotation(static_cast<const RawAnnotation*>(node));
    } else if (node->conceptType == "CallingConvAnnotation") {
        return gen->visitCallingConvAnnotation(static_cast<const CallingConvAnnotation*>(node));
    } else if (node->conceptType == "LinkAnnotation") {
        return gen->visitLinkAnnotation(static_cast<const LinkAnnotation*>(node));
    } else if (node->conceptType == "ShimAnnotation") {
        return gen->visitShimAnnotation(static_cast<const ShimAnnotation*>(node));
    } else if (node->conceptType == "PointerArithmeticAnnotation") {
        return gen->visitPointerArithmeticAnnotation(static_cast<const PointerArithmeticAnnotation*>(node));
    } else if (node->conceptType == "OpaqueAnnotation") {
        return gen->visitOpaqueAnnotation(static_cast<const OpaqueAnnotation*>(node));
    } else if (node->conceptType == "TargetAnnotation") {
        return gen->visitTargetAnnotation(static_cast<const TargetAnnotation*>(node));
    } else if (node->conceptType == "FeatureAnnotation") {
        return gen->visitFeatureAnnotation(static_cast<const FeatureAnnotation*>(node));
    } else if (node->conceptType == "OriginalAnnotation") {
        return gen->visitOriginalAnnotation(static_cast<const OriginalAnnotation*>(node));
    } else if (node->conceptType == "MappingAnnotation") {
        return gen->visitMappingAnnotation(static_cast<const MappingAnnotation*>(node));
    }
    // Subject 6: Optimization
    else if (node->conceptType == "TailCallAnnotation") {
        return gen->visitTailCallAnnotation(static_cast<const TailCallAnnotation*>(node));
    } else if (node->conceptType == "LoopAnnotation") {
        return gen->visitLoopAnnotation(static_cast<const LoopAnnotation*>(node));
    } else if (node->conceptType == "DataAnnotation") {
        return gen->visitDataAnnotation(static_cast<const DataAnnotation*>(node));
    } else if (node->conceptType == "AlignAnnotation") {
        return gen->visitAlignAnnotation(static_cast<const AlignAnnotation*>(node));
    } else if (node->conceptType == "PackAnnotation") {
        return gen->visitPackAnnotation(static_cast<const PackAnnotation*>(node));
    } else if (node->conceptType == "BoundsCheckAnnotation") {
        return gen->visitBoundsCheckAnnotation(static_cast<const BoundsCheckAnnotation*>(node));
    } else if (node->conceptType == "OverflowAnnotation") {
        return gen->visitOverflowAnnotation(static_cast<const OverflowAnnotation*>(node));
    }
    // Subject 7: Meta-Programming
    else if (node->conceptType == "MetaAnnotation") {
        return gen->visitMetaAnnotation(static_cast<const MetaAnnotation*>(node));
    } else if (node->conceptType == "SymbolAnnotation") {
        return gen->visitSymbolAnnotation(static_cast<const SymbolAnnotation*>(node));
    } else if (node->conceptType == "EvaluateAnnotation") {
        return gen->visitEvaluateAnnotation(static_cast<const EvaluateAnnotation*>(node));
    } else if (node->conceptType == "TemplateAnnotation") {
        return gen->visitTemplateAnnotation(static_cast<const TemplateAnnotation*>(node));
    } else if (node->conceptType == "SyntheticAnnotation") {
        return gen->visitSyntheticAnnotation(static_cast<const SyntheticAnnotation*>(node));
    }
    // Subject 8: Policy
    else if (node->conceptType == "PolicyAnnotation") {
        return gen->visitPolicyAnnotation(static_cast<const PolicyAnnotation*>(node));
    } else if (node->conceptType == "AmbiguityAnnotation") {
        return gen->visitAmbiguityAnnotation(static_cast<const AmbiguityAnnotation*>(node));
    } else if (node->conceptType == "CandidateAnnotation") {
        return gen->visitCandidateAnnotation(static_cast<const CandidateAnnotation*>(node));
    } else if (node->conceptType == "TradeoffAnnotation") {
        return gen->visitTradeoffAnnotation(static_cast<const TradeoffAnnotation*>(node));
    } else if (node->conceptType == "ChoiceAnnotation") {
        return gen->visitChoiceAnnotation(static_cast<const ChoiceAnnotation*>(node));
    } else if (node->conceptType == "DecisionAnnotation") {
        return gen->visitDecisionAnnotation(static_cast<const DecisionAnnotation*>(node));
    }
    // Semantic Core
    else if (node->conceptType == "IntentAnnotation") {
        return gen->visitIntentAnnotation(static_cast<const IntentAnnotation*>(node));
    } else if (node->conceptType == "ComplexityAnnotation") {
        return gen->visitComplexityAnnotation(static_cast<const ComplexityAnnotation*>(node));
    } else if (node->conceptType == "RiskAnnotation") {
        return gen->visitRiskAnnotation(static_cast<const RiskAnnotation*>(node));
    } else if (node->conceptType == "ContractAnnotation") {
        return gen->visitContractAnnotation(static_cast<const ContractAnnotation*>(node));
    } else if (node->conceptType == "SemanticTagAnnotation") {
        return gen->visitSemanticTagAnnotation(static_cast<const SemanticTagAnnotation*>(node));
    }
    // Environment
    else if (node->conceptType == "CapabilityRequirement") {
        return gen->visitCapabilityRequirement(static_cast<const CapabilityRequirement*>(node));
    }
    // Host Boundary (Step 288)
    else if (node->conceptType == "HostCall") {
        return gen->visitHostCall(static_cast<const HostCall*>(node));
    } else if (node->conceptType == "ScheduleTask") {
        return gen->visitScheduleTask(static_cast<const ScheduleTask*>(node));
    } else if (node->conceptType == "ModuleLoad") {
        return gen->visitModuleLoad(static_cast<const ModuleLoad*>(node));
    }
    // New AST nodes (Phase 11c)
    else if (node->conceptType == "ClassDeclaration") {
        return gen->visitClassDeclaration(node);
    } else if (node->conceptType == "InterfaceDeclaration") {
        return gen->visitInterfaceDeclaration(node);
    } else if (node->conceptType == "MethodDeclaration") {
        return gen->visitMethodDeclaration(node);
    } else if (node->conceptType == "GenericType") {
        return gen->visitGenericType(node);
    } else if (node->conceptType == "TypeParameter") {
        return gen->visitTypeParameter(node);
    } else if (node->conceptType == "AsyncFunction") {
        return gen->visitAsyncFunction(node);
    } else if (node->conceptType == "AwaitExpression") {
        return gen->visitAwaitExpression(node);
    } else if (node->conceptType == "LambdaExpression") {
        return gen->visitLambdaExpression(node);
    } else if (node->conceptType == "DecoratorAnnotation") {
        return gen->visitDecoratorAnnotation(node);
    }

    return unknownPrefix + node->conceptType;
}
