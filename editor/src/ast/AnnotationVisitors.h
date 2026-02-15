#pragma once
#include "Annotation.h"
#include "../EnvironmentSpec.h"

// Pure virtual interface for Subject 2-8 annotation visitors.
// Generators that support these annotations inherit from this.
class AnnotationVisitorExtended {
public:
    virtual ~AnnotationVisitorExtended() = default;

    // Subject 2: Type System
    virtual std::string visitBitWidthAnnotation(const BitWidthAnnotation*) = 0;
    virtual std::string visitEndianAnnotation(const EndianAnnotation*) = 0;
    virtual std::string visitLayoutAnnotation(const LayoutAnnotation*) = 0;
    virtual std::string visitNullabilityAnnotation(const NullabilityAnnotation*) = 0;
    virtual std::string visitVarianceAnnotation(const VarianceAnnotation*) = 0;
    virtual std::string visitIdentityAnnotation(const IdentityAnnotation*) = 0;
    virtual std::string visitMutAnnotation(const MutAnnotation*) = 0;
    virtual std::string visitTypeStateAnnotation(const TypeStateAnnotation*) = 0;

    // Subject 3: Concurrency
    virtual std::string visitAtomicAnnotation(const AtomicAnnotation*) = 0;
    virtual std::string visitSyncAnnotation(const SyncAnnotation*) = 0;
    virtual std::string visitThreadModelAnnotation(const ThreadModelAnnotation*) = 0;
    virtual std::string visitMemoryBarrierAnnotation(const MemoryBarrierAnnotation*) = 0;
    virtual std::string visitExecAnnotation(const ExecAnnotation*) = 0;
    virtual std::string visitBlockingAnnotation(const BlockingAnnotation*) = 0;
    virtual std::string visitParallelAnnotation(const ParallelAnnotation*) = 0;
    virtual std::string visitTrapAnnotation(const TrapAnnotation*) = 0;
    virtual std::string visitExceptionAnnotation(const ExceptionAnnotation*) = 0;
    virtual std::string visitPanicAnnotation(const PanicAnnotation*) = 0;

    // Subject 4: Scope
    virtual std::string visitBindingAnnotation(const BindingAnnotation*) = 0;
    virtual std::string visitLookupAnnotation(const LookupAnnotation*) = 0;
    virtual std::string visitCaptureAnnotation(const CaptureAnnotation*) = 0;
    virtual std::string visitVisibilityAnnotation(const VisibilityAnnotation*) = 0;
    virtual std::string visitNamespaceAnnotation(const NamespaceAnnotation*) = 0;
    virtual std::string visitScopeAnnotation(const ScopeAnnotation*) = 0;

    // Subject 5: Shims & Platform
    virtual std::string visitIntrinsicAnnotation(const IntrinsicAnnotation*) = 0;
    virtual std::string visitRawAnnotation(const RawAnnotation*) = 0;
    virtual std::string visitCallingConvAnnotation(const CallingConvAnnotation*) = 0;
    virtual std::string visitLinkAnnotation(const LinkAnnotation*) = 0;
    virtual std::string visitShimAnnotation(const ShimAnnotation*) = 0;
    virtual std::string visitPointerArithmeticAnnotation(const PointerArithmeticAnnotation*) = 0;
    virtual std::string visitOpaqueAnnotation(const OpaqueAnnotation*) = 0;
    virtual std::string visitTargetAnnotation(const TargetAnnotation*) = 0;
    virtual std::string visitFeatureAnnotation(const FeatureAnnotation*) = 0;
    virtual std::string visitOriginalAnnotation(const OriginalAnnotation*) = 0;
    virtual std::string visitMappingAnnotation(const MappingAnnotation*) = 0;

    // Subject 6: Optimization
    virtual std::string visitTailCallAnnotation(const TailCallAnnotation*) = 0;
    virtual std::string visitLoopAnnotation(const LoopAnnotation*) = 0;
    virtual std::string visitDataAnnotation(const DataAnnotation*) = 0;
    virtual std::string visitAlignAnnotation(const AlignAnnotation*) = 0;
    virtual std::string visitPackAnnotation(const PackAnnotation*) = 0;
    virtual std::string visitBoundsCheckAnnotation(const BoundsCheckAnnotation*) = 0;
    virtual std::string visitOverflowAnnotation(const OverflowAnnotation*) = 0;

    // Subject 7: Meta-Programming
    virtual std::string visitMetaAnnotation(const MetaAnnotation*) = 0;
    virtual std::string visitSymbolAnnotation(const SymbolAnnotation*) = 0;
    virtual std::string visitEvaluateAnnotation(const EvaluateAnnotation*) = 0;
    virtual std::string visitTemplateAnnotation(const TemplateAnnotation*) = 0;
    virtual std::string visitSyntheticAnnotation(const SyntheticAnnotation*) = 0;

    // Subject 8: Policy
    virtual std::string visitPolicyAnnotation(const PolicyAnnotation*) = 0;
    virtual std::string visitAmbiguityAnnotation(const AmbiguityAnnotation*) = 0;
    virtual std::string visitCandidateAnnotation(const CandidateAnnotation*) = 0;
    virtual std::string visitTradeoffAnnotation(const TradeoffAnnotation*) = 0;
    virtual std::string visitChoiceAnnotation(const ChoiceAnnotation*) = 0;
    virtual std::string visitDecisionAnnotation(const DecisionAnnotation*) = 0;

    // Subject 9: Workflow Routing
    virtual std::string visitContextWidthAnnotation(const ContextWidthAnnotation*) = 0;
    virtual std::string visitReviewAnnotation(const ReviewAnnotation*) = 0;
    virtual std::string visitAutomatabilityAnnotation(const AutomatabilityAnnotation*) = 0;
    virtual std::string visitPriorityAnnotation(const PriorityAnnotation*) = 0;
    virtual std::string visitImplementationStatusAnnotation(const ImplementationStatusAnnotation*) = 0;

    // Semantic Core
    virtual std::string visitIntentAnnotation(const IntentAnnotation*) = 0;
    virtual std::string visitComplexityAnnotation(const ComplexityAnnotation*) = 0;
    virtual std::string visitRiskAnnotation(const RiskAnnotation*) = 0;
    virtual std::string visitContractAnnotation(const ContractAnnotation*) = 0;
    virtual std::string visitSemanticTagAnnotation(const SemanticTagAnnotation*) = 0;

    // Environment
    virtual std::string visitCapabilityRequirement(const CapabilityRequirement*) = 0;
};
