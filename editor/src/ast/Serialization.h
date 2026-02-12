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
#include "Import.h"
#include "ExternalModule.h"
#include "TypeSignature.h"

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
    // Imports / External modules
    else if (ct == "Import") {
        auto* n = static_cast<const Import*>(node);
        props["moduleName"] = n->moduleName;
        if (!n->alias.empty()) props["alias"] = n->alias;
        if (!n->importKind.empty()) props["importKind"] = n->importKind;
    }
    else if (ct == "ExternalModule") {
        auto* n = static_cast<const ExternalModule*>(node);
        props["name"] = n->name;
        if (!n->version.empty()) props["version"] = n->version;
        if (!n->language.empty()) props["language"] = n->language;
        if (!n->semanticTags.empty()) props["semanticTags"] = n->semanticTags;
    }
    else if (ct == "TypeSignature") {
        auto* n = static_cast<const TypeSignature*>(node);
        props["name"] = n->name;
        props["variadic"] = n->variadic;
        if (!n->semanticTags.empty()) props["semanticTags"] = n->semanticTags;
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
    // Semantic annotations (Sprint 10)
    else if (ct == "IntentAnnotation") {
        auto* n = static_cast<const IntentAnnotation*>(node);
        if (!n->summary.empty()) props["summary"] = n->summary;
        if (!n->category.empty()) props["category"] = n->category;
    }
    else if (ct == "ComplexityAnnotation") {
        auto* n = static_cast<const ComplexityAnnotation*>(node);
        if (!n->timeComplexity.empty()) props["timeComplexity"] = n->timeComplexity;
        props["cognitiveComplexity"] = n->cognitiveComplexity;
        props["linesOfLogic"] = n->linesOfLogic;
    }
    else if (ct == "RiskAnnotation") {
        auto* n = static_cast<const RiskAnnotation*>(node);
        if (!n->level.empty()) props["level"] = n->level;
        if (!n->reason.empty()) props["reason"] = n->reason;
        props["dependentCount"] = n->dependentCount;
    }
    else if (ct == "ContractAnnotation") {
        auto* n = static_cast<const ContractAnnotation*>(node);
        if (!n->preconditions.empty()) props["preconditions"] = n->preconditions;
        if (!n->postconditions.empty()) props["postconditions"] = n->postconditions;
        if (!n->returnShape.empty()) props["returnShape"] = n->returnShape;
        if (!n->sideEffects.empty()) props["sideEffects"] = n->sideEffects;
    }
    else if (ct == "SemanticTagAnnotation") {
        auto* n = static_cast<const SemanticTagAnnotation*>(node);
        if (!n->tags.empty()) props["tags"] = n->tags;
    }
    // Type System — Layout & Constraints (Step 272)
    else if (ct == "BitWidthAnnotation") {
        auto* n = static_cast<const BitWidthAnnotation*>(node);
        props["width"] = n->width;
    }
    else if (ct == "EndianAnnotation") {
        auto* n = static_cast<const EndianAnnotation*>(node);
        if (!n->order.empty()) props["order"] = n->order;
    }
    else if (ct == "LayoutAnnotation") {
        auto* n = static_cast<const LayoutAnnotation*>(node);
        if (!n->mode.empty()) props["mode"] = n->mode;
        props["alignment"] = n->alignment;
    }
    else if (ct == "NullabilityAnnotation") {
        auto* n = static_cast<const NullabilityAnnotation*>(node);
        props["nullable"] = n->nullable;
        if (!n->strategy.empty()) props["strategy"] = n->strategy;
    }
    else if (ct == "VarianceAnnotation") {
        auto* n = static_cast<const VarianceAnnotation*>(node);
        if (!n->variance.empty()) props["variance"] = n->variance;
    }
    // Type System — Identity & Mutability (Step 273)
    else if (ct == "IdentityAnnotation") {
        auto* n = static_cast<const IdentityAnnotation*>(node);
        if (!n->mode.empty()) props["mode"] = n->mode;
    }
    else if (ct == "MutAnnotation") {
        auto* n = static_cast<const MutAnnotation*>(node);
        if (!n->depth.empty()) props["depth"] = n->depth;
    }
    else if (ct == "TypeStateAnnotation") {
        auto* n = static_cast<const TypeStateAnnotation*>(node);
        if (!n->state.empty()) props["state"] = n->state;
    }
    // Concurrency — Primitives & Memory Model (Step 274)
    else if (ct == "AtomicAnnotation") {
        auto* n = static_cast<const AtomicAnnotation*>(node);
        if (!n->consistency.empty()) props["consistency"] = n->consistency;
    }
    else if (ct == "SyncAnnotation") {
        auto* n = static_cast<const SyncAnnotation*>(node);
        if (!n->primitive.empty()) props["primitive"] = n->primitive;
    }
    else if (ct == "ThreadModelAnnotation") {
        auto* n = static_cast<const ThreadModelAnnotation*>(node);
        if (!n->model.empty()) props["model"] = n->model;
    }
    // MemoryBarrierAnnotation has no extra fields
    // Async, Parallelism & Error Handling (Step 275)
    else if (ct == "ExecAnnotation") {
        auto* n = static_cast<const ExecAnnotation*>(node);
        if (!n->mode.empty()) props["mode"] = n->mode;
        if (!n->runtimeHint.empty()) props["runtimeHint"] = n->runtimeHint;
    }
    else if (ct == "BlockingAnnotation") {
        auto* n = static_cast<const BlockingAnnotation*>(node);
        if (!n->kind.empty()) props["kind"] = n->kind;
    }
    else if (ct == "ParallelAnnotation") {
        auto* n = static_cast<const ParallelAnnotation*>(node);
        if (!n->kind.empty()) props["kind"] = n->kind;
    }
    else if (ct == "TrapAnnotation") {
        auto* n = static_cast<const TrapAnnotation*>(node);
        if (!n->signal.empty()) props["signal"] = n->signal;
    }
    else if (ct == "ExceptionAnnotation") {
        auto* n = static_cast<const ExceptionAnnotation*>(node);
        if (!n->style.empty()) props["style"] = n->style;
    }
    else if (ct == "PanicAnnotation") {
        auto* n = static_cast<const PanicAnnotation*>(node);
        if (!n->behavior.empty()) props["behavior"] = n->behavior;
    }
    // Scope & Namespace (Step 276)
    else if (ct == "BindingAnnotation") {
        auto* n = static_cast<const BindingAnnotation*>(node);
        if (!n->time.empty()) props["time"] = n->time;
    }
    else if (ct == "LookupAnnotation") {
        auto* n = static_cast<const LookupAnnotation*>(node);
        if (!n->mode.empty()) props["mode"] = n->mode;
    }
    else if (ct == "CaptureAnnotation") {
        auto* n = static_cast<const CaptureAnnotation*>(node);
        if (!n->strategy.empty()) props["strategy"] = n->strategy;
    }
    else if (ct == "VisibilityAnnotation") {
        auto* n = static_cast<const VisibilityAnnotation*>(node);
        if (!n->level.empty()) props["level"] = n->level;
    }
    else if (ct == "NamespaceAnnotation") {
        auto* n = static_cast<const NamespaceAnnotation*>(node);
        if (!n->style.empty()) props["style"] = n->style;
    }
    else if (ct == "ScopeAnnotation") {
        auto* n = static_cast<const ScopeAnnotation*>(node);
        if (!n->kind.empty()) props["kind"] = n->kind;
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
    if (node->hasSpan()) {
        j["span"] = {
            {"start", {{"line", node->spanStartLine}, {"col", node->spanStartCol}}},
            {"end", {{"line", node->spanEndLine}, {"col", node->spanEndCol}}}
        };
    }

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
    if (conceptName == "Import") return new Import();
    if (conceptName == "ExternalModule") return new ExternalModule();
    if (conceptName == "TypeSignature") return new TypeSignature();
    if (conceptName == "DerefStrategy") return new DerefStrategy();
    if (conceptName == "OptimizationLock") return new OptimizationLock();
    if (conceptName == "LangSpecific") return new LangSpecific();
    if (conceptName == "IntentAnnotation") return new IntentAnnotation();
    if (conceptName == "ComplexityAnnotation") return new ComplexityAnnotation();
    if (conceptName == "RiskAnnotation") return new RiskAnnotation();
    if (conceptName == "ContractAnnotation") return new ContractAnnotation();
    if (conceptName == "SemanticTagAnnotation") return new SemanticTagAnnotation();
    // Type System — Layout & Constraints (Step 272)
    if (conceptName == "BitWidthAnnotation") return new BitWidthAnnotation();
    if (conceptName == "EndianAnnotation") return new EndianAnnotation();
    if (conceptName == "LayoutAnnotation") return new LayoutAnnotation();
    if (conceptName == "NullabilityAnnotation") return new NullabilityAnnotation();
    if (conceptName == "VarianceAnnotation") return new VarianceAnnotation();
    // Type System — Identity & Mutability (Step 273)
    if (conceptName == "IdentityAnnotation") return new IdentityAnnotation();
    if (conceptName == "MutAnnotation") return new MutAnnotation();
    if (conceptName == "TypeStateAnnotation") return new TypeStateAnnotation();
    // Concurrency (Step 274)
    if (conceptName == "AtomicAnnotation") return new AtomicAnnotation();
    if (conceptName == "SyncAnnotation") return new SyncAnnotation();
    if (conceptName == "ThreadModelAnnotation") return new ThreadModelAnnotation();
    if (conceptName == "MemoryBarrierAnnotation") return new MemoryBarrierAnnotation();
    // Async, Parallelism & Error Handling (Step 275)
    if (conceptName == "ExecAnnotation") return new ExecAnnotation();
    if (conceptName == "BlockingAnnotation") return new BlockingAnnotation();
    if (conceptName == "ParallelAnnotation") return new ParallelAnnotation();
    if (conceptName == "TrapAnnotation") return new TrapAnnotation();
    if (conceptName == "ExceptionAnnotation") return new ExceptionAnnotation();
    if (conceptName == "PanicAnnotation") return new PanicAnnotation();
    // Scope & Namespace (Step 276)
    if (conceptName == "BindingAnnotation") return new BindingAnnotation();
    if (conceptName == "LookupAnnotation") return new LookupAnnotation();
    if (conceptName == "CaptureAnnotation") return new CaptureAnnotation();
    if (conceptName == "VisibilityAnnotation") return new VisibilityAnnotation();
    if (conceptName == "NamespaceAnnotation") return new NamespaceAnnotation();
    if (conceptName == "ScopeAnnotation") return new ScopeAnnotation();
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
    else if (ct == "Import") {
        auto* n = static_cast<Import*>(node);
        if (props.contains("moduleName")) n->moduleName = props["moduleName"].get<std::string>();
        if (props.contains("alias")) n->alias = props["alias"].get<std::string>();
        if (props.contains("importKind")) n->importKind = props["importKind"].get<std::string>();
    }
    else if (ct == "ExternalModule") {
        auto* n = static_cast<ExternalModule*>(node);
        if (props.contains("name")) n->name = props["name"].get<std::string>();
        if (props.contains("version")) n->version = props["version"].get<std::string>();
        if (props.contains("language")) n->language = props["language"].get<std::string>();
        if (props.contains("semanticTags") && props["semanticTags"].is_array()) {
            n->semanticTags.clear();
            for (const auto& tag : props["semanticTags"]) {
                if (tag.is_string()) n->semanticTags.push_back(tag.get<std::string>());
            }
        }
    }
    else if (ct == "TypeSignature") {
        auto* n = static_cast<TypeSignature*>(node);
        if (props.contains("name")) n->name = props["name"].get<std::string>();
        if (props.contains("variadic")) n->variadic = props["variadic"].get<bool>();
        if (props.contains("semanticTags") && props["semanticTags"].is_array()) {
            n->semanticTags.clear();
            for (const auto& tag : props["semanticTags"]) {
                if (tag.is_string()) n->semanticTags.push_back(tag.get<std::string>());
            }
        }
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
    // Semantic annotations (Sprint 10)
    else if (ct == "IntentAnnotation") {
        auto* n = static_cast<IntentAnnotation*>(node);
        if (props.contains("summary")) n->summary = props["summary"].get<std::string>();
        if (props.contains("category")) n->category = props["category"].get<std::string>();
    }
    else if (ct == "ComplexityAnnotation") {
        auto* n = static_cast<ComplexityAnnotation*>(node);
        if (props.contains("timeComplexity")) n->timeComplexity = props["timeComplexity"].get<std::string>();
        if (props.contains("cognitiveComplexity")) n->cognitiveComplexity = props["cognitiveComplexity"].get<int>();
        if (props.contains("linesOfLogic")) n->linesOfLogic = props["linesOfLogic"].get<int>();
    }
    else if (ct == "RiskAnnotation") {
        auto* n = static_cast<RiskAnnotation*>(node);
        if (props.contains("level")) n->level = props["level"].get<std::string>();
        if (props.contains("reason")) n->reason = props["reason"].get<std::string>();
        if (props.contains("dependentCount")) n->dependentCount = props["dependentCount"].get<int>();
    }
    else if (ct == "ContractAnnotation") {
        auto* n = static_cast<ContractAnnotation*>(node);
        if (props.contains("preconditions")) n->preconditions = props["preconditions"].get<std::string>();
        if (props.contains("postconditions")) n->postconditions = props["postconditions"].get<std::string>();
        if (props.contains("returnShape")) n->returnShape = props["returnShape"].get<std::string>();
        if (props.contains("sideEffects")) n->sideEffects = props["sideEffects"].get<std::string>();
    }
    else if (ct == "SemanticTagAnnotation") {
        auto* n = static_cast<SemanticTagAnnotation*>(node);
        if (props.contains("tags") && props["tags"].is_array()) {
            n->tags.clear();
            for (const auto& tag : props["tags"]) {
                if (tag.is_string()) n->tags.push_back(tag.get<std::string>());
            }
        }
    }
    // Type System — Layout & Constraints (Step 272)
    else if (ct == "BitWidthAnnotation") {
        auto* n = static_cast<BitWidthAnnotation*>(node);
        if (props.contains("width")) n->width = props["width"].get<int>();
    }
    else if (ct == "EndianAnnotation") {
        auto* n = static_cast<EndianAnnotation*>(node);
        if (props.contains("order")) n->order = props["order"].get<std::string>();
    }
    else if (ct == "LayoutAnnotation") {
        auto* n = static_cast<LayoutAnnotation*>(node);
        if (props.contains("mode")) n->mode = props["mode"].get<std::string>();
        if (props.contains("alignment")) n->alignment = props["alignment"].get<int>();
    }
    else if (ct == "NullabilityAnnotation") {
        auto* n = static_cast<NullabilityAnnotation*>(node);
        if (props.contains("nullable")) n->nullable = props["nullable"].get<bool>();
        if (props.contains("strategy")) n->strategy = props["strategy"].get<std::string>();
    }
    else if (ct == "VarianceAnnotation") {
        auto* n = static_cast<VarianceAnnotation*>(node);
        if (props.contains("variance")) n->variance = props["variance"].get<std::string>();
    }
    // Type System — Identity & Mutability (Step 273)
    else if (ct == "IdentityAnnotation") {
        auto* n = static_cast<IdentityAnnotation*>(node);
        if (props.contains("mode")) n->mode = props["mode"].get<std::string>();
    }
    else if (ct == "MutAnnotation") {
        auto* n = static_cast<MutAnnotation*>(node);
        if (props.contains("depth")) n->depth = props["depth"].get<std::string>();
    }
    else if (ct == "TypeStateAnnotation") {
        auto* n = static_cast<TypeStateAnnotation*>(node);
        if (props.contains("state")) n->state = props["state"].get<std::string>();
    }
    // Concurrency (Step 274)
    else if (ct == "AtomicAnnotation") {
        auto* n = static_cast<AtomicAnnotation*>(node);
        if (props.contains("consistency")) n->consistency = props["consistency"].get<std::string>();
    }
    else if (ct == "SyncAnnotation") {
        auto* n = static_cast<SyncAnnotation*>(node);
        if (props.contains("primitive")) n->primitive = props["primitive"].get<std::string>();
    }
    else if (ct == "ThreadModelAnnotation") {
        auto* n = static_cast<ThreadModelAnnotation*>(node);
        if (props.contains("model")) n->model = props["model"].get<std::string>();
    }
    // MemoryBarrierAnnotation has no extra fields
    // Async, Parallelism & Error Handling (Step 275)
    else if (ct == "ExecAnnotation") {
        auto* n = static_cast<ExecAnnotation*>(node);
        if (props.contains("mode")) n->mode = props["mode"].get<std::string>();
        if (props.contains("runtimeHint")) n->runtimeHint = props["runtimeHint"].get<std::string>();
    }
    else if (ct == "BlockingAnnotation") {
        auto* n = static_cast<BlockingAnnotation*>(node);
        if (props.contains("kind")) n->kind = props["kind"].get<std::string>();
    }
    else if (ct == "ParallelAnnotation") {
        auto* n = static_cast<ParallelAnnotation*>(node);
        if (props.contains("kind")) n->kind = props["kind"].get<std::string>();
    }
    else if (ct == "TrapAnnotation") {
        auto* n = static_cast<TrapAnnotation*>(node);
        if (props.contains("signal")) n->signal = props["signal"].get<std::string>();
    }
    else if (ct == "ExceptionAnnotation") {
        auto* n = static_cast<ExceptionAnnotation*>(node);
        if (props.contains("style")) n->style = props["style"].get<std::string>();
    }
    else if (ct == "PanicAnnotation") {
        auto* n = static_cast<PanicAnnotation*>(node);
        if (props.contains("behavior")) n->behavior = props["behavior"].get<std::string>();
    }
    // Scope & Namespace (Step 276)
    else if (ct == "BindingAnnotation") {
        auto* n = static_cast<BindingAnnotation*>(node);
        if (props.contains("time")) n->time = props["time"].get<std::string>();
    }
    else if (ct == "LookupAnnotation") {
        auto* n = static_cast<LookupAnnotation*>(node);
        if (props.contains("mode")) n->mode = props["mode"].get<std::string>();
    }
    else if (ct == "CaptureAnnotation") {
        auto* n = static_cast<CaptureAnnotation*>(node);
        if (props.contains("strategy")) n->strategy = props["strategy"].get<std::string>();
    }
    else if (ct == "VisibilityAnnotation") {
        auto* n = static_cast<VisibilityAnnotation*>(node);
        if (props.contains("level")) n->level = props["level"].get<std::string>();
    }
    else if (ct == "NamespaceAnnotation") {
        auto* n = static_cast<NamespaceAnnotation*>(node);
        if (props.contains("style")) n->style = props["style"].get<std::string>();
    }
    else if (ct == "ScopeAnnotation") {
        auto* n = static_cast<ScopeAnnotation*>(node);
        if (props.contains("kind")) n->kind = props["kind"].get<std::string>();
    }
}

inline std::string generateNodeId() {
    static int counter = 0;
    return "node_" + std::to_string(++counter);
}

inline ASTNode* fromJson(const json& j) {
    ASTNode* node = createNode(j["concept"].get<std::string>());
    if (!node) return nullptr;

    node->id = j.contains("id") ? j["id"].get<std::string>()
                                 : generateNodeId();
    if (j.contains("span")) {
        const auto& span = j["span"];
        if (span.contains("start") && span.contains("end")) {
            const auto& s = span["start"];
            const auto& e = span["end"];
            if (s.contains("line") && s.contains("col") && e.contains("line") && e.contains("col")) {
                node->setSpan(s["line"].get<int>(), s["col"].get<int>(),
                              e["line"].get<int>(), e["col"].get<int>());
            }
        }
    }

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
