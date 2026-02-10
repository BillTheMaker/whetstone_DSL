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
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
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
            lang == "javascript" || lang == "java" || lang == "csharp") {
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
            out.push_back({mod->id, "DeallocateAnnotation", "Explicit",
                           "C requires explicit malloc/free", 0.90});
        }
        else if (lang == "rust") {
            out.push_back({mod->id, "OwnerAnnotation", "Single",
                           "Rust uses single-owner lifetime tracking", 0.95});
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

    void inferFunction(const Function* fn, const std::string& lang,
                       std::vector<Suggestion>& out) const {
        // Skip if function already has a memory annotation
        if (hasMemoryAnnotation(fn)) return;

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
