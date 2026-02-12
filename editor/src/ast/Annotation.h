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
