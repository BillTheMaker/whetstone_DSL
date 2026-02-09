#pragma once
// Step 64: Memory annotation validation
//
// AnnotationValidator: checks canonical memory annotations for consistency.
//
// Rules:
//   @Deallocate(Explicit) without a deallocation point → "Missing Intent" error
//   @Owner(Single) with aliasing (variable assigned to another) → alias error
//   Conflicting parent/child annotations of same family → conflict error
//   @Reclaim(Tracing), @Lifetime(RAII) with proper usage → pass

#include <string>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"

class AnnotationValidator {
public:
    struct Diagnostic {
        std::string severity;  // "error" or "warning"
        std::string message;
        std::string nodeId;
    };

    std::vector<Diagnostic> validate(const ASTNode* root) const {
        std::vector<Diagnostic> diags;
        validateNode(root, diags);
        return diags;
    }

private:
    void validateNode(const ASTNode* node,
                      std::vector<Diagnostic>& diags) const {
        if (!node) return;

        // Check annotations on this node
        for (const auto* anno : node->getChildren("annotations")) {
            if (anno->conceptType == "DeallocateAnnotation") {
                auto* da = static_cast<const DeallocateAnnotation*>(anno);
                if (da->strategy == "Explicit") {
                    checkDeallocateExplicit(node, diags);
                }
            }
            else if (anno->conceptType == "OwnerAnnotation") {
                auto* oa = static_cast<const OwnerAnnotation*>(anno);
                if (oa->strategy == "Single") {
                    checkOwnerSingleAliasing(node, diags);
                }
                checkConflictWithAncestor(node, anno, diags);
            }
            else if (anno->conceptType == "ReclaimAnnotation" ||
                     anno->conceptType == "LifetimeAnnotation" ||
                     anno->conceptType == "AllocateAnnotation") {
                checkConflictWithAncestor(node, anno, diags);
            }
        }

        // Recurse into children
        for (auto* child : node->allChildren()) {
            validateNode(child, diags);
        }
    }

    // @Deallocate(Explicit): the enclosing function body must contain
    // evidence of deallocation (a FunctionCall to delete/free, or a
    // DeallocateAnnotation with a deallocateLocation set).
    // If not found, emit a "Missing Intent" error.
    void checkDeallocateExplicit(const ASTNode* node,
                                 std::vector<Diagnostic>& diags) const {
        // Find the function body to scan
        const ASTNode* fn = node;
        if (fn->conceptType != "Function") {
            // Walk up to the enclosing function
            fn = node->parent;
            while (fn && fn->conceptType != "Function") fn = fn->parent;
        }
        if (!fn) {
            diags.push_back({"error",
                             "Missing Intent: @Deallocate(Explicit) without "
                             "enclosing function scope",
                             node->id});
            return;
        }

        // Look for deallocation evidence in the function body
        bool found = false;
        for (auto* child : fn->getChildren("body")) {
            if (hasDeallocEvidence(child)) {
                found = true;
                break;
            }
        }

        // Also check the annotation itself for a filled deallocateLocation
        for (auto* anno : node->getChildren("annotations")) {
            if (anno->conceptType == "DeallocateAnnotation") {
                auto* da = static_cast<const DeallocateAnnotation*>(anno);
                if (!da->deallocateLocation.empty()) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) {
            diags.push_back({"error",
                             "Missing Intent: @Deallocate(Explicit) requires a "
                             "corresponding deallocation point (delete/free)",
                             node->id});
        }
    }

    // Recursively check for deallocation evidence (FunctionCall to
    // delete/free/release, or a Return that explicitly frees).
    bool hasDeallocEvidence(const ASTNode* node) const {
        if (!node) return false;
        if (node->conceptType == "FunctionCall") {
            auto* fc = static_cast<const FunctionCall*>(node);
            if (fc->functionName == "delete" || fc->functionName == "free" ||
                fc->functionName == "release" || fc->functionName == "deallocate")
                return true;
        }
        if (node->conceptType == "ExpressionStatement") {
            auto* child = node->getChild("expression");
            if (child && hasDeallocEvidence(child)) return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasDeallocEvidence(child)) return true;
        }
        return false;
    }

    // @Owner(Single): check for aliasing assignments in the function body.
    // An alias occurs when a locally-declared variable appears on the
    // value side of an Assignment (copied to another variable).
    void checkOwnerSingleAliasing(const ASTNode* node,
                                  std::vector<Diagnostic>& diags) const {
        const ASTNode* fn = node;
        if (fn->conceptType != "Function") {
            fn = node->parent;
            while (fn && fn->conceptType != "Function") fn = fn->parent;
        }
        if (!fn) return;

        // Collect locally-declared variable names
        std::vector<std::string> localNames;
        for (auto* child : fn->getChildren("body")) {
            if (child->conceptType == "Variable") {
                localNames.push_back(
                    static_cast<const Variable*>(child)->name);
            }
        }

        // Scan body for assignments where value references a local variable
        std::vector<const ASTNode*> assignments;
        collectByType(fn, "Assignment", assignments);

        for (auto* assignNode : assignments) {
            auto* valNode = assignNode->getChild("value");
            if (valNode && valNode->conceptType == "VariableReference") {
                auto* vr = static_cast<const VariableReference*>(valNode);
                for (const auto& local : localNames) {
                    if (vr->variableName == local) {
                        diags.push_back({"error",
                            "@Owner(Single) violation: variable '" + local +
                            "' is aliased via assignment — single ownership "
                            "does not allow aliasing",
                            node->id});
                        return;  // one diagnostic per annotation
                    }
                }
            }
        }
    }

    // Check if a node's annotation conflicts with an ancestor's annotation
    // of the same family (same conceptType) but different strategy.
    void checkConflictWithAncestor(const ASTNode* node,
                                   const ASTNode* anno,
                                   std::vector<Diagnostic>& diags) const {
        std::string strategy = getStrategy(anno);
        if (strategy.empty()) return;

        // Walk up ancestors
        const ASTNode* cur = node->parent;
        while (cur) {
            for (auto* parentAnno : cur->getChildren("annotations")) {
                if (parentAnno->conceptType == anno->conceptType) {
                    std::string parentStrategy = getStrategy(parentAnno);
                    if (!parentStrategy.empty() &&
                        parentStrategy != strategy) {
                        diags.push_back({"error",
                            "Annotation conflict: " + anno->conceptType +
                            "(" + strategy + ") on node '" + node->id +
                            "' conflicts with " + anno->conceptType +
                            "(" + parentStrategy + ") on ancestor '" +
                            cur->id + "'",
                            node->id});
                        return;
                    }
                }
            }
            cur = cur->parent;
        }
    }

    // Extract the strategy string from any canonical annotation.
    static std::string getStrategy(const ASTNode* anno) {
        const auto& ct = anno->conceptType;
        if (ct == "DeallocateAnnotation")
            return static_cast<const DeallocateAnnotation*>(anno)->strategy;
        if (ct == "LifetimeAnnotation")
            return static_cast<const LifetimeAnnotation*>(anno)->strategy;
        if (ct == "ReclaimAnnotation")
            return static_cast<const ReclaimAnnotation*>(anno)->strategy;
        if (ct == "OwnerAnnotation")
            return static_cast<const OwnerAnnotation*>(anno)->strategy;
        if (ct == "AllocateAnnotation")
            return static_cast<const AllocateAnnotation*>(anno)->strategy;
        return "";
    }

    static void collectByType(const ASTNode* node, const std::string& type,
                              std::vector<const ASTNode*>& out) {
        if (!node) return;
        if (node->conceptType == type) out.push_back(node);
        for (auto* child : node->allChildren()) {
            collectByType(child, type, out);
        }
    }
};
