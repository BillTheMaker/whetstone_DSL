#pragma once
// SemannoFormat.h — Semanno inline comment format standard
//
// Format:  @semanno:type(key=value,key2=value2)
//
// SemannoEmitter  — converts annotation ASTNodes to Semanno comment strings
// SemannoParser   — parses Semanno comment strings back to structured data
//
// Covers all 67+ annotation types defined in Annotation.h and EnvironmentSpec.h.

#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "EnvironmentSpec.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace semanno_detail {

inline std::string escapeValue(const std::string& v) {
    std::string out;
    out.reserve(v.size() + 2);
    for (char c : v) {
        if (c == '"')       out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else                out += c;
    }
    return out;
}

inline std::string unescapeValue(const std::string& v) {
    std::string out;
    out.reserve(v.size());
    for (size_t i = 0; i < v.size(); ++i) {
        if (v[i] == '\\' && i + 1 < v.size()) {
            if (v[i + 1] == '"')       { out += '"';  ++i; }
            else if (v[i + 1] == '\\') { out += '\\'; ++i; }
            else                        out += v[i];
        } else {
            out += v[i];
        }
    }
    return out;
}

inline std::string joinVec(const std::vector<std::string>& vec) {
    std::string out;
    for (size_t i = 0; i < vec.size(); ++i) {
        if (i > 0) out += ";";
        out += escapeValue(vec[i]);
    }
    return out;
}

inline std::vector<std::string> splitVec(const std::string& s) {
    std::vector<std::string> out;
    if (s.empty()) return out;
    std::string cur;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            cur += s[i];
            cur += s[i + 1];
            ++i;
        } else if (s[i] == ';') {
            out.push_back(unescapeValue(cur));
            cur.clear();
        } else {
            cur += s[i];
        }
    }
    out.push_back(unescapeValue(cur));
    return out;
}

// Build "key=\"value\"" pair; omits the pair entirely when value is empty.
inline void appendProp(std::string& buf, const std::string& key,
                       const std::string& val, bool& first) {
    if (val.empty()) return;
    if (!first) buf += ",";
    first = false;
    buf += key + "=\"" + escapeValue(val) + "\"";
}

inline void appendInt(std::string& buf, const std::string& key,
                      int val, bool& first) {
    if (!first) buf += ",";
    first = false;
    buf += key + "=" + std::to_string(val);
}

inline void appendBool(std::string& buf, const std::string& key,
                       bool val, bool& first) {
    if (!first) buf += ",";
    first = false;
    buf += key + "=" + (val ? "true" : "false");
}

inline void appendVec(std::string& buf, const std::string& key,
                      const std::vector<std::string>& vec, bool& first) {
    if (vec.empty()) return;
    if (!first) buf += ",";
    first = false;
    buf += key + "=\"" + joinVec(vec) + "\"";
}

} // namespace semanno_detail

// ---------------------------------------------------------------------------
// SemannoEmitter
// ---------------------------------------------------------------------------

class SemannoEmitter {
public:
    /// Returns "@semanno:type(key=value,...)" or empty string if unrecognised.
    static std::string emit(const ASTNode* anno) {
        if (!anno) return "";
        const std::string& ct = anno->conceptType;

        std::string tag;
        std::string props;
        bool first = true;

        using namespace semanno_detail;

        // --- Subject 1: Memory ---
        if (ct == "DeallocateAnnotation") {
            tag = "deallocate";
            auto* a = static_cast<const DeallocateAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
            appendProp(props, "deallocateLocation", a->deallocateLocation, first);
            appendProp(props, "owner", a->owner, first);
        } else if (ct == "LifetimeAnnotation") {
            tag = "lifetime";
            auto* a = static_cast<const LifetimeAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
            appendProp(props, "lifetimeScope", a->lifetimeScope, first);
        } else if (ct == "ReclaimAnnotation") {
            tag = "reclaim";
            auto* a = static_cast<const ReclaimAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
            appendProp(props, "reclaimPattern", a->reclaimPattern, first);
        } else if (ct == "OwnerAnnotation") {
            tag = "owner";
            auto* a = static_cast<const OwnerAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
            appendProp(props, "ownerType", a->ownerType, first);
        } else if (ct == "AllocateAnnotation") {
            tag = "allocate";
            auto* a = static_cast<const AllocateAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
            appendProp(props, "allocationPattern", a->allocationPattern, first);
        } else if (ct == "HotColdAnnotation") {
            tag = "hotcold";
            auto* a = static_cast<const HotColdAnnotation*>(anno);
            appendProp(props, "hint", a->hint, first);
        } else if (ct == "InlineAnnotation") {
            tag = "inline";
            auto* a = static_cast<const InlineAnnotation*>(anno);
            appendProp(props, "mode", a->mode, first);
        } else if (ct == "PureAnnotation") {
            tag = "pure";
        } else if (ct == "ConstExprAnnotation") {
            tag = "constexpr";
        } else if (ct == "DerefStrategy") {
            tag = "deref";
            auto* a = static_cast<const DerefStrategy*>(anno);
            appendProp(props, "strategy", a->strategy, first);
        } else if (ct == "OptimizationLock") {
            tag = "optlock";
            auto* a = static_cast<const OptimizationLock*>(anno);
            appendProp(props, "lockedBy", a->lockedBy, first);
            appendProp(props, "lockReason", a->lockReason, first);
            appendProp(props, "lockLevel", a->lockLevel, first);
        } else if (ct == "LangSpecific") {
            tag = "langspecific";
            auto* a = static_cast<const LangSpecific*>(anno);
            appendProp(props, "language", a->language, first);
            appendProp(props, "idiomType", a->idiomType, first);

        // --- Subject 2: Type System ---
        } else if (ct == "BitWidthAnnotation") {
            tag = "bitwidth";
            auto* a = static_cast<const BitWidthAnnotation*>(anno);
            appendInt(props, "width", a->width, first);
        } else if (ct == "EndianAnnotation") {
            tag = "endian";
            auto* a = static_cast<const EndianAnnotation*>(anno);
            appendProp(props, "order", a->order, first);
        } else if (ct == "LayoutAnnotation") {
            tag = "layout";
            auto* a = static_cast<const LayoutAnnotation*>(anno);
            appendProp(props, "mode", a->mode, first);
            appendInt(props, "alignment", a->alignment, first);
        } else if (ct == "NullabilityAnnotation") {
            tag = "nullability";
            auto* a = static_cast<const NullabilityAnnotation*>(anno);
            appendBool(props, "nullable", a->nullable, first);
            appendProp(props, "strategy", a->strategy, first);
        } else if (ct == "VarianceAnnotation") {
            tag = "variance";
            auto* a = static_cast<const VarianceAnnotation*>(anno);
            appendProp(props, "variance", a->variance, first);
        } else if (ct == "IdentityAnnotation") {
            tag = "identity";
            auto* a = static_cast<const IdentityAnnotation*>(anno);
            appendProp(props, "mode", a->mode, first);
        } else if (ct == "MutAnnotation") {
            tag = "mut";
            auto* a = static_cast<const MutAnnotation*>(anno);
            appendProp(props, "depth", a->depth, first);
        } else if (ct == "TypeStateAnnotation") {
            tag = "typestate";
            auto* a = static_cast<const TypeStateAnnotation*>(anno);
            appendProp(props, "state", a->state, first);

        // --- Subject 3: Concurrency ---
        } else if (ct == "AtomicAnnotation") {
            tag = "atomic";
            auto* a = static_cast<const AtomicAnnotation*>(anno);
            appendProp(props, "consistency", a->consistency, first);
        } else if (ct == "SyncAnnotation") {
            tag = "sync";
            auto* a = static_cast<const SyncAnnotation*>(anno);
            appendProp(props, "primitive", a->primitive, first);
        } else if (ct == "ThreadModelAnnotation") {
            tag = "threadmodel";
            auto* a = static_cast<const ThreadModelAnnotation*>(anno);
            appendProp(props, "model", a->model, first);
        } else if (ct == "MemoryBarrierAnnotation") {
            tag = "memorybarrier";
        } else if (ct == "ExecAnnotation") {
            tag = "exec";
            auto* a = static_cast<const ExecAnnotation*>(anno);
            appendProp(props, "mode", a->mode, first);
            appendProp(props, "runtimeHint", a->runtimeHint, first);
        } else if (ct == "BlockingAnnotation") {
            tag = "blocking";
            auto* a = static_cast<const BlockingAnnotation*>(anno);
            appendProp(props, "kind", a->kind, first);
        } else if (ct == "ParallelAnnotation") {
            tag = "parallel";
            auto* a = static_cast<const ParallelAnnotation*>(anno);
            appendProp(props, "kind", a->kind, first);
        } else if (ct == "TrapAnnotation") {
            tag = "trap";
            auto* a = static_cast<const TrapAnnotation*>(anno);
            appendProp(props, "signal", a->signal, first);
        } else if (ct == "ExceptionAnnotation") {
            tag = "exception";
            auto* a = static_cast<const ExceptionAnnotation*>(anno);
            appendProp(props, "style", a->style, first);
        } else if (ct == "PanicAnnotation") {
            tag = "panic";
            auto* a = static_cast<const PanicAnnotation*>(anno);
            appendProp(props, "behavior", a->behavior, first);

        // --- Subject 4: Scope ---
        } else if (ct == "BindingAnnotation") {
            tag = "binding";
            auto* a = static_cast<const BindingAnnotation*>(anno);
            appendProp(props, "time", a->time, first);
        } else if (ct == "LookupAnnotation") {
            tag = "lookup";
            auto* a = static_cast<const LookupAnnotation*>(anno);
            appendProp(props, "mode", a->mode, first);
        } else if (ct == "CaptureAnnotation") {
            tag = "capture";
            auto* a = static_cast<const CaptureAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
        } else if (ct == "VisibilityAnnotation") {
            tag = "visibility";
            auto* a = static_cast<const VisibilityAnnotation*>(anno);
            appendProp(props, "level", a->level, first);
        } else if (ct == "NamespaceAnnotation") {
            tag = "namespace";
            auto* a = static_cast<const NamespaceAnnotation*>(anno);
            appendProp(props, "style", a->style, first);
        } else if (ct == "ScopeAnnotation") {
            tag = "scope";
            auto* a = static_cast<const ScopeAnnotation*>(anno);
            appendProp(props, "kind", a->kind, first);

        // --- Subject 5: Shims ---
        } else if (ct == "IntrinsicAnnotation") {
            tag = "intrinsic";
            auto* a = static_cast<const IntrinsicAnnotation*>(anno);
            appendProp(props, "instruction", a->instruction, first);
            appendProp(props, "arch", a->arch, first);
        } else if (ct == "RawAnnotation") {
            tag = "raw";
            auto* a = static_cast<const RawAnnotation*>(anno);
            appendProp(props, "language", a->language, first);
            appendProp(props, "code", a->code, first);
        } else if (ct == "CallingConvAnnotation") {
            tag = "callingconv";
            auto* a = static_cast<const CallingConvAnnotation*>(anno);
            appendProp(props, "convention", a->convention, first);
        } else if (ct == "LinkAnnotation") {
            tag = "link";
            auto* a = static_cast<const LinkAnnotation*>(anno);
            appendProp(props, "symbolName", a->symbolName, first);
            appendProp(props, "library", a->library, first);
        } else if (ct == "ShimAnnotation") {
            tag = "shim";
            auto* a = static_cast<const ShimAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
        } else if (ct == "PointerArithmeticAnnotation") {
            tag = "pointerarith";
        } else if (ct == "OpaqueAnnotation") {
            tag = "opaque";
            auto* a = static_cast<const OpaqueAnnotation*>(anno);
            appendProp(props, "reason", a->reason, first);
        } else if (ct == "TargetAnnotation") {
            tag = "target";
            auto* a = static_cast<const TargetAnnotation*>(anno);
            appendProp(props, "platform", a->platform, first);
            appendProp(props, "arch", a->arch, first);
        } else if (ct == "FeatureAnnotation") {
            tag = "feature";
            auto* a = static_cast<const FeatureAnnotation*>(anno);
            appendProp(props, "flag", a->flag, first);
            appendBool(props, "enabled", a->enabled, first);
        } else if (ct == "OriginalAnnotation") {
            tag = "original";
            auto* a = static_cast<const OriginalAnnotation*>(anno);
            appendProp(props, "sourceCode", a->sourceCode, first);
            appendProp(props, "sourceLanguage", a->sourceLanguage, first);
        } else if (ct == "MappingAnnotation") {
            tag = "mapping";
            auto* a = static_cast<const MappingAnnotation*>(anno);
            appendVec(props, "history", a->history, first);

        // --- Subject 6: Optimization ---
        } else if (ct == "TailCallAnnotation") {
            tag = "tailcall";
        } else if (ct == "LoopAnnotation") {
            tag = "loop";
            auto* a = static_cast<const LoopAnnotation*>(anno);
            appendProp(props, "hint", a->hint, first);
            appendInt(props, "factor", a->factor, first);
        } else if (ct == "DataAnnotation") {
            tag = "data";
            auto* a = static_cast<const DataAnnotation*>(anno);
            appendProp(props, "hint", a->hint, first);
        } else if (ct == "AlignAnnotation") {
            tag = "align";
            auto* a = static_cast<const AlignAnnotation*>(anno);
            appendInt(props, "bytes", a->bytes, first);
        } else if (ct == "PackAnnotation") {
            tag = "pack";
        } else if (ct == "BoundsCheckAnnotation") {
            tag = "boundscheck";
            auto* a = static_cast<const BoundsCheckAnnotation*>(anno);
            appendBool(props, "enabled", a->enabled, first);
        } else if (ct == "OverflowAnnotation") {
            tag = "overflow";
            auto* a = static_cast<const OverflowAnnotation*>(anno);
            appendProp(props, "behavior", a->behavior, first);

        // --- Subject 7: Meta-Programming ---
        } else if (ct == "MetaAnnotation") {
            tag = "meta";
            auto* a = static_cast<const MetaAnnotation*>(anno);
            appendProp(props, "state", a->state, first);
            appendProp(props, "phase", a->phase, first);
        } else if (ct == "SymbolAnnotation") {
            tag = "symbol";
            auto* a = static_cast<const SymbolAnnotation*>(anno);
            appendProp(props, "mode", a->mode, first);
        } else if (ct == "EvaluateAnnotation") {
            tag = "evaluate";
            auto* a = static_cast<const EvaluateAnnotation*>(anno);
            appendProp(props, "phase", a->phase, first);
        } else if (ct == "TemplateAnnotation") {
            tag = "template";
            auto* a = static_cast<const TemplateAnnotation*>(anno);
            appendProp(props, "specialization", a->specialization, first);
        } else if (ct == "SyntheticAnnotation") {
            tag = "synthetic";
            auto* a = static_cast<const SyntheticAnnotation*>(anno);
            appendProp(props, "generator", a->generator, first);
            appendBool(props, "isStructuralRisk", a->isStructuralRisk, first);

        // --- Subject 8: Policy ---
        } else if (ct == "PolicyAnnotation") {
            tag = "policy";
            auto* a = static_cast<const PolicyAnnotation*>(anno);
            appendProp(props, "strictness", a->strictness, first);
            appendProp(props, "perf", a->perf, first);
            appendProp(props, "style", a->style, first);
            appendBool(props, "binaryStable", a->binaryStable, first);
        } else if (ct == "AmbiguityAnnotation") {
            tag = "ambiguity";
            auto* a = static_cast<const AmbiguityAnnotation*>(anno);
            appendProp(props, "intent", a->intent, first);
            appendVec(props, "options", a->options, first);
            appendProp(props, "level", a->level, first);
            appendProp(props, "description", a->description, first);
        } else if (ct == "CandidateAnnotation") {
            tag = "candidate";
            auto* a = static_cast<const CandidateAnnotation*>(anno);
            appendVec(props, "inferredTypes", a->inferredTypes, first);
        } else if (ct == "TradeoffAnnotation") {
            tag = "tradeoff";
            auto* a = static_cast<const TradeoffAnnotation*>(anno);
            appendProp(props, "reason", a->reason, first);
            appendProp(props, "safetyCost", a->safetyCost, first);
            appendProp(props, "perfCost", a->perfCost, first);
        } else if (ct == "ChoiceAnnotation") {
            tag = "choice";
            auto* a = static_cast<const ChoiceAnnotation*>(anno);
            appendProp(props, "choiceId", a->choiceId, first);
            appendVec(props, "options", a->options, first);
        } else if (ct == "DecisionAnnotation") {
            tag = "decision";
            auto* a = static_cast<const DecisionAnnotation*>(anno);
            appendProp(props, "choiceId", a->choiceId, first);
            appendProp(props, "selection", a->selection, first);
            appendProp(props, "author", a->author, first);
            appendProp(props, "reason", a->reason, first);

        // --- Subject 9: Workflow Routing ---
        } else if (ct == "ContextWidthAnnotation") {
            tag = "contextwidth";
            auto* a = static_cast<const ContextWidthAnnotation*>(anno);
            appendProp(props, "width", a->width, first);
        } else if (ct == "ReviewAnnotation") {
            tag = "review";
            auto* a = static_cast<const ReviewAnnotation*>(anno);
            appendBool(props, "required", a->required, first);
            appendProp(props, "reviewer", a->reviewer, first);
            appendProp(props, "reason", a->reason, first);
        } else if (ct == "AutomatabilityAnnotation") {
            tag = "automatability";
            auto* a = static_cast<const AutomatabilityAnnotation*>(anno);
            appendProp(props, "strategy", a->strategy, first);
            if (a->confidence > 0.0) {
                if (!first) props += ",";
                props += "confidence=" + std::to_string(a->confidence);
                first = false;
            }
        } else if (ct == "PriorityAnnotation") {
            tag = "priority";
            auto* a = static_cast<const PriorityAnnotation*>(anno);
            appendProp(props, "level", a->level, first);
            appendVec(props, "blockedBy", a->blockedBy, first);
        } else if (ct == "ImplementationStatusAnnotation") {
            tag = "implstatus";
            auto* a = static_cast<const ImplementationStatusAnnotation*>(anno);
            appendProp(props, "status", a->status, first);
            appendProp(props, "assignee", a->assignee, first);

        // --- Semantic Core ---
        } else if (ct == "IntentAnnotation") {
            tag = "intent";
            auto* a = static_cast<const IntentAnnotation*>(anno);
            appendProp(props, "summary", a->summary, first);
            appendProp(props, "category", a->category, first);
        } else if (ct == "ComplexityAnnotation") {
            tag = "complexity";
            auto* a = static_cast<const ComplexityAnnotation*>(anno);
            appendProp(props, "timeComplexity", a->timeComplexity, first);
            appendInt(props, "cognitiveComplexity", a->cognitiveComplexity, first);
            appendInt(props, "linesOfLogic", a->linesOfLogic, first);
        } else if (ct == "RiskAnnotation") {
            tag = "risk";
            auto* a = static_cast<const RiskAnnotation*>(anno);
            appendProp(props, "level", a->level, first);
            appendProp(props, "reason", a->reason, first);
            appendInt(props, "dependentCount", a->dependentCount, first);
        } else if (ct == "ContractAnnotation") {
            tag = "contract";
            auto* a = static_cast<const ContractAnnotation*>(anno);
            appendProp(props, "preconditions", a->preconditions, first);
            appendProp(props, "postconditions", a->postconditions, first);
            appendProp(props, "returnShape", a->returnShape, first);
            appendProp(props, "sideEffects", a->sideEffects, first);
        } else if (ct == "SemanticTagAnnotation") {
            tag = "tags";
            auto* a = static_cast<const SemanticTagAnnotation*>(anno);
            appendVec(props, "tags", a->tags, first);

        // --- Environment ---
        } else if (ct == "CapabilityRequirement") {
            tag = "capability";
            auto* a = static_cast<const CapabilityRequirement*>(anno);
            appendProp(props, "capability", a->capability, first);
            appendBool(props, "required", a->required, first);

        } else {
            return "";  // unrecognised annotation type
        }

        // Assemble final string
        if (props.empty())
            return "@semanno:" + tag;
        return "@semanno:" + tag + "(" + props + ")";
    }
};

// ---------------------------------------------------------------------------
// SemannoParser
// ---------------------------------------------------------------------------

struct SemannoEntry {
    std::string type;                             // e.g. "intent"
    std::map<std::string, std::string> properties; // key→raw value string
};

class SemannoParser {
public:
    /// Check whether a line contains a Semanno annotation.
    static bool isSemannoComment(const std::string& line) {
        return line.find("@semanno:") != std::string::npos;
    }

    /// Parse a single Semanno comment line.
    /// Accepts lines with any comment prefix: "//", "#", "/*", "--", etc.
    /// Returns a SemannoEntry with type and key-value properties.
    static SemannoEntry parse(const std::string& line) {
        SemannoEntry entry;

        // Locate the "@semanno:" marker
        auto pos = line.find("@semanno:");
        if (pos == std::string::npos) return entry;

        pos += 9;  // skip past "@semanno:"

        // Extract the type name (until '(' or end of relevant content)
        size_t typeEnd = pos;
        while (typeEnd < line.size() && line[typeEnd] != '(' &&
               line[typeEnd] != ')' && line[typeEnd] != ' ' &&
               line[typeEnd] != '\t' && line[typeEnd] != '\n' &&
               line[typeEnd] != '\r') {
            ++typeEnd;
        }
        entry.type = line.substr(pos, typeEnd - pos);

        // Strip any trailing comment close markers from type (e.g. "*/" )
        while (!entry.type.empty() &&
               (entry.type.back() == '*' || entry.type.back() == '/')) {
            entry.type.pop_back();
        }

        // If there is no property block, we are done.
        if (typeEnd >= line.size() || line[typeEnd] != '(') return entry;

        // Find the matching closing paren (respecting escaped quotes).
        size_t propStart = typeEnd + 1;
        size_t propEnd = findMatchingParen(line, propStart);
        if (propEnd == std::string::npos) propEnd = line.size();

        std::string propBlock = line.substr(propStart, propEnd - propStart);
        parseProperties(propBlock, entry.properties);

        return entry;
    }

private:
    /// Find the closing ')' that matches an opening '(' at `start`,
    /// respecting quoted strings.
    static size_t findMatchingParen(const std::string& s, size_t start) {
        bool inQuote = false;
        for (size_t i = start; i < s.size(); ++i) {
            if (s[i] == '\\' && i + 1 < s.size()) {
                ++i;  // skip escaped char
                continue;
            }
            if (s[i] == '"') {
                inQuote = !inQuote;
            } else if (s[i] == ')' && !inQuote) {
                return i;
            }
        }
        return std::string::npos;
    }

    /// Parse "key=\"value\",key2=value2,..." into a map.
    static void parseProperties(const std::string& block,
                                std::map<std::string, std::string>& props) {
        size_t i = 0;
        while (i < block.size()) {
            // Skip whitespace
            while (i < block.size() && (block[i] == ' ' || block[i] == '\t'))
                ++i;
            if (i >= block.size()) break;

            // Read key (up to '=')
            size_t keyStart = i;
            while (i < block.size() && block[i] != '=') ++i;
            if (i >= block.size()) break;
            std::string key = block.substr(keyStart, i - keyStart);
            // Trim trailing whitespace from key
            while (!key.empty() && (key.back() == ' ' || key.back() == '\t'))
                key.pop_back();
            ++i;  // skip '='

            // Skip whitespace after '='
            while (i < block.size() && (block[i] == ' ' || block[i] == '\t'))
                ++i;

            std::string value;
            if (i < block.size() && block[i] == '"') {
                // Quoted value — read until unescaped closing '"'
                ++i;  // skip opening '"'
                while (i < block.size()) {
                    if (block[i] == '\\' && i + 1 < block.size()) {
                        value += block[i];
                        value += block[i + 1];
                        i += 2;
                    } else if (block[i] == '"') {
                        ++i;  // skip closing '"'
                        break;
                    } else {
                        value += block[i];
                        ++i;
                    }
                }
                // Unescape the value
                value = semanno_detail::unescapeValue(value);
            } else {
                // Unquoted value — read until ',' or end
                size_t valStart = i;
                while (i < block.size() && block[i] != ',') ++i;
                value = block.substr(valStart, i - valStart);
                // Trim trailing whitespace
                while (!value.empty() &&
                       (value.back() == ' ' || value.back() == '\t'))
                    value.pop_back();
            }

            props[key] = value;

            // Skip comma separator
            if (i < block.size() && block[i] == ',') ++i;
        }
    }
};

// end of SemannoFormat.h
