#pragma once
// Step 300: Extended transform engine
//
// Adds float constant folding, dead variable elimination,
// and annotation-aware optimization to the base TransformEngine.

#include <string>
#include <vector>
#include <cstdlib>
#include "TransformEngine.h"
#include "ast/ASTNode.h"
#include "ast/Expression.h"
#include "ast/Variable.h"
#include "ast/Function.h"
#include "ast/Annotation.h"

class TransformEngineExtended : public TransformEngine {
public:
    // Float constant folding: folds BinaryOperations on FloatLiterals
    TransformResult floatConstantFolding() {
        TransformResult result{false, "", 0};
        if (!root_) return result;
        foldFloatConstants(root_, result);
        return result;
    }

    // Dead variable elimination: removes unused variable declarations
    TransformResult deadVariableElimination() {
        TransformResult result{false, "", 0};
        if (!root_) return result;
        eliminateDeadVariables(root_, result);
        return result;
    }

    // Annotation-aware: skip nodes with @OptimizationLock, preserve @BoundsCheck
    TransformResult annotationAwareOptimize() {
        TransformResult result{false, "", 0};
        if (!root_) return result;

        // Check for OptimizationLock
        if (hasAnnotationType(root_, "OptimizationLock")) {
            result.warning = "Optimization skipped: OptimizationLock present";
            return result;
        }

        // Apply folding, skip BoundsCheck-protected nodes
        foldConstantsSkipProtected(root_, result);
        return result;
    }

    // Apply all extended transforms
    TransformResult applyAllExtended() {
        TransformResult combined{false, "", 0};

        auto r1 = floatConstantFolding();
        auto r2 = deadVariableElimination();
        auto r3 = annotationAwareOptimize();

        combined.applied = r1.applied || r2.applied || r3.applied;
        combined.nodesModified = r1.nodesModified + r2.nodesModified + r3.nodesModified;
        if (!r3.warning.empty()) combined.warning = r3.warning;

        return combined;
    }

protected:
    // Extended root pointer accessible in this class
    ASTNode* root_ = nullptr;

public:
    void setRoot(ASTNode* root) {
        root_ = root;
        TransformEngine::setRoot(root);
    }

private:
    // Recursively fold constant BinaryOperations on FloatLiterals (bottom-up)
    void foldFloatConstants(ASTNode* node, TransformResult& result) {
        if (!node) return;

        // Process children first (bottom-up) so nested folds work
        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (auto* child : children) {
                foldFloatConstants(child, result);
            }
        }

        // Check each child role for foldable BinaryOperations on float operands
        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (size_t i = 0; i < children.size(); ++i) {
                auto* child = children[i];
                if (child->conceptType == "BinaryOperation") {
                    auto* binOp = static_cast<BinaryOperation*>(child);
                    auto* left = binOp->getChild("left");
                    auto* right = binOp->getChild("right");

                    if (left && right &&
                        left->conceptType == "FloatLiteral" &&
                        right->conceptType == "FloatLiteral") {

                        double lv = std::stod(static_cast<FloatLiteral*>(left)->value);
                        double rv = std::stod(static_cast<FloatLiteral*>(right)->value);
                        double res = 0;

                        if (binOp->op == "+") res = lv + rv;
                        else if (binOp->op == "-") res = lv - rv;
                        else if (binOp->op == "*") res = lv * rv;
                        else if (binOp->op == "/" && rv != 0) res = lv / rv;
                        else continue;

                        auto* replacement = new FloatLiteral();
                        replacement->id = "folded_" + binOp->id;
                        replacement->value = std::to_string(res);
                        node->setChild(role, replacement);

                        result.applied = true;
                        result.nodesModified++;
                    }
                }
            }
        }
    }

    // Remove variable declarations that are never referenced in the function body
    void eliminateDeadVariables(ASTNode* node, TransformResult& result) {
        if (!node) return;

        if (node->conceptType == "Function") {
            auto body = node->getChildren("body");

            // Collect declared variable names
            std::vector<std::string> declaredVars;
            for (auto* stmt : body) {
                if (stmt->conceptType == "Variable") {
                    declaredVars.push_back(static_cast<Variable*>(stmt)->name);
                }
            }

            // Check if each variable is referenced anywhere in the body
            for (const auto& varName : declaredVars) {
                bool used = false;
                for (auto* stmt : body) {
                    if (stmt->conceptType != "Variable" && hasReference(stmt, varName)) {
                        used = true;
                        break;
                    }
                }
                if (!used) {
                    // Remove the variable declaration
                    for (auto* stmt : body) {
                        if (stmt->conceptType == "Variable" &&
                            static_cast<Variable*>(stmt)->name == varName) {
                            node->removeChild(stmt);
                            result.applied = true;
                            result.nodesModified++;
                            break;
                        }
                    }
                }
            }
        }

        for (auto* child : node->allChildren()) {
            eliminateDeadVariables(child, result);
        }
    }

    // Check if a subtree contains a VariableReference to the given name
    bool hasReference(const ASTNode* node, const std::string& name) const {
        if (!node) return false;
        if (node->conceptType == "VariableReference") {
            if (static_cast<const VariableReference*>(node)->variableName == name)
                return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasReference(child, name)) return true;
        }
        return false;
    }

    // Recursively check if any node in the subtree has the given annotation type
    bool hasAnnotationType(const ASTNode* node, const std::string& type) const {
        if (!node) return false;
        for (const auto* anno : node->getChildren("annotations")) {
            if (anno->conceptType == type) return true;
        }
        for (auto* child : node->allChildren()) {
            if (hasAnnotationType(child, type)) return true;
        }
        return false;
    }

    // Fold constants but skip nodes protected by BoundsCheck or OptimizationLock
    void foldConstantsSkipProtected(ASTNode* node, TransformResult& result) {
        if (!node) return;

        // Skip nodes with BoundsCheck or OptimizationLock annotations
        for (const auto* anno : node->getChildren("annotations")) {
            if (anno->conceptType == "BoundsCheckAnnotation" ||
                anno->conceptType == "OptimizationLock")
                return;
        }

        for (const auto& role : node->childRoles()) {
            auto children = node->getChildren(role);
            for (auto* child : children) {
                foldConstantsSkipProtected(child, result);
            }
        }
    }
};
