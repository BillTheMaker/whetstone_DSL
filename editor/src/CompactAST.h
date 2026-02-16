#pragma once
// Step 248: Compact AST response format
//
// Token-efficient AST serialization for agent consumption.
// Provides compact mode, subtree extraction, and AST diff support.

#include "ast/ASTNode.h"
#include "ast/Serialization.h"
#include "ast/Annotation.h"
#include "ast/HostBoundary.h"
#include "ast/SqlNodes.h"
#include "ASTUtils.h"
#include "EnvironmentSpec.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <map>

using json = nlohmann::json;

// --- Extract semantic annotation summary from a node's annotations ---
// Returns a compact JSON object with semantic fields, or null if none exist.
inline json extractSemanticSummary(const ASTNode* node) {
    if (!node) return json();
    auto annos = node->getChildren("annotations");
    if (annos.empty()) return json();
    json sem;
    for (const auto* a : annos) {
        if (a->conceptType == "IntentAnnotation") {
            auto* ia = static_cast<const IntentAnnotation*>(a);
            json obj;
            if (!ia->summary.empty()) obj["summary"] = ia->summary;
            if (!ia->category.empty()) obj["category"] = ia->category;
            if (!obj.empty()) sem["intent"] = obj;
        }
        else if (a->conceptType == "ComplexityAnnotation") {
            auto* ca = static_cast<const ComplexityAnnotation*>(a);
            json obj;
            if (!ca->timeComplexity.empty()) obj["time"] = ca->timeComplexity;
            if (ca->cognitiveComplexity > 0) obj["cognitive"] = ca->cognitiveComplexity;
            if (ca->linesOfLogic > 0) obj["lines"] = ca->linesOfLogic;
            if (!obj.empty()) sem["complexity"] = obj;
        }
        else if (a->conceptType == "RiskAnnotation") {
            auto* ra = static_cast<const RiskAnnotation*>(a);
            json obj;
            if (!ra->level.empty()) obj["level"] = ra->level;
            if (!ra->reason.empty()) obj["reason"] = ra->reason;
            if (ra->dependentCount > 0) obj["dependents"] = ra->dependentCount;
            if (!obj.empty()) sem["risk"] = obj;
        }
        else if (a->conceptType == "ContractAnnotation") {
            auto* ca = static_cast<const ContractAnnotation*>(a);
            json obj;
            if (!ca->preconditions.empty()) obj["pre"] = ca->preconditions;
            if (!ca->postconditions.empty()) obj["post"] = ca->postconditions;
            if (!ca->returnShape.empty()) obj["returns"] = ca->returnShape;
            if (!ca->sideEffects.empty()) obj["sideEffects"] = ca->sideEffects;
            if (!obj.empty()) sem["contract"] = obj;
        }
        else if (a->conceptType == "SemanticTagAnnotation") {
            auto* ta = static_cast<const SemanticTagAnnotation*>(a);
            if (!ta->tags.empty()) sem["tags"] = ta->tags;
        }
        // Type System — Layout & Constraints (Step 272)
        else if (a->conceptType == "BitWidthAnnotation") {
            auto* ba = static_cast<const BitWidthAnnotation*>(a);
            sem["bitWidth"] = ba->width;
        }
        else if (a->conceptType == "EndianAnnotation") {
            auto* ea = static_cast<const EndianAnnotation*>(a);
            if (!ea->order.empty()) sem["endian"] = ea->order;
        }
        else if (a->conceptType == "LayoutAnnotation") {
            auto* la = static_cast<const LayoutAnnotation*>(a);
            json obj;
            if (!la->mode.empty()) obj["mode"] = la->mode;
            if (la->alignment > 0) obj["alignment"] = la->alignment;
            if (!obj.empty()) sem["layout"] = obj;
        }
        else if (a->conceptType == "NullabilityAnnotation") {
            auto* na = static_cast<const NullabilityAnnotation*>(a);
            json obj;
            obj["nullable"] = na->nullable;
            if (!na->strategy.empty()) obj["strategy"] = na->strategy;
            sem["nullability"] = obj;
        }
        else if (a->conceptType == "VarianceAnnotation") {
            auto* va = static_cast<const VarianceAnnotation*>(a);
            if (!va->variance.empty()) sem["variance"] = va->variance;
        }
        // Type System — Identity & Mutability (Step 273)
        else if (a->conceptType == "IdentityAnnotation") {
            auto* ia = static_cast<const IdentityAnnotation*>(a);
            if (!ia->mode.empty()) sem["identity"] = ia->mode;
        }
        else if (a->conceptType == "MutAnnotation") {
            auto* ma = static_cast<const MutAnnotation*>(a);
            if (!ma->depth.empty()) sem["mutability"] = ma->depth;
        }
        else if (a->conceptType == "TypeStateAnnotation") {
            auto* ts = static_cast<const TypeStateAnnotation*>(a);
            if (!ts->state.empty()) sem["typeState"] = ts->state;
        }
        // Concurrency (Step 274)
        else if (a->conceptType == "AtomicAnnotation") {
            auto* aa = static_cast<const AtomicAnnotation*>(a);
            if (!aa->consistency.empty()) sem["atomic"] = aa->consistency;
        }
        else if (a->conceptType == "SyncAnnotation") {
            auto* sa = static_cast<const SyncAnnotation*>(a);
            if (!sa->primitive.empty()) sem["sync"] = sa->primitive;
        }
        else if (a->conceptType == "ThreadModelAnnotation") {
            auto* tm = static_cast<const ThreadModelAnnotation*>(a);
            if (!tm->model.empty()) sem["threadModel"] = tm->model;
        }
        else if (a->conceptType == "MemoryBarrierAnnotation") {
            sem["memoryBarrier"] = true;
        }
        // Async, Parallelism & Error Handling (Step 275)
        else if (a->conceptType == "ExecAnnotation") {
            auto* ea = static_cast<const ExecAnnotation*>(a);
            json obj;
            if (!ea->mode.empty()) obj["mode"] = ea->mode;
            if (!ea->runtimeHint.empty()) obj["runtime"] = ea->runtimeHint;
            if (!obj.empty()) sem["exec"] = obj;
        }
        else if (a->conceptType == "BlockingAnnotation") {
            auto* ba = static_cast<const BlockingAnnotation*>(a);
            if (!ba->kind.empty()) sem["blocking"] = ba->kind;
        }
        else if (a->conceptType == "ParallelAnnotation") {
            auto* pa = static_cast<const ParallelAnnotation*>(a);
            if (!pa->kind.empty()) sem["parallel"] = pa->kind;
        }
        else if (a->conceptType == "TrapAnnotation") {
            auto* ta = static_cast<const TrapAnnotation*>(a);
            if (!ta->signal.empty()) sem["trap"] = ta->signal;
        }
        else if (a->conceptType == "ExceptionAnnotation") {
            auto* ea = static_cast<const ExceptionAnnotation*>(a);
            if (!ea->style.empty()) sem["exception"] = ea->style;
        }
        else if (a->conceptType == "PanicAnnotation") {
            auto* pa = static_cast<const PanicAnnotation*>(a);
            if (!pa->behavior.empty()) sem["panic"] = pa->behavior;
        }
        // Scope & Namespace (Step 276)
        else if (a->conceptType == "BindingAnnotation") {
            auto* ba = static_cast<const BindingAnnotation*>(a);
            if (!ba->time.empty()) sem["binding"] = ba->time;
        }
        else if (a->conceptType == "LookupAnnotation") {
            auto* la = static_cast<const LookupAnnotation*>(a);
            if (!la->mode.empty()) sem["lookup"] = la->mode;
        }
        else if (a->conceptType == "CaptureAnnotation") {
            auto* ca = static_cast<const CaptureAnnotation*>(a);
            if (!ca->strategy.empty()) sem["capture"] = ca->strategy;
        }
        else if (a->conceptType == "VisibilityAnnotation") {
            auto* va = static_cast<const VisibilityAnnotation*>(a);
            if (!va->level.empty()) sem["visibility"] = va->level;
        }
        else if (a->conceptType == "NamespaceAnnotation") {
            auto* na = static_cast<const NamespaceAnnotation*>(a);
            if (!na->style.empty()) sem["namespace"] = na->style;
        }
        else if (a->conceptType == "ScopeAnnotation") {
            auto* sa = static_cast<const ScopeAnnotation*>(a);
            if (!sa->kind.empty()) sem["scope"] = sa->kind;
        }
        // Shim & Escape Hatch (Step 278)
        else if (a->conceptType == "IntrinsicAnnotation") {
            auto* ia = static_cast<const IntrinsicAnnotation*>(a);
            json obj;
            if (!ia->instruction.empty()) obj["instruction"] = ia->instruction;
            if (!ia->arch.empty()) obj["arch"] = ia->arch;
            if (!obj.empty()) sem["intrinsic"] = obj;
        }
        else if (a->conceptType == "RawAnnotation") {
            auto* ra = static_cast<const RawAnnotation*>(a);
            json obj;
            if (!ra->language.empty()) obj["language"] = ra->language;
            if (!ra->code.empty()) obj["code"] = ra->code;
            if (!obj.empty()) sem["raw"] = obj;
        }
        else if (a->conceptType == "CallingConvAnnotation") {
            auto* cc = static_cast<const CallingConvAnnotation*>(a);
            if (!cc->convention.empty()) sem["callingConv"] = cc->convention;
        }
        else if (a->conceptType == "LinkAnnotation") {
            auto* la = static_cast<const LinkAnnotation*>(a);
            json obj;
            if (!la->symbolName.empty()) obj["symbol"] = la->symbolName;
            if (!la->library.empty()) obj["library"] = la->library;
            if (!obj.empty()) sem["link"] = obj;
        }
        else if (a->conceptType == "ShimAnnotation") {
            auto* sa = static_cast<const ShimAnnotation*>(a);
            if (!sa->strategy.empty()) sem["shim"] = sa->strategy;
        }
        else if (a->conceptType == "PointerArithmeticAnnotation") {
            sem["pointerArithmetic"] = true;
        }
        else if (a->conceptType == "OpaqueAnnotation") {
            auto* oa = static_cast<const OpaqueAnnotation*>(a);
            if (!oa->reason.empty()) sem["opaque"] = oa->reason;
        }
        // Platform & Provenance (Step 279)
        else if (a->conceptType == "TargetAnnotation") {
            auto* ta = static_cast<const TargetAnnotation*>(a);
            json obj;
            if (!ta->platform.empty()) obj["platform"] = ta->platform;
            if (!ta->arch.empty()) obj["arch"] = ta->arch;
            if (!obj.empty()) sem["target"] = obj;
        }
        else if (a->conceptType == "FeatureAnnotation") {
            auto* fa = static_cast<const FeatureAnnotation*>(a);
            json obj;
            if (!fa->flag.empty()) obj["flag"] = fa->flag;
            obj["enabled"] = fa->enabled;
            sem["feature"] = obj;
        }
        else if (a->conceptType == "OriginalAnnotation") {
            auto* oa = static_cast<const OriginalAnnotation*>(a);
            json obj;
            if (!oa->sourceLanguage.empty()) obj["lang"] = oa->sourceLanguage;
            if (!oa->sourceCode.empty()) obj["hasSource"] = true;
            if (!obj.empty()) sem["original"] = obj;
        }
        else if (a->conceptType == "MappingAnnotation") {
            auto* ma = static_cast<const MappingAnnotation*>(a);
            if (!ma->history.empty()) sem["mappingSteps"] = (int)ma->history.size();
        }
        // Optimization Completion (Step 280)
        else if (a->conceptType == "TailCallAnnotation") {
            sem["tailCall"] = true;
        }
        else if (a->conceptType == "LoopAnnotation") {
            auto* la = static_cast<const LoopAnnotation*>(a);
            json obj;
            if (!la->hint.empty()) obj["hint"] = la->hint;
            if (la->factor > 0) obj["factor"] = la->factor;
            if (!obj.empty()) sem["loop"] = obj;
        }
        else if (a->conceptType == "DataAnnotation") {
            auto* da = static_cast<const DataAnnotation*>(a);
            if (!da->hint.empty()) sem["data"] = da->hint;
        }
        else if (a->conceptType == "AlignAnnotation") {
            auto* aa = static_cast<const AlignAnnotation*>(a);
            if (aa->bytes > 0) sem["align"] = aa->bytes;
        }
        else if (a->conceptType == "PackAnnotation") {
            sem["pack"] = true;
        }
        else if (a->conceptType == "BoundsCheckAnnotation") {
            auto* bc = static_cast<const BoundsCheckAnnotation*>(a);
            sem["boundsCheck"] = bc->enabled;
        }
        else if (a->conceptType == "OverflowAnnotation") {
            auto* oa = static_cast<const OverflowAnnotation*>(a);
            if (!oa->behavior.empty()) sem["overflow"] = oa->behavior;
        }
        // Meta-Programming (Step 281)
        else if (a->conceptType == "MetaAnnotation") {
            auto* ma = static_cast<const MetaAnnotation*>(a);
            json obj;
            if (!ma->state.empty()) obj["state"] = ma->state;
            if (!ma->phase.empty()) obj["phase"] = ma->phase;
            if (!obj.empty()) sem["meta"] = obj;
        }
        else if (a->conceptType == "SymbolAnnotation") {
            auto* sa = static_cast<const SymbolAnnotation*>(a);
            if (!sa->mode.empty()) sem["symbol"] = sa->mode;
        }
        else if (a->conceptType == "EvaluateAnnotation") {
            auto* ea = static_cast<const EvaluateAnnotation*>(a);
            if (!ea->phase.empty()) sem["evaluate"] = ea->phase;
        }
        else if (a->conceptType == "TemplateAnnotation") {
            auto* ta = static_cast<const TemplateAnnotation*>(a);
            if (!ta->specialization.empty()) sem["template"] = ta->specialization;
        }
        else if (a->conceptType == "SyntheticAnnotation") {
            auto* sa = static_cast<const SyntheticAnnotation*>(a);
            json obj;
            if (!sa->generator.empty()) obj["generator"] = sa->generator;
            obj["risk"] = sa->isStructuralRisk;
            sem["synthetic"] = obj;
        }
        // Strategy & Policy (Step 282)
        else if (a->conceptType == "PolicyAnnotation") {
            auto* pa = static_cast<const PolicyAnnotation*>(a);
            json obj;
            if (!pa->strictness.empty()) obj["strictness"] = pa->strictness;
            if (!pa->perf.empty()) obj["perf"] = pa->perf;
            if (!pa->style.empty()) obj["style"] = pa->style;
            if (!obj.empty()) sem["policy"] = obj;
        }
        else if (a->conceptType == "AmbiguityAnnotation") {
            auto* aa = static_cast<const AmbiguityAnnotation*>(a);
            if (!aa->intent.empty()) sem["ambiguity"] = aa->intent;
        }
        else if (a->conceptType == "CandidateAnnotation") {
            auto* ca = static_cast<const CandidateAnnotation*>(a);
            if (!ca->inferredTypes.empty()) sem["candidates"] = ca->inferredTypes;
        }
        else if (a->conceptType == "TradeoffAnnotation") {
            auto* ta = static_cast<const TradeoffAnnotation*>(a);
            if (!ta->reason.empty()) sem["tradeoff"] = ta->reason;
        }
        else if (a->conceptType == "ChoiceAnnotation") {
            auto* ca = static_cast<const ChoiceAnnotation*>(a);
            if (!ca->choiceId.empty()) sem["choice"] = ca->choiceId;
        }
        else if (a->conceptType == "DecisionAnnotation") {
            auto* da = static_cast<const DecisionAnnotation*>(a);
            json obj;
            if (!da->choiceId.empty()) obj["choiceId"] = da->choiceId;
            if (!da->selection.empty()) obj["selection"] = da->selection;
            if (!obj.empty()) sem["decision"] = obj;
        }
        // Subject 9: Workflow Routing (Step 315)
        else if (a->conceptType == "ContextWidthAnnotation") {
            auto* cw = static_cast<const ContextWidthAnnotation*>(a);
            if (!cw->width.empty()) sem["contextWidth"] = cw->width;
        }
        else if (a->conceptType == "ReviewAnnotation") {
            auto* ra = static_cast<const ReviewAnnotation*>(a);
            json obj;
            obj["required"] = ra->required;
            if (!ra->reviewer.empty()) obj["reviewer"] = ra->reviewer;
            sem["review"] = obj;
        }
        else if (a->conceptType == "AutomatabilityAnnotation") {
            auto* aa = static_cast<const AutomatabilityAnnotation*>(a);
            json obj;
            if (!aa->strategy.empty()) obj["strategy"] = aa->strategy;
            if (aa->confidence > 0.0) obj["confidence"] = aa->confidence;
            sem["automatability"] = obj;
        }
        else if (a->conceptType == "PriorityAnnotation") {
            auto* pa = static_cast<const PriorityAnnotation*>(a);
            if (!pa->level.empty()) sem["priority"] = pa->level;
        }
        else if (a->conceptType == "ImplementationStatusAnnotation") {
            auto* isa = static_cast<const ImplementationStatusAnnotation*>(a);
            if (!isa->status.empty()) sem["implStatus"] = isa->status;
        }
        // Environment Layer (Step 285)
        else if (a->conceptType == "CapabilityRequirement") {
            auto* cr = static_cast<const CapabilityRequirement*>(a);
            json obj;
            if (!cr->capability.empty()) obj["capability"] = cr->capability;
            obj["required"] = cr->required;
            sem["capabilityReq"] = obj;
        }
    }
    return sem.empty() ? json() : sem;
}

// --- Extract a human-readable name from any AST node ---
inline std::string getNodeName(const ASTNode* node) {
    if (!node) return "";
    const auto& ct = node->conceptType;
    if (ct == "Module")
        return static_cast<const Module*>(node)->name;
    if (ct == "Function")
        return static_cast<const Function*>(node)->name;
    if (ct == "Variable")
        return static_cast<const Variable*>(node)->name;
    if (ct == "Parameter")
        return static_cast<const Parameter*>(node)->name;
    if (ct == "FunctionCall")
        return static_cast<const FunctionCall*>(node)->functionName;
    if (ct == "VariableReference")
        return static_cast<const VariableReference*>(node)->variableName;
    if (ct == "BinaryOperation")
        return static_cast<const BinaryOperation*>(node)->op;
    if (ct == "UnaryOperation")
        return static_cast<const UnaryOperation*>(node)->op;
    if (ct == "StringLiteral")
        return static_cast<const StringLiteral*>(node)->value;
    if (ct == "Import")
        return static_cast<const Import*>(node)->moduleName;
    if (ct == "ExternalModule")
        return static_cast<const ExternalModule*>(node)->name;
    if (ct == "PrimitiveType")
        return static_cast<const PrimitiveType*>(node)->kind;
    if (ct == "CustomType")
        return static_cast<const CustomType*>(node)->typeName;
    if (ct == "MemberAccess")
        return static_cast<const MemberAccess*>(node)->memberName;
    if (ct == "TypeSignature")
        return static_cast<const TypeSignature*>(node)->name;
    if (ct == "ForLoop")
        return static_cast<const ForLoop*>(node)->iteratorName;
    if (ct == "IntegerLiteral")
        return std::to_string(
            static_cast<const IntegerLiteral*>(node)->value);
    if (ct == "BooleanLiteral")
        return static_cast<const BooleanLiteral*>(node)->value
            ? "true" : "false";
    // Class declarations (Sprint 11c + 12c)
    if (ct == "ClassDeclaration")
        return static_cast<const ClassDeclaration*>(node)->name;
    if (ct == "InterfaceDeclaration")
        return static_cast<const InterfaceDeclaration*>(node)->name;
    if (ct == "MethodDeclaration")
        return static_cast<const MethodDeclaration*>(node)->name;
    // Generic types (Sprint 12c)
    if (ct == "GenericType") {
        auto* g = static_cast<const GenericType*>(node);
        return g->isClassTemplate ? ("template:" + g->baseName) : g->baseName;
    }
    if (ct == "TypeParameter")
        return static_cast<const TypeParameter*>(node)->name;
    // Host Boundary (Step 288)
    if (ct == "HostCall")
        return static_cast<const HostCall*>(node)->name;
    if (ct == "ModuleLoad")
        return static_cast<const ModuleLoad*>(node)->moduleName;
    if (ct == "ScheduleTask")
        return static_cast<const ScheduleTask*>(node)->queue;
    // Preprocessor nodes (Step 337)
    if (ct == "IncludeDirective")
        return static_cast<const IncludeDirective*>(node)->path;
    if (ct == "PragmaDirective")
        return static_cast<const PragmaDirective*>(node)->directive;
    if (ct == "MacroDefinition")
        return static_cast<const MacroDefinition*>(node)->name;
    if (ct == "EnumDeclaration")
        return static_cast<const EnumDeclaration*>(node)->name;
    if (ct == "EnumMember")
        return static_cast<const EnumMember*>(node)->name;
    if (ct == "NamespaceDeclaration")
        return static_cast<const NamespaceDeclaration*>(node)->name;
    if (ct == "TypeAlias")
        return static_cast<const TypeAlias*>(node)->aliasName;
    if (ct == "TableDeclaration")
        return static_cast<const TableDeclaration*>(node)->name;
    if (ct == "ColumnDefinition")
        return static_cast<const ColumnDefinition*>(node)->name;
    if (ct == "SelectQuery")
        return "select";
    if (ct == "InsertStatement")
        return static_cast<const InsertStatement*>(node)->tableName;
    if (ct == "UpdateStatement")
        return static_cast<const UpdateStatement*>(node)->tableName;
    if (ct == "DeleteStatement")
        return static_cast<const DeleteStatement*>(node)->tableName;
    if (ct == "JoinClause")
        return static_cast<const JoinClause*>(node)->tableName;
    if (ct == "WhereClause")
        return "where";
    if (ct == "IndexDefinition")
        return static_cast<const IndexDefinition*>(node)->name;
    return "";
}

// --- Compact AST serialization ---
// Returns: {id, type, name, line, childCount, children: [child_ids]}
// Uses short keys and omits empty fields for minimal token usage.
inline json toJsonCompact(const ASTNode* node) {
    if (!node) return json();
    json j;
    j["id"] = node->id;
    j["type"] = node->conceptType;
    std::string name = getNodeName(node);
    if (!name.empty()) j["name"] = name;
    if (node->hasSpan()) j["line"] = node->spanStartLine;

    // Include semantic summary if annotations exist
    json sem = extractSemanticSummary(node);
    if (!sem.empty()) j["semantic"] = sem;

    auto kids = node->allChildren();
    if (!kids.empty()) {
        j["childCount"] = (int)kids.size();
        json childIds = json::array();
        for (const auto* child : kids)
            childIds.push_back(child->id);
        j["children"] = childIds;
    }

    return j;
}

// Compact summary: top-level nodes only (Module + direct children).
// Deeper nodes omitted — use getASTSubtree for detail.
inline json toJsonCompactSummary(const ASTNode* root) {
    if (!root) return json::array();
    json nodes = json::array();
    // Root node
    json rootJ;
    rootJ["id"] = root->id;
    rootJ["type"] = root->conceptType;
    std::string rname = getNodeName(root);
    if (!rname.empty()) rootJ["name"] = rname;
    if (root->hasSpan()) rootJ["line"] = root->spanStartLine;
    auto rootKids = root->allChildren();
    if (!rootKids.empty()) rootJ["childCount"] = (int)rootKids.size();
    nodes.push_back(rootJ);
    // Direct children (depth 1 only — functions, imports, etc.)
    for (const auto* child : rootKids) {
        json cj;
        cj["id"] = child->id;
        cj["type"] = child->conceptType;
        std::string cname = getNodeName(child);
        if (!cname.empty()) cj["name"] = cname;
        if (child->hasSpan()) cj["line"] = child->spanStartLine;
        json csem = extractSemanticSummary(child);
        if (!csem.empty()) cj["semantic"] = csem;
        auto grandkids = child->allChildren();
        if (!grandkids.empty())
            cj["childCount"] = (int)grandkids.size();
        nodes.push_back(cj);
    }
    return nodes;
}

// Collect all nodes in compact format (flat list)
inline json toJsonCompactTree(const ASTNode* node) {
    if (!node) return json::array();
    json nodes = json::array();
    nodes.push_back(toJsonCompact(node));
    for (const auto* child : node->allChildren()) {
        json childNodes = toJsonCompactTree(child);
        for (auto& cn : childNodes)
            nodes.push_back(std::move(cn));
    }
    return nodes;
}

// --- Subtree extraction ---
// Returns full JSON for the subtree rooted at nodeId
inline json toJsonSubtree(ASTNode* root, const std::string& nodeId) {
    ASTNode* target = findNodeById(root, nodeId);
    if (!target) return json();
    return toJson(target);
}

// --- Token estimate ---
// Rough estimate: characters / 4 (approximates LLM tokens)
inline int tokenEstimate(const json& j) {
    std::string s = j.dump();
    return (int)s.size() / 4;
}

// --- AST version tracking ---
// Stored in HeadlessBufferState, records which node IDs changed per version.
struct ASTVersionTracker {
    int version = 0;
    // version -> list of affected node IDs
    std::map<int, std::vector<std::string>> changes;

    void recordMutation(const std::vector<std::string>& affectedIds) {
        ++version;
        changes[version] = affectedIds;
    }

    // Get all node IDs that changed since a given version
    std::vector<std::string> changedSince(int sinceVersion) const {
        std::vector<std::string> result;
        for (const auto& [v, ids] : changes) {
            if (v > sinceVersion) {
                for (const auto& id : ids)
                    result.push_back(id);
            }
        }
        return result;
    }

    // Build a diff response: full JSON for changed nodes only
    json buildDiff(ASTNode* root, int sinceVersion) const {
        auto changedIds = changedSince(sinceVersion);
        json nodes = json::array();
        for (const auto& id : changedIds) {
            ASTNode* node = findNodeById(root, id);
            if (node)
                nodes.push_back(toJson(node));
        }
        return {
            {"sinceVersion", sinceVersion},
            {"currentVersion", version},
            {"changedCount", (int)nodes.size()},
            {"nodes", nodes}
        };
    }

    // Prune old entries to prevent unbounded growth
    void pruneOlderThan(int keepVersion) {
        auto it = changes.begin();
        while (it != changes.end() && it->first < keepVersion)
            it = changes.erase(it);
    }
};
