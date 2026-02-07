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
