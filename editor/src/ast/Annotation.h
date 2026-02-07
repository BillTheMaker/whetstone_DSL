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
