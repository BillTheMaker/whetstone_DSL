#pragma once
// Steps 297-298: Extended annotation validation for all annotation types
//
// AnnotationValidatorExtended: validates ALL annotation types from Subject 2-8.
// Diagnostic codes E0600-E1202.
//
// The base AnnotationValidator handles memory annotations (E01xx-E05xx).
// This class adds validation for type system, concurrency, scope, shim,
// optimization, meta-programming, and policy annotations.

#include <string>
#include <vector>
#include <initializer_list>
#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "AnnotationValidator.h"

class AnnotationValidatorExtended {
public:
    using Diagnostic = AnnotationValidator::Diagnostic;

    std::vector<Diagnostic> validate(const ASTNode* root) const {
        std::vector<Diagnostic> diags;
        validateNode(root, diags);
        return diags;
    }

private:
    static bool isPowerOf2(int v) { return v > 0 && (v & (v - 1)) == 0; }

    static bool isOneOf(const std::string& val, std::initializer_list<const char*> options) {
        for (auto* opt : options) if (val == opt) return true;
        return false;
    }

    void validateNode(const ASTNode* node, std::vector<Diagnostic>& diags) const {
        if (!node) return;
        for (const auto* anno : node->getChildren("annotations")) {
            validateAnnotation(node, anno, diags);
        }
        for (auto* child : node->allChildren()) {
            validateNode(child, diags);
        }
    }

    void validateAnnotation(const ASTNode* host, const ASTNode* anno,
                            std::vector<Diagnostic>& diags) const {
        const auto& ct = anno->conceptType;

        // === Subject 2: Type System Annotations ===

        if (ct == "BitWidthAnnotation") {
            auto* a = static_cast<const BitWidthAnnotation*>(anno);
            if (!isPowerOf2(a->width))
                diags.push_back({"error",
                    "[E0600] BitWidth must be power of 2, got " + std::to_string(a->width),
                    host->id});
        }
        else if (ct == "EndianAnnotation") {
            auto* a = static_cast<const EndianAnnotation*>(anno);
            if (!isOneOf(a->order, {"big", "little"}))
                diags.push_back({"error",
                    "[E0601] Endian order must be 'big' or 'little', got '" + a->order + "'",
                    host->id});
        }
        else if (ct == "LayoutAnnotation") {
            auto* a = static_cast<const LayoutAnnotation*>(anno);
            if (!isOneOf(a->mode, {"packed", "aligned"}))
                diags.push_back({"error",
                    "[E0602] Layout mode must be 'packed' or 'aligned', got '" + a->mode + "'",
                    host->id});
            if (a->alignment != 0 && !isPowerOf2(a->alignment))
                diags.push_back({"error",
                    "[E0603] Layout alignment must be positive power of 2, got " + std::to_string(a->alignment),
                    host->id});
        }
        else if (ct == "NullabilityAnnotation") {
            auto* a = static_cast<const NullabilityAnnotation*>(anno);
            if (!isOneOf(a->strategy, {"strict", "nullable"}))
                diags.push_back({"error",
                    "[E0604] Nullability strategy must be 'strict' or 'nullable', got '" + a->strategy + "'",
                    host->id});
        }
        else if (ct == "VarianceAnnotation") {
            auto* a = static_cast<const VarianceAnnotation*>(anno);
            if (!isOneOf(a->variance, {"covariant", "contravariant", "invariant"}))
                diags.push_back({"error",
                    "[E0605] Variance must be 'covariant', 'contravariant', or 'invariant', got '" + a->variance + "'",
                    host->id});
        }
        else if (ct == "IdentityAnnotation") {
            auto* a = static_cast<const IdentityAnnotation*>(anno);
            if (!isOneOf(a->mode, {"nominal", "structural"}))
                diags.push_back({"error",
                    "[E0606] Identity mode must be 'nominal' or 'structural', got '" + a->mode + "'",
                    host->id});
        }
        else if (ct == "MutAnnotation") {
            auto* a = static_cast<const MutAnnotation*>(anno);
            if (!isOneOf(a->depth, {"shallow", "deep", "interior"}))
                diags.push_back({"error",
                    "[E0607] Mut depth must be 'shallow', 'deep', or 'interior', got '" + a->depth + "'",
                    host->id});
        }
        else if (ct == "TypeStateAnnotation") {
            auto* a = static_cast<const TypeStateAnnotation*>(anno);
            if (!isOneOf(a->state, {"erased", "reified"}))
                diags.push_back({"error",
                    "[E0608] TypeState must be 'erased' or 'reified', got '" + a->state + "'",
                    host->id});
        }

        // === Subject 3: Concurrency Annotations ===

        else if (ct == "AtomicAnnotation") {
            auto* a = static_cast<const AtomicAnnotation*>(anno);
            if (!isOneOf(a->consistency, {"seq_cst", "relaxed", "acquire", "release"}))
                diags.push_back({"error",
                    "[E0700] Atomic consistency must be 'seq_cst', 'relaxed', 'acquire', or 'release', got '" + a->consistency + "'",
                    host->id});
        }
        else if (ct == "SyncAnnotation") {
            auto* a = static_cast<const SyncAnnotation*>(anno);
            if (!isOneOf(a->primitive, {"monitor", "spin", "semaphore"}))
                diags.push_back({"error",
                    "[E0701] Sync primitive must be 'monitor', 'spin', or 'semaphore', got '" + a->primitive + "'",
                    host->id});
        }
        else if (ct == "ThreadModelAnnotation") {
            auto* a = static_cast<const ThreadModelAnnotation*>(anno);
            if (!isOneOf(a->model, {"green", "os", "fiber"}))
                diags.push_back({"error",
                    "[E0702] ThreadModel must be 'green', 'os', or 'fiber', got '" + a->model + "'",
                    host->id});
        }
        else if (ct == "ExecAnnotation") {
            auto* a = static_cast<const ExecAnnotation*>(anno);
            if (!isOneOf(a->mode, {"async", "event"}))
                diags.push_back({"error",
                    "[E0703] Exec mode must be 'async' or 'event', got '" + a->mode + "'",
                    host->id});
            // E0704: async mode needs ThreadModel on enclosing scope
            if (a->mode == "async") {
                if (!hasThreadModelOnAncestor(host))
                    diags.push_back({"error",
                        "[E0704] Exec mode 'async' requires ThreadModel annotation on enclosing scope",
                        host->id});
            }
        }
        else if (ct == "BlockingAnnotation") {
            auto* a = static_cast<const BlockingAnnotation*>(anno);
            if (!isOneOf(a->kind, {"io", "compute"}))
                diags.push_back({"error",
                    "[E0705] Blocking kind must be 'io' or 'compute', got '" + a->kind + "'",
                    host->id});
        }
        else if (ct == "ParallelAnnotation") {
            auto* a = static_cast<const ParallelAnnotation*>(anno);
            if (!isOneOf(a->kind, {"data", "task"}))
                diags.push_back({"error",
                    "[E0706] Parallel kind must be 'data' or 'task', got '" + a->kind + "'",
                    host->id});
        }
        else if (ct == "ExceptionAnnotation") {
            auto* a = static_cast<const ExceptionAnnotation*>(anno);
            if (!isOneOf(a->style, {"checked", "unchecked"}))
                diags.push_back({"error",
                    "[E0707] Exception style must be 'checked' or 'unchecked', got '" + a->style + "'",
                    host->id});
        }
        else if (ct == "PanicAnnotation") {
            auto* a = static_cast<const PanicAnnotation*>(anno);
            if (!isOneOf(a->behavior, {"abort", "unwind"}))
                diags.push_back({"error",
                    "[E0708] Panic behavior must be 'abort' or 'unwind', got '" + a->behavior + "'",
                    host->id});
        }

        // === Subject 4: Scope & Namespace Annotations ===

        else if (ct == "BindingAnnotation") {
            auto* a = static_cast<const BindingAnnotation*>(anno);
            if (!isOneOf(a->time, {"static", "dynamic"}))
                diags.push_back({"error",
                    "[E0800] Binding time must be 'static' or 'dynamic', got '" + a->time + "'",
                    host->id});
        }
        else if (ct == "LookupAnnotation") {
            auto* a = static_cast<const LookupAnnotation*>(anno);
            if (!isOneOf(a->mode, {"lexical", "hoisted"}))
                diags.push_back({"error",
                    "[E0801] Lookup mode must be 'lexical' or 'hoisted', got '" + a->mode + "'",
                    host->id});
        }
        else if (ct == "CaptureAnnotation") {
            auto* a = static_cast<const CaptureAnnotation*>(anno);
            if (!isOneOf(a->strategy, {"value", "ref", "move"}))
                diags.push_back({"error",
                    "[E0802] Capture strategy must be 'value', 'ref', or 'move', got '" + a->strategy + "'",
                    host->id});
        }
        else if (ct == "VisibilityAnnotation") {
            auto* a = static_cast<const VisibilityAnnotation*>(anno);
            if (!isOneOf(a->level, {"private", "internal", "friend", "public"}))
                diags.push_back({"error",
                    "[E0803] Visibility level must be 'private', 'internal', 'friend', or 'public', got '" + a->level + "'",
                    host->id});
        }
        else if (ct == "NamespaceAnnotation") {
            auto* a = static_cast<const NamespaceAnnotation*>(anno);
            if (!isOneOf(a->style, {"qualified", "flat"}))
                diags.push_back({"error",
                    "[E0804] Namespace style must be 'qualified' or 'flat', got '" + a->style + "'",
                    host->id});
        }
        else if (ct == "ScopeAnnotation") {
            auto* a = static_cast<const ScopeAnnotation*>(anno);
            if (!isOneOf(a->kind, {"local", "global_leaked", "singleton"}))
                diags.push_back({"error",
                    "[E0805] Scope kind must be 'local', 'global_leaked', or 'singleton', got '" + a->kind + "'",
                    host->id});
        }

        // === Subject 5: Shim & Escape Hatch Annotations ===

        else if (ct == "CallingConvAnnotation") {
            auto* a = static_cast<const CallingConvAnnotation*>(anno);
            if (!isOneOf(a->convention, {"stdcall", "cdecl", "fastcall"}))
                diags.push_back({"error",
                    "[E0900] CallingConv convention must be 'stdcall', 'cdecl', or 'fastcall', got '" + a->convention + "'",
                    host->id});
        }
        else if (ct == "ShimAnnotation") {
            auto* a = static_cast<const ShimAnnotation*>(anno);
            if (!isOneOf(a->strategy, {"vtable", "trampoline", "union_tag", "cast"}))
                diags.push_back({"error",
                    "[E0901] Shim strategy must be 'vtable', 'trampoline', 'union_tag', or 'cast', got '" + a->strategy + "'",
                    host->id});
        }

        // === Subject 6: Optimization Annotations ===

        else if (ct == "InlineAnnotation") {
            // E1000: Inline(Always) + TailCall conflict
            auto* a = static_cast<const InlineAnnotation*>(anno);
            if (a->mode == "Always") {
                for (const auto* sibling : host->getChildren("annotations")) {
                    if (sibling->conceptType == "TailCallAnnotation") {
                        diags.push_back({"error",
                            "[E1000] Inline(Always) conflicts with TailCall annotation",
                            host->id});
                        break;
                    }
                }
            }
        }
        else if (ct == "LoopAnnotation") {
            auto* a = static_cast<const LoopAnnotation*>(anno);
            if (!isOneOf(a->hint, {"unroll", "vectorize", "fuse"}))
                diags.push_back({"error",
                    "[E1001] Loop hint must be 'unroll', 'vectorize', or 'fuse', got '" + a->hint + "'",
                    host->id});
            if (a->factor < 0 || (a->factor == 0 && a->hint == "unroll"))
                diags.push_back({"error",
                    "[E1002] Loop factor must be positive, got " + std::to_string(a->factor),
                    host->id});
        }
        else if (ct == "AlignAnnotation") {
            auto* a = static_cast<const AlignAnnotation*>(anno);
            if (!isPowerOf2(a->bytes))
                diags.push_back({"error",
                    "[E1003] Align bytes must be positive power of 2, got " + std::to_string(a->bytes),
                    host->id});
        }
        else if (ct == "OverflowAnnotation") {
            auto* a = static_cast<const OverflowAnnotation*>(anno);
            if (!isOneOf(a->behavior, {"wrap", "saturation", "panic"}))
                diags.push_back({"error",
                    "[E1004] Overflow behavior must be 'wrap', 'saturation', or 'panic', got '" + a->behavior + "'",
                    host->id});
        }

        // === Subject 7: Meta-Programming Annotations ===

        else if (ct == "MetaAnnotation") {
            auto* a = static_cast<const MetaAnnotation*>(anno);
            if (!isOneOf(a->state, {"quoted", "unquoted"}))
                diags.push_back({"error",
                    "[E1100] Meta state must be 'quoted' or 'unquoted', got '" + a->state + "'",
                    host->id});
            if (!isOneOf(a->phase, {"compile", "runtime"}))
                diags.push_back({"error",
                    "[E1101] Meta phase must be 'compile' or 'runtime', got '" + a->phase + "'",
                    host->id});
        }
        else if (ct == "SymbolAnnotation") {
            auto* a = static_cast<const SymbolAnnotation*>(anno);
            if (!isOneOf(a->mode, {"gensym", "interned"}))
                diags.push_back({"error",
                    "[E1102] Symbol mode must be 'gensym' or 'interned', got '" + a->mode + "'",
                    host->id});
        }
        else if (ct == "EvaluateAnnotation") {
            auto* a = static_cast<const EvaluateAnnotation*>(anno);
            if (!isOneOf(a->phase, {"compile_time", "runtime"}))
                diags.push_back({"error",
                    "[E1103] Evaluate phase must be 'compile_time' or 'runtime', got '" + a->phase + "'",
                    host->id});
        }
        else if (ct == "TemplateAnnotation") {
            auto* a = static_cast<const TemplateAnnotation*>(anno);
            if (!isOneOf(a->specialization, {"trait", "monomorphize", "erasure"}))
                diags.push_back({"error",
                    "[E1104] Template specialization must be 'trait', 'monomorphize', or 'erasure', got '" + a->specialization + "'",
                    host->id});
        }

        // === Subject 8: Policy Annotations ===

        else if (ct == "PolicyAnnotation") {
            auto* a = static_cast<const PolicyAnnotation*>(anno);
            if (!a->strictness.empty() && !isOneOf(a->strictness, {"high", "low"}))
                diags.push_back({"error",
                    "[E1200] Policy strictness must be 'high' or 'low', got '" + a->strictness + "'",
                    host->id});
            if (!a->perf.empty() && !isOneOf(a->perf, {"critical", "normal"}))
                diags.push_back({"error",
                    "[E1201] Policy perf must be 'critical' or 'normal', got '" + a->perf + "'",
                    host->id});
            if (!a->style.empty() && !isOneOf(a->style, {"idiomatic", "literal"}))
                diags.push_back({"error",
                    "[E1202] Policy style must be 'idiomatic' or 'literal', got '" + a->style + "'",
                    host->id});
        }
    }

    // Walk ancestors looking for a ThreadModelAnnotation
    bool hasThreadModelOnAncestor(const ASTNode* node) const {
        const ASTNode* cur = node->parent;
        while (cur) {
            for (const auto* anno : cur->getChildren("annotations")) {
                if (anno->conceptType == "ThreadModelAnnotation")
                    return true;
            }
            cur = cur->parent;
        }
        return false;
    }
};
