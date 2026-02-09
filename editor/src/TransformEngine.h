#pragma once
#include <string>
#include <vector>
#include <functional>
#include "ast/ASTNode.h"
#include "ast/Expression.h"
#include "ast/Statement.h"
#include "ast/Annotation.h"

class TransformEngine {
public:
    struct TransformResult {
        bool applied;
        std::string warning;
        int nodesModified;
    };

    void setRoot(ASTNode* root) { root_ = root; }

    TransformResult constantFolding() {
        TransformResult result{false, "", 0};
        if (!root_) return result;
        std::string lockWarning = checkOptimizationLock(root_);
        foldConstants(root_, result);
        if (!lockWarning.empty() && result.applied) {
            result.warning = lockWarning;
        }
        return result;
    }

    TransformResult deadCodeElimination() {
        TransformResult result{false, "", 0};
        if (!root_) return result;
        eliminateDeadCode(root_, result);
        return result;
    }

    std::vector<TransformResult> applyAll() {
        std::vector<TransformResult> results;
        results.push_back(constantFolding());
        results.push_back(deadCodeElimination());
        return results;
    }

private:
    ASTNode* root_ = nullptr;

    // Recursively fold constant BinaryOperations on IntegerLiterals
    void foldConstants(ASTNode* node, TransformResult& result) {
        // Process children first (bottom-up) so nested folds work
        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (auto* child : children) {
                foldConstants(child, result);
            }
        }

        // Check if this node's parent has a child that is a foldable BinaryOperation
        // We look at each child role for BinaryOperations we can fold
        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (size_t i = 0; i < children.size(); ++i) {
                auto* child = children[i];
                if (child->conceptType == "BinaryOperation") {
                    auto* binOp = static_cast<BinaryOperation*>(child);
                    auto* leftNode = binOp->getChild("left");
                    auto* rightNode = binOp->getChild("right");

                    if (leftNode && rightNode &&
                        leftNode->conceptType == "IntegerLiteral" &&
                        rightNode->conceptType == "IntegerLiteral") {

                        auto* leftLit = static_cast<IntegerLiteral*>(leftNode);
                        auto* rightLit = static_cast<IntegerLiteral*>(rightNode);
                        int foldedValue = evaluateOp(binOp->op, leftLit->value, rightLit->value);

                        auto* replacement = new IntegerLiteral("folded_" + binOp->id, foldedValue);
                        node->setChild(role, replacement);

                        result.applied = true;
                        result.nodesModified++;
                    }
                }
            }
        }
    }

    int evaluateOp(const std::string& op, int left, int right) {
        if (op == "+") return left + right;
        if (op == "-") return left - right;
        if (op == "*") return left * right;
        if (op == "/" && right != 0) return left / right;
        if (op == "%") return left % right;
        return 0;
    }

    // Remove statements after Return in function bodies
    void eliminateDeadCode(ASTNode* node, TransformResult& result) {
        if (node->conceptType == "Function") {
            auto body = node->getChildren("body");
            bool foundReturn = false;
            std::vector<ASTNode*> toRemove;

            for (auto* stmt : body) {
                if (foundReturn) {
                    toRemove.push_back(stmt);
                }
                if (stmt->conceptType == "Return") {
                    foundReturn = true;
                }
            }

            for (auto* dead : toRemove) {
                node->removeChild(dead);
                result.applied = true;
                result.nodesModified++;
            }
        }

        // Recurse into children
        for (auto* child : node->allChildren()) {
            eliminateDeadCode(child, result);
        }
    }

    // Walk ancestors for OptimizationLock annotations
    std::string checkOptimizationLock(ASTNode* node) {
        std::string warning;
        checkOptimizationLockRecursive(node, warning);
        return warning;
    }

    void checkOptimizationLockRecursive(ASTNode* node, std::string& warning) {
        if (node->conceptType == "OptimizationLock") {
            auto* lock = static_cast<OptimizationLock*>(node);
            if (lock->lockLevel == "warning") {
                warning = "OptimizationLock: " + lock->lockReason +
                          " (locked by " + lock->lockedBy + ")";
            }
        }
        for (auto* child : node->allChildren()) {
            checkOptimizationLockRecursive(child, warning);
        }
    }
};
