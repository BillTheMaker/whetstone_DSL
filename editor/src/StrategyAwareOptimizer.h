#pragma once
#include <string>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "ast/Expression.h"
#include "ast/Statement.h"

class StrategyAwareOptimizer {
public:
    struct OptResult {
        bool applied;
        std::string warning;
        std::string blocked;
        int nodesModified;
    };

    void setRoot(ASTNode* root) { root_ = root; }

    OptResult inlineVariable(const std::string& variableId) {
        OptResult result{false, "", "", 0};
        auto* node = findNode(root_, variableId);
        if (!node) return result;

        auto constraint = getAnnotationConstraint(node);
        if (constraint == Constraint::BlockDuplication) {
            result.blocked = "@Owner(Single) blocks inlining (would create alias)";
            return result;
        }

        result.applied = true;
        result.nodesModified = 1;
        return result;
    }

    OptResult reorderStatements(const std::string& functionId) {
        OptResult result{false, "", "", 0};
        auto* fn = findNode(root_, functionId);
        if (!fn) return result;

        auto constraint = getAnnotationConstraint(fn);
        if (constraint == Constraint::BlockReorder) {
            result.blocked = "@Deallocate(Explicit) blocks reordering around deallocation points";
            return result;
        }

        result.applied = true;
        result.nodesModified = 1;
        return result;
    }

    OptResult duplicateNode(const std::string& nodeId) {
        OptResult result{false, "", "", 0};
        auto* node = findNode(root_, nodeId);
        if (!node) return result;

        auto constraint = getAnnotationConstraint(node);
        if (constraint == Constraint::BlockDuplication) {
            result.blocked = "@Owner(Single) blocks duplication (would create alias)";
            return result;
        }
        if (constraint == Constraint::BlockDynamicAlloc) {
            result.blocked = "@Allocate(Static) blocks duplication (would require dynamic allocation)";
            return result;
        }

        result.applied = true;
        result.nodesModified = 1;
        return result;
    }

    OptResult optimizeFunction(const std::string& functionId) {
        OptResult result{false, "", "", 0};
        auto* fn = findNode(root_, functionId);
        if (!fn) return result;

        auto constraint = getAnnotationConstraint(fn);
        if (constraint == Constraint::BlockDuplication) {
            result.blocked = "@Owner(Single) blocks optimization (aliasing risk)";
            return result;
        }
        if (constraint == Constraint::BlockReorder) {
            result.blocked = "@Deallocate(Explicit) blocks optimization (reorder risk)";
            return result;
        }
        if (constraint == Constraint::BlockDynamicAlloc) {
            result.blocked = "@Allocate(Static) blocks optimization (dynamic alloc risk)";
            return result;
        }

        // FreeRestructure or no constraint — proceed
        result.applied = true;
        result.nodesModified = 1;
        return result;
    }

private:
    ASTNode* root_ = nullptr;

    enum class Constraint {
        None,
        FreeRestructure,    // @Reclaim(Tracing) — anything goes
        BlockDuplication,   // @Owner(Single) — no aliasing
        BlockReorder,       // @Deallocate(Explicit) — no reordering around dealloc
        BlockDynamicAlloc   // @Allocate(Static) — no dynamic allocation
    };

    // Find the dominant annotation constraint for a node (check self + ancestors)
    Constraint getAnnotationConstraint(ASTNode* node) {
        // Check the node itself and its ancestors for annotations
        ASTNode* current = node;
        while (current) {
            auto annotations = current->getChildren("annotations");
            for (auto* anno : annotations) {
                if (anno->conceptType == "OwnerAnnotation") {
                    auto* oa = static_cast<OwnerAnnotation*>(anno);
                    if (oa->strategy == "Single") return Constraint::BlockDuplication;
                }
                if (anno->conceptType == "DeallocateAnnotation") {
                    auto* da = static_cast<DeallocateAnnotation*>(anno);
                    if (da->strategy == "Explicit") return Constraint::BlockReorder;
                }
                if (anno->conceptType == "AllocateAnnotation") {
                    auto* aa = static_cast<AllocateAnnotation*>(anno);
                    if (aa->strategy == "Static") return Constraint::BlockDynamicAlloc;
                }
                if (anno->conceptType == "ReclaimAnnotation") {
                    auto* ra = static_cast<ReclaimAnnotation*>(anno);
                    if (ra->strategy == "Tracing") return Constraint::FreeRestructure;
                }
            }
            current = current->parent;
        }
        return Constraint::None;
    }

    ASTNode* findNode(ASTNode* node, const std::string& id) {
        if (!node) return nullptr;
        if (node->id == id) return node;
        for (auto* child : node->allChildren()) {
            auto* found = findNode(child, id);
            if (found) return found;
        }
        return nullptr;
    }
};
