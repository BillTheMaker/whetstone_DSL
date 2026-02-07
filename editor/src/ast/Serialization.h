#pragma once
#include <nlohmann/json.hpp>
#include "ASTNode.h"
#include "Module.h"
#include "Function.h"
#include "Variable.h"
#include "Parameter.h"
#include "Statement.h"
#include "Expression.h"
#include "Type.h"
#include "Annotation.h"

using json = nlohmann::json;

inline json propertiesToJson(const ASTNode* node) {
    json props = json::object();
    const auto& ct = node->conceptType;

    // Module
    if (ct == "Module") {
        auto* n = static_cast<const Module*>(node);
        props["name"] = n->name;
        props["targetLanguage"] = n->targetLanguage;
    }
    // Function
    else if (ct == "Function") {
        auto* n = static_cast<const Function*>(node);
        props["name"] = n->name;
    }
    // Variable
    else if (ct == "Variable") {
        auto* n = static_cast<const Variable*>(node);
        props["name"] = n->name;
    }
    // Parameter
    else if (ct == "Parameter") {
        auto* n = static_cast<const Parameter*>(node);
        props["name"] = n->name;
    }
    // Statements
    else if (ct == "ForLoop") {
        auto* n = static_cast<const ForLoop*>(node);
        props["iteratorName"] = n->iteratorName;
    }
    // Expressions
    else if (ct == "BinaryOperation") {
        auto* n = static_cast<const BinaryOperation*>(node);
        props["op"] = n->op;
    }
    else if (ct == "UnaryOperation") {
        auto* n = static_cast<const UnaryOperation*>(node);
        props["op"] = n->op;
    }
    else if (ct == "FunctionCall") {
        auto* n = static_cast<const FunctionCall*>(node);
        props["functionName"] = n->functionName;
    }
    else if (ct == "VariableReference") {
        auto* n = static_cast<const VariableReference*>(node);
        props["variableName"] = n->variableName;
    }
    else if (ct == "IntegerLiteral") {
        auto* n = static_cast<const IntegerLiteral*>(node);
        props["value"] = n->value;
    }
    else if (ct == "FloatLiteral") {
        auto* n = static_cast<const FloatLiteral*>(node);
        props["value"] = n->value;
    }
    else if (ct == "StringLiteral") {
        auto* n = static_cast<const StringLiteral*>(node);
        props["value"] = n->value;
    }
    else if (ct == "BooleanLiteral") {
        auto* n = static_cast<const BooleanLiteral*>(node);
        props["value"] = n->value;
    }
    else if (ct == "MemberAccess") {
        auto* n = static_cast<const MemberAccess*>(node);
        props["memberName"] = n->memberName;
    }
    // Types
    else if (ct == "PrimitiveType") {
        auto* n = static_cast<const PrimitiveType*>(node);
        props["kind"] = n->kind;
    }
    else if (ct == "CustomType") {
        auto* n = static_cast<const CustomType*>(node);
        props["typeName"] = n->typeName;
    }
    // Annotations
    else if (ct == "DerefStrategy") {
        auto* n = static_cast<const DerefStrategy*>(node);
        props["strategy"] = n->strategy;
        if (!n->derefLocation.empty()) props["derefLocation"] = n->derefLocation;
        if (!n->owner.empty()) props["owner"] = n->owner;
    }
    else if (ct == "OptimizationLock") {
        auto* n = static_cast<const OptimizationLock*>(node);
        props["lockedBy"] = n->lockedBy;
        props["lockReason"] = n->lockReason;
        props["lockLevel"] = n->lockLevel;
        if (!n->affectedStrategies.empty()) props["affectedStrategies"] = n->affectedStrategies;
        if (!n->timestamp.empty()) props["timestamp"] = n->timestamp;
    }
    else if (ct == "LangSpecific") {
        auto* n = static_cast<const LangSpecific*>(node);
        props["language"] = n->language;
        props["idiomType"] = n->idiomType;
        props["rawSyntax"] = n->rawSyntax;
        if (!n->semanticHint.empty()) props["semanticHint"] = n->semanticHint;
        if (!n->position.empty()) props["position"] = n->position;
    }
    // NullLiteral, ListLiteral, IndexAccess, Block, Assignment, IfStatement,
    // WhileLoop, Return, ExpressionStatement, ListType, SetType, MapType,
    // TupleType, ArrayType, OptionalType — no extra properties

    return props;
}

inline json toJson(const ASTNode* node) {
    json j;
    j["id"] = node->id;
    j["concept"] = node->conceptType;
    j["properties"] = propertiesToJson(node);

    json children = json::object();
    for (const auto& role : node->childRoles()) {
        json arr = json::array();
        for (const auto* child : node->getChildren(role)) {
            arr.push_back(toJson(child));
        }
        children[role] = arr;
    }
    j["children"] = children;

    return j;
}
