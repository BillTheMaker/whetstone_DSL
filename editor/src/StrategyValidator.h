#pragma once
#include <string>
#include <vector>
#include <set>
#include <map>
#include "ast/ASTNode.h"
#include "ast/Annotation.h"
#include "ast/Expression.h"
#include "ast/Statement.h"

class StrategyValidator {
public:
    struct Violation {
        std::string severity;   // "error" or "warning"
        std::string category;   // "use-after-free", "leak", "double-move", "aliasing", "destructor-unreachable"
        std::string message;
        std::string nodeId;
    };

    std::vector<Violation> validateInvariants(const ASTNode* root) const {
        std::vector<Violation> violations;
        validateNode(root, violations);
        return violations;
    }

private:
    void validateNode(const ASTNode* node, std::vector<Violation>& violations) const {
        if (node->conceptType == "Function") {
            validateFunction(node, violations);
        }
        for (auto* child : node->allChildren()) {
            validateNode(child, violations);
        }
    }

    void validateFunction(const ASTNode* fn, std::vector<Violation>& violations) const {
        auto annotations = fn->getChildren("annotations");
        for (auto* anno : annotations) {
            if (anno->conceptType == "DeallocateAnnotation") {
                auto* da = static_cast<DeallocateAnnotation*>(anno);
                if (da->strategy == "Explicit") {
                    validateExplicitDealloc(fn, violations);
                }
            }
            if (anno->conceptType == "OwnerAnnotation") {
                auto* oa = static_cast<OwnerAnnotation*>(anno);
                if (oa->strategy == "Single") {
                    validateSingleOwnership(fn, violations);
                }
            }
            if (anno->conceptType == "LifetimeAnnotation") {
                auto* la = static_cast<LifetimeAnnotation*>(anno);
                if (la->strategy == "RAII") {
                    validateRAII(fn, violations);
                }
            }
        }
    }

    void validateExplicitDealloc(const ASTNode* fn, std::vector<Violation>& violations) const {
        auto body = fn->getChildren("body");

        // Collect declared variables and track delete/free calls
        std::set<std::string> declaredVars;
        std::set<std::string> freedVars;
        std::vector<std::pair<std::string, std::string>> usesAfterFree; // (varName, nodeId)

        // Scan body in order
        for (auto* stmt : body) {
            if (stmt->conceptType == "Variable") {
                auto* var = static_cast<Variable*>(stmt);
                declaredVars.insert(var->name);
            }

            // Check for delete/free calls
            std::string deletedVar = getDeleteTarget(stmt);
            if (!deletedVar.empty()) {
                freedVars.insert(deletedVar);
            }

            // Check for uses of already-freed variables
            if (!freedVars.empty()) {
                std::vector<std::string> refs;
                collectVarRefs(stmt, refs);
                for (const auto& ref : refs) {
                    // Skip the delete statement itself
                    if (deletedVar == ref) continue;
                    if (freedVars.count(ref)) {
                        usesAfterFree.push_back({ref, stmt->id});
                    }
                }
            }
        }

        // Report use-after-free
        for (const auto& [varName, nodeId] : usesAfterFree) {
            violations.push_back({
                "error", "use-after-free",
                "Use of '" + varName + "' after deallocation",
                nodeId
            });
        }

        // Report leaks: declared vars without corresponding free
        for (const auto& varName : declaredVars) {
            if (freedVars.find(varName) == freedVars.end()) {
                violations.push_back({
                    "error", "leak",
                    "Variable '" + varName + "' allocated but never deallocated under @Deallocate(Explicit)",
                    fn->id
                });
            }
        }
    }

    void validateSingleOwnership(const ASTNode* fn, std::vector<Violation>& violations) const {
        auto body = fn->getChildren("body");

        // Track how many times each variable is used as an assignment source
        std::map<std::string, int> assignmentSources;

        for (auto* stmt : body) {
            if (stmt->conceptType == "Assignment") {
                auto* value = stmt->getChild("value");
                if (value && value->conceptType == "VariableReference") {
                    auto* ref = static_cast<VariableReference*>(value);
                    assignmentSources[ref->variableName]++;
                }
            }
        }

        // If any variable is assigned to more than one target, it's aliasing
        for (const auto& [varName, count] : assignmentSources) {
            if (count > 1) {
                violations.push_back({
                    "error", "aliasing",
                    "Variable '" + varName + "' aliased multiple times under @Owner(Single)",
                    fn->id
                });
            }
        }
    }

    void validateRAII(const ASTNode* fn, std::vector<Violation>& violations) const {
        // RAII: check that variables with destructors are reachable on all paths
        // For now, basic check: no early return without scope cleanup
        // (simplified — just a placeholder for the invariant check)
    }

    // Extract the variable name being deleted/freed from a statement
    std::string getDeleteTarget(const ASTNode* stmt) const {
        if (stmt->conceptType == "ExpressionStatement") {
            auto* expr = stmt->getChild("expression");
            if (expr && expr->conceptType == "FunctionCall") {
                auto* call = static_cast<FunctionCall*>(expr);
                if (call->functionName == "delete" || call->functionName == "free") {
                    auto args = call->getChildren("arguments");
                    if (!args.empty() && args[0]->conceptType == "VariableReference") {
                        return static_cast<VariableReference*>(args[0])->variableName;
                    }
                }
            }
        }
        return "";
    }

    // Collect all variable references in a subtree
    void collectVarRefs(const ASTNode* node, std::vector<std::string>& refs) const {
        if (node->conceptType == "VariableReference") {
            refs.push_back(static_cast<const VariableReference*>(node)->variableName);
        }
        for (auto* child : node->allChildren()) {
            collectVarRefs(child, refs);
        }
    }
};
