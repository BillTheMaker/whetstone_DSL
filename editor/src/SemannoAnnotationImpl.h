#pragma once
// SemannoAnnotationImpl.h — CRTP mixin for Semanno annotation visitor methods
//
// Provides default implementations for all 56 extended annotation visitor
// methods defined in AnnotationVisitorExtended.  Each implementation delegates
// to SemannoEmitter::emit() and prepends the language-specific comment prefix
// supplied by the Derived generator via commentPrefix().
//
// Usage:
//   class PythonGenerator : public ProjectionGenerator,
//                           public SemannoAnnotationImpl<PythonGenerator> {
//   public:
//       std::string commentPrefix() const { return "# "; }
//       // ... the 56 visit*() methods are inherited from the mixin ...
//   };

#include "SemannoFormat.h"
#include "ast/AnnotationVisitors.h"

// CRTP mixin: Derived must provide commentPrefix() -> std::string
// Uses virtual inheritance so methods properly override the pure virtuals
// in AnnotationVisitorExtended shared with ProjectionGenerator.
template<typename Derived>
class SemannoAnnotationImpl : public virtual AnnotationVisitorExtended {
protected:
    std::string semanno(const ASTNode* anno) const {
        auto* self = static_cast<const Derived*>(this);
        return self->commentPrefix() + SemannoEmitter::emit(anno);
    }

public:
    // Subject 2: Type System
    std::string visitBitWidthAnnotation(const BitWidthAnnotation* a) override { return semanno(a); }
    std::string visitEndianAnnotation(const EndianAnnotation* a) override { return semanno(a); }
    std::string visitLayoutAnnotation(const LayoutAnnotation* a) override { return semanno(a); }
    std::string visitNullabilityAnnotation(const NullabilityAnnotation* a) override { return semanno(a); }
    std::string visitVarianceAnnotation(const VarianceAnnotation* a) override { return semanno(a); }
    std::string visitIdentityAnnotation(const IdentityAnnotation* a) override { return semanno(a); }
    std::string visitMutAnnotation(const MutAnnotation* a) override { return semanno(a); }
    std::string visitTypeStateAnnotation(const TypeStateAnnotation* a) override { return semanno(a); }

    // Subject 3: Concurrency
    std::string visitAtomicAnnotation(const AtomicAnnotation* a) override { return semanno(a); }
    std::string visitSyncAnnotation(const SyncAnnotation* a) override { return semanno(a); }
    std::string visitThreadModelAnnotation(const ThreadModelAnnotation* a) override { return semanno(a); }
    std::string visitMemoryBarrierAnnotation(const MemoryBarrierAnnotation* a) override { return semanno(a); }
    std::string visitExecAnnotation(const ExecAnnotation* a) override { return semanno(a); }
    std::string visitBlockingAnnotation(const BlockingAnnotation* a) override { return semanno(a); }
    std::string visitParallelAnnotation(const ParallelAnnotation* a) override { return semanno(a); }
    std::string visitTrapAnnotation(const TrapAnnotation* a) override { return semanno(a); }
    std::string visitExceptionAnnotation(const ExceptionAnnotation* a) override { return semanno(a); }
    std::string visitPanicAnnotation(const PanicAnnotation* a) override { return semanno(a); }

    // Subject 4: Scope
    std::string visitBindingAnnotation(const BindingAnnotation* a) override { return semanno(a); }
    std::string visitLookupAnnotation(const LookupAnnotation* a) override { return semanno(a); }
    std::string visitCaptureAnnotation(const CaptureAnnotation* a) override { return semanno(a); }
    std::string visitVisibilityAnnotation(const VisibilityAnnotation* a) override { return semanno(a); }
    std::string visitNamespaceAnnotation(const NamespaceAnnotation* a) override { return semanno(a); }
    std::string visitScopeAnnotation(const ScopeAnnotation* a) override { return semanno(a); }

    // Subject 5: Shims & Platform
    std::string visitIntrinsicAnnotation(const IntrinsicAnnotation* a) override { return semanno(a); }
    std::string visitRawAnnotation(const RawAnnotation* a) override { return semanno(a); }
    std::string visitCallingConvAnnotation(const CallingConvAnnotation* a) override { return semanno(a); }
    std::string visitLinkAnnotation(const LinkAnnotation* a) override { return semanno(a); }
    std::string visitShimAnnotation(const ShimAnnotation* a) override { return semanno(a); }
    std::string visitPointerArithmeticAnnotation(const PointerArithmeticAnnotation* a) override { return semanno(a); }
    std::string visitOpaqueAnnotation(const OpaqueAnnotation* a) override { return semanno(a); }
    std::string visitTargetAnnotation(const TargetAnnotation* a) override { return semanno(a); }
    std::string visitFeatureAnnotation(const FeatureAnnotation* a) override { return semanno(a); }
    std::string visitOriginalAnnotation(const OriginalAnnotation* a) override { return semanno(a); }
    std::string visitMappingAnnotation(const MappingAnnotation* a) override { return semanno(a); }

    // Subject 6: Optimization
    std::string visitTailCallAnnotation(const TailCallAnnotation* a) override { return semanno(a); }
    std::string visitLoopAnnotation(const LoopAnnotation* a) override { return semanno(a); }
    std::string visitDataAnnotation(const DataAnnotation* a) override { return semanno(a); }
    std::string visitAlignAnnotation(const AlignAnnotation* a) override { return semanno(a); }
    std::string visitPackAnnotation(const PackAnnotation* a) override { return semanno(a); }
    std::string visitBoundsCheckAnnotation(const BoundsCheckAnnotation* a) override { return semanno(a); }
    std::string visitOverflowAnnotation(const OverflowAnnotation* a) override { return semanno(a); }

    // Subject 7: Meta-Programming
    std::string visitMetaAnnotation(const MetaAnnotation* a) override { return semanno(a); }
    std::string visitSymbolAnnotation(const SymbolAnnotation* a) override { return semanno(a); }
    std::string visitEvaluateAnnotation(const EvaluateAnnotation* a) override { return semanno(a); }
    std::string visitTemplateAnnotation(const TemplateAnnotation* a) override { return semanno(a); }
    std::string visitSyntheticAnnotation(const SyntheticAnnotation* a) override { return semanno(a); }

    // Subject 8: Policy
    std::string visitPolicyAnnotation(const PolicyAnnotation* a) override { return semanno(a); }
    std::string visitAmbiguityAnnotation(const AmbiguityAnnotation* a) override { return semanno(a); }
    std::string visitCandidateAnnotation(const CandidateAnnotation* a) override { return semanno(a); }
    std::string visitTradeoffAnnotation(const TradeoffAnnotation* a) override { return semanno(a); }
    std::string visitChoiceAnnotation(const ChoiceAnnotation* a) override { return semanno(a); }
    std::string visitDecisionAnnotation(const DecisionAnnotation* a) override { return semanno(a); }

    // Semantic Core
    std::string visitIntentAnnotation(const IntentAnnotation* a) override { return semanno(a); }
    std::string visitComplexityAnnotation(const ComplexityAnnotation* a) override { return semanno(a); }
    std::string visitRiskAnnotation(const RiskAnnotation* a) override { return semanno(a); }
    std::string visitContractAnnotation(const ContractAnnotation* a) override { return semanno(a); }
    std::string visitSemanticTagAnnotation(const SemanticTagAnnotation* a) override { return semanno(a); }

    // Environment
    std::string visitCapabilityRequirement(const CapabilityRequirement* a) override { return semanno(a); }
};
