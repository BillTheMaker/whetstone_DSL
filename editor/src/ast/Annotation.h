#pragma once
#include "ASTNode.h"

class Annotation : public ASTNode {
public:
    Annotation() { conceptType = "Annotation"; }
};

class DerefStrategy : public Annotation {
public:
    std::string strategy;
    std::string derefLocation;
    std::string owner;
    DerefStrategy() { conceptType = "DerefStrategy"; }
    DerefStrategy(const std::string& id, const std::string& strategy)
        : strategy(strategy) {
        this->id = id;
        this->conceptType = "DerefStrategy";
    }
    // children: derefTime (0..1) via setChild("derefTime", ...)
};

class OptimizationLock : public Annotation {
public:
    std::string lockedBy;
    std::string lockReason;
    std::string lockLevel;
    std::string affectedStrategies;
    std::string timestamp;
    OptimizationLock() { conceptType = "OptimizationLock"; }
};

class LangSpecific : public Annotation {
public:
    std::string language;
    std::string idiomType;
    std::string rawSyntax;
    std::string semanticHint;
    std::string position;
    LangSpecific() { conceptType = "LangSpecific"; }
};

// New canonical memory annotations replacing the old DerefStrategy

class DeallocateAnnotation : public Annotation {
public:
    std::string strategy;  // "Explicit" for manual allocation/deallocation
    std::string deallocateLocation;
    std::string owner;
    DeallocateAnnotation() { 
        conceptType = "DeallocateAnnotation"; 
        strategy = "Explicit";
    }
    DeallocateAnnotation(const std::string& id, const std::string& strategy_val)
        : strategy(strategy_val) {
        this->id = id;
        this->conceptType = "DeallocateAnnotation";
    }
};

class LifetimeAnnotation : public Annotation {
public:
    std::string strategy;  // "RAII" for destructor-based cleanup
    std::string lifetimeScope;
    LifetimeAnnotation() { 
        conceptType = "LifetimeAnnotation"; 
        strategy = "RAII";
    }
    LifetimeAnnotation(const std::string& id, const std::string& strategy_val)
        : strategy(strategy_val) {
        this->id = id;
        this->conceptType = "LifetimeAnnotation";
    }
};

class ReclaimAnnotation : public Annotation {
public:
    std::string strategy;  // "Tracing", "Cycle", "Escape" for different GC strategies
    std::string reclaimPattern;
    ReclaimAnnotation() { 
        conceptType = "ReclaimAnnotation"; 
        strategy = "Tracing";
    }
    ReclaimAnnotation(const std::string& id, const std::string& strategy_val)
        : strategy(strategy_val) {
        this->id = id;
        this->conceptType = "ReclaimAnnotation";
    }
};

class OwnerAnnotation : public Annotation {
public:
    std::string strategy;  // "Single", "Shared_ARC" for ownership strategies
    std::string ownerType;
    OwnerAnnotation() { 
        conceptType = "OwnerAnnotation"; 
        strategy = "Single";
    }
    OwnerAnnotation(const std::string& id, const std::string& strategy_val)
        : strategy(strategy_val) {
        this->id = id;
        this->conceptType = "OwnerAnnotation";
    }
};

class AllocateAnnotation : public Annotation {
public:
    std::string strategy;  // "Static", "Register", "Allocator" for allocation strategies
    std::string allocationPattern;
    AllocateAnnotation() {
        conceptType = "AllocateAnnotation";
        strategy = "Static";
    }
    AllocateAnnotation(const std::string& id, const std::string& strategy_val)
        : strategy(strategy_val) {
        this->id = id;
        this->conceptType = "AllocateAnnotation";
    }
};

// Optimization annotations (Step 67)

class HotColdAnnotation : public Annotation {
public:
    std::string hint;  // "Hot" or "Cold"
    HotColdAnnotation() { conceptType = "HotColdAnnotation"; }
};

class InlineAnnotation : public Annotation {
public:
    std::string mode;  // "Always", "Never", "Hint"
    InlineAnnotation() { conceptType = "InlineAnnotation"; }
};

class PureAnnotation : public Annotation {
public:
    PureAnnotation() { conceptType = "PureAnnotation"; }
};

class ConstExprAnnotation : public Annotation {
public:
    ConstExprAnnotation() { conceptType = "ConstExprAnnotation"; }
};

// Semantic annotations for AI guidance (Sprint 10, Step 266)

class IntentAnnotation : public Annotation {
public:
    std::string summary;   // 1-sentence description of what/why
    std::string category;  // "validation", "transformation", "io",
                           // "coordination", "computation", "initialization"
    IntentAnnotation() { conceptType = "IntentAnnotation"; }
};

class ComplexityAnnotation : public Annotation {
public:
    std::string timeComplexity;    // "O(1)", "O(n)", "O(n^2)", etc.
    int cognitiveComplexity = 0;   // 1-10 scale
    int linesOfLogic = 0;
    ComplexityAnnotation() { conceptType = "ComplexityAnnotation"; }
};

class RiskAnnotation : public Annotation {
public:
    std::string level;          // "low", "medium", "high", "critical"
    std::string reason;
    int dependentCount = 0;     // how many callers/consumers
    RiskAnnotation() { conceptType = "RiskAnnotation"; }
};

class ContractAnnotation : public Annotation {
public:
    std::string preconditions;
    std::string postconditions;
    std::string returnShape;    // human-readable type/shape description
    std::string sideEffects;    // "none", "io", "mutation", "network"
    ContractAnnotation() { conceptType = "ContractAnnotation"; }
};

class SemanticTagAnnotation : public Annotation {
public:
    std::vector<std::string> tags;  // e.g. ["@serialize", "@validation"]
    SemanticTagAnnotation() { conceptType = "SemanticTagAnnotation"; }
};

// Type System Annotations — Layout & Constraints (Step 272, Subject 2)

class BitWidthAnnotation : public Annotation {
public:
    int width = 0;  // e.g. 32 for Java int
    BitWidthAnnotation() { conceptType = "BitWidthAnnotation"; }
};

class EndianAnnotation : public Annotation {
public:
    std::string order;  // "big" | "little"
    EndianAnnotation() { conceptType = "EndianAnnotation"; }
};

class LayoutAnnotation : public Annotation {
public:
    std::string mode;     // "packed" | "aligned"
    int alignment = 0;
    LayoutAnnotation() { conceptType = "LayoutAnnotation"; }
};

class NullabilityAnnotation : public Annotation {
public:
    bool nullable = false;
    std::string strategy;  // "strict" | "nullable"
    NullabilityAnnotation() { conceptType = "NullabilityAnnotation"; }
};

class VarianceAnnotation : public Annotation {
public:
    std::string variance;  // "covariant" | "contravariant" | "invariant"
    VarianceAnnotation() { conceptType = "VarianceAnnotation"; }
};

// Type System Annotations — Identity & Mutability (Step 273, Subject 2)

class IdentityAnnotation : public Annotation {
public:
    std::string mode;  // "nominal" | "structural"
    IdentityAnnotation() { conceptType = "IdentityAnnotation"; }
};

class MutAnnotation : public Annotation {
public:
    std::string depth;  // "shallow" | "deep" | "interior"
    MutAnnotation() { conceptType = "MutAnnotation"; }
};

class TypeStateAnnotation : public Annotation {
public:
    std::string state;  // "erased" | "reified"
    TypeStateAnnotation() { conceptType = "TypeStateAnnotation"; }
};

// Concurrency Annotations — Primitives & Memory Model (Step 274, Subject 3)

class AtomicAnnotation : public Annotation {
public:
    std::string consistency;  // "seq_cst" | "relaxed" | "acquire" | "release"
    AtomicAnnotation() { conceptType = "AtomicAnnotation"; }
};

class SyncAnnotation : public Annotation {
public:
    std::string primitive;  // "monitor" | "spin" | "semaphore"
    SyncAnnotation() { conceptType = "SyncAnnotation"; }
};

class ThreadModelAnnotation : public Annotation {
public:
    std::string model;  // "green" | "os" | "fiber"
    ThreadModelAnnotation() { conceptType = "ThreadModelAnnotation"; }
};

class MemoryBarrierAnnotation : public Annotation {
public:
    MemoryBarrierAnnotation() { conceptType = "MemoryBarrierAnnotation"; }
};

// Async, Parallelism & Error Handling Annotations (Step 275, Subject 3)

class ExecAnnotation : public Annotation {
public:
    std::string mode;         // "async" | "event"
    std::string runtimeHint;  // e.g. "tokio", "libuv"
    ExecAnnotation() { conceptType = "ExecAnnotation"; }
};

class BlockingAnnotation : public Annotation {
public:
    std::string kind;  // "io" | "compute"
    BlockingAnnotation() { conceptType = "BlockingAnnotation"; }
};

class ParallelAnnotation : public Annotation {
public:
    std::string kind;  // "data" | "task"
    ParallelAnnotation() { conceptType = "ParallelAnnotation"; }
};

class TrapAnnotation : public Annotation {
public:
    std::string signal;  // e.g. "SIGSEGV", "SIGFPE"
    TrapAnnotation() { conceptType = "TrapAnnotation"; }
};

class ExceptionAnnotation : public Annotation {
public:
    std::string style;  // "checked" | "unchecked"
    ExceptionAnnotation() { conceptType = "ExceptionAnnotation"; }
};

class PanicAnnotation : public Annotation {
public:
    std::string behavior;  // "abort" | "unwind"
    PanicAnnotation() { conceptType = "PanicAnnotation"; }
};

// Scope & Namespace Annotations (Step 276, Subject 4)

class BindingAnnotation : public Annotation {
public:
    std::string time;  // "static" | "dynamic"
    BindingAnnotation() { conceptType = "BindingAnnotation"; }
};

class LookupAnnotation : public Annotation {
public:
    std::string mode;  // "lexical" | "hoisted"
    LookupAnnotation() { conceptType = "LookupAnnotation"; }
};

class CaptureAnnotation : public Annotation {
public:
    std::string strategy;  // "value" | "ref" | "move"
    CaptureAnnotation() { conceptType = "CaptureAnnotation"; }
};

class VisibilityAnnotation : public Annotation {
public:
    std::string level;  // "private" | "internal" | "friend" | "public"
    VisibilityAnnotation() { conceptType = "VisibilityAnnotation"; }
};

class NamespaceAnnotation : public Annotation {
public:
    std::string style;  // "qualified" | "flat"
    NamespaceAnnotation() { conceptType = "NamespaceAnnotation"; }
};

class ScopeAnnotation : public Annotation {
public:
    std::string kind;  // "local" | "global_leaked" | "singleton"
    ScopeAnnotation() { conceptType = "ScopeAnnotation"; }
};

// Shim & Escape Hatch Annotations (Step 278, Subject 5)

class IntrinsicAnnotation : public Annotation {
public:
    std::string instruction;  // e.g. "_mm256_add_ps"
    std::string arch;         // e.g. "x86_avx2"
    IntrinsicAnnotation() { conceptType = "IntrinsicAnnotation"; }
};

class RawAnnotation : public Annotation {
public:
    std::string language;  // target language
    std::string code;      // literal code fragment
    RawAnnotation() { conceptType = "RawAnnotation"; }
};

class CallingConvAnnotation : public Annotation {
public:
    std::string convention;  // "stdcall" | "cdecl" | "fastcall"
    CallingConvAnnotation() { conceptType = "CallingConvAnnotation"; }
};

class LinkAnnotation : public Annotation {
public:
    std::string symbolName;
    std::string library;
    LinkAnnotation() { conceptType = "LinkAnnotation"; }
};

class ShimAnnotation : public Annotation {
public:
    std::string strategy;  // "vtable" | "trampoline" | "union_tag" | "cast"
    ShimAnnotation() { conceptType = "ShimAnnotation"; }
};

class PointerArithmeticAnnotation : public Annotation {
public:
    PointerArithmeticAnnotation() { conceptType = "PointerArithmeticAnnotation"; }
};

class OpaqueAnnotation : public Annotation {
public:
    std::string reason;
    OpaqueAnnotation() { conceptType = "OpaqueAnnotation"; }
};

// Platform & Provenance Annotations (Step 279, Subject 5)

class TargetAnnotation : public Annotation {
public:
    std::string platform;  // e.g. "linux", "windows"
    std::string arch;      // e.g. "x86_64", "arm64"
    TargetAnnotation() { conceptType = "TargetAnnotation"; }
};

class FeatureAnnotation : public Annotation {
public:
    std::string flag;      // e.g. "SSE4_2", "NEON"
    bool enabled = true;
    FeatureAnnotation() { conceptType = "FeatureAnnotation"; }
};

class OriginalAnnotation : public Annotation {
public:
    std::string sourceCode;
    std::string sourceLanguage;
    OriginalAnnotation() { conceptType = "OriginalAnnotation"; }
};

class MappingAnnotation : public Annotation {
public:
    std::vector<std::string> history;  // transformation steps applied
    MappingAnnotation() { conceptType = "MappingAnnotation"; }
};

// Optimization Annotation Completion (Step 280, Subject 6)

class TailCallAnnotation : public Annotation {
public:
    TailCallAnnotation() { conceptType = "TailCallAnnotation"; }
};

class LoopAnnotation : public Annotation {
public:
    std::string hint;  // "unroll" | "vectorize" | "fuse"
    int factor = 0;    // optional unroll/vectorize factor
    LoopAnnotation() { conceptType = "LoopAnnotation"; }
};

class DataAnnotation : public Annotation {
public:
    std::string hint;  // "prefetch" | "restrict"
    DataAnnotation() { conceptType = "DataAnnotation"; }
};

class AlignAnnotation : public Annotation {
public:
    int bytes = 0;
    AlignAnnotation() { conceptType = "AlignAnnotation"; }
};

class PackAnnotation : public Annotation {
public:
    PackAnnotation() { conceptType = "PackAnnotation"; }
};

class BoundsCheckAnnotation : public Annotation {
public:
    bool enabled = true;
    BoundsCheckAnnotation() { conceptType = "BoundsCheckAnnotation"; }
};

class OverflowAnnotation : public Annotation {
public:
    std::string behavior;  // "wrap" | "saturation" | "panic"
    OverflowAnnotation() { conceptType = "OverflowAnnotation"; }
};

// Meta-Programming Annotations (Step 281, Subject 7)

class MetaAnnotation : public Annotation {
public:
    std::string state;  // "quoted" | "unquoted"
    std::string phase;  // "compile" | "runtime"
    MetaAnnotation() { conceptType = "MetaAnnotation"; }
};

class SymbolAnnotation : public Annotation {
public:
    std::string mode;  // "gensym" | "interned"
    SymbolAnnotation() { conceptType = "SymbolAnnotation"; }
};

class EvaluateAnnotation : public Annotation {
public:
    std::string phase;  // "compile_time" | "runtime"
    EvaluateAnnotation() { conceptType = "EvaluateAnnotation"; }
};

class TemplateAnnotation : public Annotation {
public:
    std::string specialization;  // "trait" | "monomorphize" | "erasure"
    TemplateAnnotation() { conceptType = "TemplateAnnotation"; }
};

class SyntheticAnnotation : public Annotation {
public:
    std::string generator;
    bool isStructuralRisk = false;
    SyntheticAnnotation() { conceptType = "SyntheticAnnotation"; }
};

// Strategy & Policy Annotations (Step 282, Subject 8)

class PolicyAnnotation : public Annotation {
public:
    std::string strictness;    // "high" | "low"
    std::string perf;          // "critical" | "normal"
    std::string style;         // "idiomatic" | "literal"
    bool binaryStable = false;
    PolicyAnnotation() { conceptType = "PolicyAnnotation"; }
};

class AmbiguityAnnotation : public Annotation {
public:
    std::string intent;
    std::vector<std::string> options;
    AmbiguityAnnotation() { conceptType = "AmbiguityAnnotation"; }
};

class CandidateAnnotation : public Annotation {
public:
    std::vector<std::string> inferredTypes;
    CandidateAnnotation() { conceptType = "CandidateAnnotation"; }
};

class TradeoffAnnotation : public Annotation {
public:
    std::string reason;
    std::string safetyCost;
    std::string perfCost;
    TradeoffAnnotation() { conceptType = "TradeoffAnnotation"; }
};

class ChoiceAnnotation : public Annotation {
public:
    std::string choiceId;
    std::vector<std::string> options;
    ChoiceAnnotation() { conceptType = "ChoiceAnnotation"; }
};

class DecisionAnnotation : public Annotation {
public:
    std::string choiceId;
    std::string selection;
    std::string author;
    std::string reason;
    DecisionAnnotation() { conceptType = "DecisionAnnotation"; }
};
