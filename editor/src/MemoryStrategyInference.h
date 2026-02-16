#pragma once
// Step 65: Memory strategy inference
//
// MemoryStrategyInference: analyses an AST and suggests appropriate
// memory annotations based on the module's target language and
// usage patterns.  Suggestions are returned as data — they are
// never auto-applied to the AST.

#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Type.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"

class MemoryStrategyInference {
public:
    struct Suggestion {
        std::string nodeId;
        std::string annotationType;  // e.g. "ReclaimAnnotation"
        std::string strategy;        // e.g. "Tracing"
        std::string reason;
        double confidence;           // 0.0 – 1.0
    };

    struct FeedbackStats {
        int accepted = 0;
        int rejected = 0;
    };

    // Analyse the AST and return suggestions.  Does NOT modify the AST.
    std::vector<Suggestion> inferAnnotations(const ASTNode* root) const {
        std::vector<Suggestion> out;
        if (!root) return out;
        inferNode(root, out);
        applyFeedback(out);
        return out;
    }

    void recordFeedback(const Suggestion& suggestion, bool accepted) const {
        auto& stats = feedback_[feedbackKey(suggestion)];
        if (accepted) {
            stats.accepted += 1;
        } else {
            stats.rejected += 1;
        }
    }

private:
    inline static std::unordered_map<std::string, FeedbackStats> feedback_;

    void inferNode(const ASTNode* node,
                   std::vector<Suggestion>& out) const {
        if (!node) return;

        if (node->conceptType == "Module") {
            inferModule(static_cast<const Module*>(node), out);
        }

        // Recurse — but we already handle functions inside inferModule,
        // so only recurse into non-Module children here for nested modules.
        for (auto* child : node->allChildren()) {
            if (child->conceptType == "Module") {
                inferNode(child, out);
            }
        }
    }

    void inferModule(const Module* mod,
                     std::vector<Suggestion>& out) const {
        const auto& lang = mod->targetLanguage;

        // --- language-level defaults ---

        if (lang == "python" || lang == "elisp" || lang == "ruby" ||
            lang == "javascript" || lang == "java" || lang == "csharp" ||
            lang == "kotlin") {
            // GC languages → @Reclaim(Tracing) on the module
            out.push_back({mod->id, "ReclaimAnnotation", "Tracing",
                           lang + " uses tracing garbage collection",
                           0.95});
        }
        else if (lang == "go") {
            out.push_back({mod->id, "ReclaimAnnotation", "Escape",
                           "Go uses escape-analysis-based GC", 0.90});
        }
        else if (lang == "cpp") {
            inferCppModule(mod, out);
        }
        else if (lang == "c") {
            inferCModule(mod, out);
        }
        else if (lang == "rust") {
            out.push_back({mod->id, "OwnerAnnotation", "Single",
                           "Rust uses single-owner lifetime tracking", 0.95});
        }
        else if (lang == "wat" || lang == "wasm") {
            inferWatModule(mod, out);
        }
        else if (lang == "swift" || lang == "objc") {
            out.push_back({mod->id, "OwnerAnnotation", "Shared_ARC",
                           lang + " uses automatic reference counting", 0.90});
        }

        // --- per-function analysis ---
        for (auto* child : mod->getChildren("functions")) {
            if (child->conceptType == "Function") {
                inferFunction(static_cast<const Function*>(child), lang, out);
            }
        }
    }

    void inferCppModule(const Module* mod,
                        std::vector<Suggestion>& out) const {
        // Default C++ suggestion: RAII is idiomatic
        out.push_back({mod->id, "LifetimeAnnotation", "RAII",
                       "C++ idiom: RAII with scope-based cleanup", 0.70});
    }

    void inferCModule(const Module* mod,
                      std::vector<Suggestion>& out) const {
        out.push_back({mod->id, "OwnerAnnotation", "Manual",
                       "C defaults to manual ownership", 0.95});
        out.push_back({mod->id, "LifetimeAnnotation", "Scope",
                       "C locals default to lexical scope lifetime", 0.92});
        out.push_back({mod->id, "ReclaimAnnotation", "Explicit",
                       "C uses explicit reclamation (free)", 0.95});
    }

    void inferWatModule(const Module* mod,
                        std::vector<Suggestion>& out) const {
        out.push_back({mod->id, "OwnerAnnotation", "Manual",
                       "WAT linear memory is manually managed", 0.92});
        out.push_back({mod->id, "ReclaimAnnotation", "Explicit",
                       "WAT has explicit memory lifetime operations", 0.88});
    }

    void inferFunction(const Function* fn, const std::string& lang,
                       std::vector<Suggestion>& out) const {
        // Skip if function already has a memory annotation
        if (hasMemoryAnnotation(fn)) return;

        if (lang == "c") {
            inferCFunction(fn, out);
            return;
        }
        if (lang == "wat" || lang == "wasm") {
            inferWatFunction(fn, out);
            return;
        }

        // Check body for patterns
        bool hasAlloc = false;
        bool hasDealloc = false;
        bool allConst = true;

        for (auto* child : fn->getChildren("body")) {
            if (child->conceptType == "Variable") {
                // Any mutable variable means not all-const
                allConst = false;
            }
            scanForAllocDealloc(child, hasAlloc, hasDealloc);
        }

        if (lang == "cpp") {
            if (hasAlloc && !hasDealloc) {
                out.push_back({fn->id, "LifetimeAnnotation", "RAII",
                               "Function allocates without explicit dealloc — "
                               "RAII recommended",
                               0.75});
            } else if (hasAlloc && hasDealloc) {
                out.push_back({fn->id, "DeallocateAnnotation", "Explicit",
                               "Function uses explicit alloc/dealloc pattern",
                               0.80});
            }
        }

        // Immutable function body (no variables) → static candidate
        if (allConst && fn->getChildren("body").empty() == false) {
            // Only suggest if not already covered by a language-level rule
            if (lang == "cpp" || lang == "c") {
                out.push_back({fn->id, "AllocateAnnotation", "Static",
                               "Function body contains no mutable state — "
                               "static allocation candidate",
                               0.50});
            }
        }
    }

    void inferCFunction(const Function* fn,
                        std::vector<Suggestion>& out) const {
        bool hasAlloc = false;

        for (auto* paramNode : fn->getChildren("parameters")) {
            if (paramNode->conceptType != "Parameter") continue;
            auto* typeNode = paramNode->getChild("type");
            if (isPointerType(typeNode)) {
                out.push_back({paramNode->id, "OwnerAnnotation", "Borrowed",
                               "Pointer parameters in C are borrowed by default",
                               0.86});
            }
        }

        for (auto* stmt : fn->getChildren("body")) {
            hasAlloc = hasAlloc || containsAllocCall(stmt);
            inferCLocalLifetime(stmt, out);
        }

        auto* retType = fn->getChild("returnType");
        if (isPointerType(retType)) {
            out.push_back({fn->id, "OwnerAnnotation", inferCReturnOwner(fn->name),
                           "C pointer returns are convention-based ownership transfers",
                           0.72});
        }

        if (hasAlloc) {
            out.push_back({fn->id, "OwnerAnnotation", "Manual",
                           "malloc/calloc allocation indicates manual ownership",
                           0.94});
            out.push_back({fn->id, "ReclaimAnnotation", "Explicit",
                           "malloc/calloc requires explicit free", 0.94});
        }
    }

    void inferWatFunction(const Function* fn,
                          std::vector<Suggestion>& out) const {
        bool hasMemoryOps = false;
        for (auto* stmt : fn->getChildren("body")) {
            hasMemoryOps = hasMemoryOps || containsWatMemoryOp(stmt);
        }
        if (hasMemoryOps) {
            out.push_back({fn->id, "OwnerAnnotation", "Manual",
                           "WAT memory/load/store operations require manual ownership",
                           0.90});
        }
    }

    static bool hasMemoryAnnotation(const ASTNode* node) {
        for (auto* anno : node->getChildren("annotations")) {
            const auto& ct = anno->conceptType;
            if (ct == "DeallocateAnnotation" || ct == "LifetimeAnnotation" ||
                ct == "ReclaimAnnotation" || ct == "OwnerAnnotation" ||
                ct == "AllocateAnnotation")
                return true;
        }
        return false;
    }

    static void scanForAllocDealloc(const ASTNode* node,
                                    bool& hasAlloc, bool& hasDealloc) {
        if (!node) return;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "malloc" || fc->functionName == "new" ||
                fc->functionName == "calloc" || fc->functionName == "allocate")
                hasAlloc = true;
            if (fc->functionName == "free" || fc->functionName == "delete" ||
                fc->functionName == "release" || fc->functionName == "deallocate")
                hasDealloc = true;
        }
        for (auto* child : node->allChildren()) {
            scanForAllocDealloc(child, hasAlloc, hasDealloc);
        }
    }

    static bool containsAllocCall(const ASTNode* node) {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "malloc" || fc->functionName == "calloc") {
                return true;
            }
        }
        for (auto* child : node->allChildren()) {
            if (containsAllocCall(child)) return true;
        }
        return false;
    }

    static bool containsWatMemoryOp(const ASTNode* node) {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            std::string fn = lowerAscii(fc->functionName);
            if (fn.find("memory.") != std::string::npos ||
                fn.find(".load") != std::string::npos ||
                fn.find(".store") != std::string::npos ||
                fn == "memory.grow")
                return true;
        }
        for (auto* child : node->allChildren()) {
            if (containsWatMemoryOp(child)) return true;
        }
        return false;
    }

    static void inferCLocalLifetime(const ASTNode* node,
                                    std::vector<Suggestion>& out) {
        if (!node) return;
        if (node->conceptType == "Variable") {
            auto* v = static_cast<const Variable*>(node);
            auto* typeNode = v->getChild("type");
            std::string typeKind = typeString(typeNode);
            std::string lowered = lowerAscii(typeKind);
            if (lowered.find("static") != std::string::npos) {
                out.push_back({v->id, "LifetimeAnnotation", "Static",
                               "Static storage duration detected in C declaration",
                               0.90});
            } else {
                out.push_back({v->id, "LifetimeAnnotation", "Scope",
                               "Local C variable uses scope-bound lifetime", 0.88});
            }
        }
        for (auto* child : node->allChildren()) {
            inferCLocalLifetime(child, out);
        }
    }

    static std::string inferCReturnOwner(const std::string& fnName) {
        std::string lower = lowerAscii(fnName);
        if (startsWith(lower, "get") || startsWith(lower, "borrow") ||
            startsWith(lower, "peek") || startsWith(lower, "view")) {
            return "Borrowed";
        }
        return "Transferred";
    }

    static bool isPointerType(const ASTNode* typeNode) {
        return typeString(typeNode).find('*') != std::string::npos;
    }

    static std::string typeString(const ASTNode* typeNode) {
        if (!typeNode) return "";
        if (typeNode->conceptType == "PrimitiveType") {
            return static_cast<const PrimitiveType*>(typeNode)->kind;
        }
        if (typeNode->conceptType == "CustomType") {
            return static_cast<const CustomType*>(typeNode)->typeName;
        }
        return "";
    }

    static std::string lowerAscii(const std::string& input) {
        std::string out = input;
        std::transform(out.begin(), out.end(), out.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return out;
    }

    static bool startsWith(const std::string& value, const std::string& prefix) {
        return value.rfind(prefix, 0) == 0;
    }

    static std::string feedbackKey(const Suggestion& suggestion) {
        return suggestion.annotationType + ":" + suggestion.strategy;
    }

    void applyFeedback(std::vector<Suggestion>& out) const {
        for (auto& s : out) {
            auto it = feedback_.find(feedbackKey(s));
            if (it == feedback_.end()) continue;
            const auto& stats = it->second;
            int total = stats.accepted + stats.rejected;
            if (total <= 0) continue;
            double bias = (double)(stats.accepted - stats.rejected) /
                          (double)total;
            double factor = 1.0 + 0.25 * bias;
            s.confidence = std::clamp(s.confidence * factor, 0.0, 1.0);
        }
    }
};
