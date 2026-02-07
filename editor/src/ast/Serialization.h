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

// --- Deserialization ---

inline ASTNode* createNode(const std::string& conceptName) {
    if (conceptName == "Module") return new Module();
    if (conceptName == "Function") return new Function();
    if (conceptName == "Variable") return new Variable();
    if (conceptName == "Parameter") return new Parameter();
    if (conceptName == "Block") return new Block();
    if (conceptName == "Assignment") return new Assignment();
    if (conceptName == "IfStatement") return new IfStatement();
    if (conceptName == "WhileLoop") return new WhileLoop();
    if (conceptName == "ForLoop") return new ForLoop();
    if (conceptName == "Return") return new Return();
    if (conceptName == "ExpressionStatement") return new ExpressionStatement();
    if (conceptName == "BinaryOperation") return new BinaryOperation();
    if (conceptName == "UnaryOperation") return new UnaryOperation();
    if (conceptName == "FunctionCall") return new FunctionCall();
    if (conceptName == "VariableReference") return new VariableReference();
    if (conceptName == "IntegerLiteral") return new IntegerLiteral();
    if (conceptName == "FloatLiteral") return new FloatLiteral();
    if (conceptName == "StringLiteral") return new StringLiteral();
    if (conceptName == "BooleanLiteral") return new BooleanLiteral();
    if (conceptName == "NullLiteral") return new NullLiteral();
    if (conceptName == "ListLiteral") return new ListLiteral();
    if (conceptName == "IndexAccess") return new IndexAccess();
    if (conceptName == "MemberAccess") return new MemberAccess();
    if (conceptName == "PrimitiveType") return new PrimitiveType();
    if (conceptName == "ListType") return new ListType();
    if (conceptName == "SetType") return new SetType();
    if (conceptName == "MapType") return new MapType();
    if (conceptName == "TupleType") return new TupleType();
    if (conceptName == "ArrayType") return new ArrayType();
    if (conceptName == "OptionalType") return new OptionalType();
    if (conceptName == "CustomType") return new CustomType();
    if (conceptName == "DerefStrategy") return new DerefStrategy();
    if (conceptName == "OptimizationLock") return new OptimizationLock();
    if (conceptName == "LangSpecific") return new LangSpecific();
    return nullptr;
}

inline void setPropertiesFromJson(ASTNode* node, const json& props) {
    const auto& ct = node->conceptType;

    if (ct == "Module") {
        auto* n = static_cast<Module*>(node);
        if (props.contains("name")) n->name = props["name"].get<std::string>();
        if (props.contains("targetLanguage")) n->targetLanguage = props["targetLanguage"].get<std::string>();
    }
    else if (ct == "Function") {
        auto* n = static_cast<Function*>(node);
        if (props.contains("name")) n->name = props["name"].get<std::string>();
    }
    else if (ct == "Variable") {
        auto* n = static_cast<Variable*>(node);
        if (props.contains("name")) n->name = props["name"].get<std::string>();
    }
    else if (ct == "Parameter") {
        auto* n = static_cast<Parameter*>(node);
        if (props.contains("name")) n->name = props["name"].get<std::string>();
    }
    else if (ct == "ForLoop") {
        auto* n = static_cast<ForLoop*>(node);
        if (props.contains("iteratorName")) n->iteratorName = props["iteratorName"].get<std::string>();
    }
    else if (ct == "BinaryOperation") {
        auto* n = static_cast<BinaryOperation*>(node);
        if (props.contains("op")) n->op = props["op"].get<std::string>();
    }
    else if (ct == "UnaryOperation") {
        auto* n = static_cast<UnaryOperation*>(node);
        if (props.contains("op")) n->op = props["op"].get<std::string>();
    }
    else if (ct == "FunctionCall") {
        auto* n = static_cast<FunctionCall*>(node);
        if (props.contains("functionName")) n->functionName = props["functionName"].get<std::string>();
    }
    else if (ct == "VariableReference") {
        auto* n = static_cast<VariableReference*>(node);
        if (props.contains("variableName")) n->variableName = props["variableName"].get<std::string>();
    }
    else if (ct == "IntegerLiteral") {
        auto* n = static_cast<IntegerLiteral*>(node);
        if (props.contains("value")) n->value = props["value"].get<int>();
    }
    else if (ct == "FloatLiteral") {
        auto* n = static_cast<FloatLiteral*>(node);
        if (props.contains("value")) n->value = props["value"].get<std::string>();
    }
    else if (ct == "StringLiteral") {
        auto* n = static_cast<StringLiteral*>(node);
        if (props.contains("value")) n->value = props["value"].get<std::string>();
    }
    else if (ct == "BooleanLiteral") {
        auto* n = static_cast<BooleanLiteral*>(node);
        if (props.contains("value")) n->value = props["value"].get<bool>();
    }
    else if (ct == "MemberAccess") {
        auto* n = static_cast<MemberAccess*>(node);
        if (props.contains("memberName")) n->memberName = props["memberName"].get<std::string>();
    }
    else if (ct == "PrimitiveType") {
        auto* n = static_cast<PrimitiveType*>(node);
        if (props.contains("kind")) n->kind = props["kind"].get<std::string>();
    }
    else if (ct == "CustomType") {
        auto* n = static_cast<CustomType*>(node);
        if (props.contains("typeName")) n->typeName = props["typeName"].get<std::string>();
    }
    else if (ct == "DerefStrategy") {
        auto* n = static_cast<DerefStrategy*>(node);
        if (props.contains("strategy")) n->strategy = props["strategy"].get<std::string>();
        if (props.contains("derefLocation")) n->derefLocation = props["derefLocation"].get<std::string>();
        if (props.contains("owner")) n->owner = props["owner"].get<std::string>();
    }
    else if (ct == "OptimizationLock") {
        auto* n = static_cast<OptimizationLock*>(node);
        if (props.contains("lockedBy")) n->lockedBy = props["lockedBy"].get<std::string>();
        if (props.contains("lockReason")) n->lockReason = props["lockReason"].get<std::string>();
        if (props.contains("lockLevel")) n->lockLevel = props["lockLevel"].get<std::string>();
        if (props.contains("affectedStrategies")) n->affectedStrategies = props["affectedStrategies"].get<std::string>();
        if (props.contains("timestamp")) n->timestamp = props["timestamp"].get<std::string>();
    }
    else if (ct == "LangSpecific") {
        auto* n = static_cast<LangSpecific*>(node);
        if (props.contains("language")) n->language = props["language"].get<std::string>();
        if (props.contains("idiomType")) n->idiomType = props["idiomType"].get<std::string>();
        if (props.contains("rawSyntax")) n->rawSyntax = props["rawSyntax"].get<std::string>();
        if (props.contains("semanticHint")) n->semanticHint = props["semanticHint"].get<std::string>();
        if (props.contains("position")) n->position = props["position"].get<std::string>();
    }
}

inline ASTNode* fromJson(const json& j) {
    ASTNode* node = createNode(j["concept"].get<std::string>());
    if (!node) return nullptr;

    node->id = j["id"].get<std::string>();

    if (j.contains("properties")) {
        setPropertiesFromJson(node, j["properties"]);
    }

    if (j.contains("children")) {
        for (auto& [role, arr] : j["children"].items()) {
            for (auto& childJson : arr) {
                ASTNode* child = fromJson(childJson);
                if (child) node->addChild(role, child);
            }
        }
    }

    return node;
}

inline void deleteTree(ASTNode* node) {
    if (!node) return;
    for (auto* child : node->allChildren()) {
        deleteTree(child);
    }
    delete node;
}
