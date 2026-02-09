#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "BatchMutationAPI.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Statement.h"
#include "ast/Expression.h"

struct RefactorPlan {
    bool success = false;
    std::string error;
    std::vector<BatchMutationAPI::Mutation> mutations;
};

inline std::string nextRefactorId(const std::string& prefix) {
    static int counter = 0;
    return prefix + "_" + std::to_string(counter++);
}

inline void collectVariableRefs(ASTNode* node, const std::string& name,
                                std::vector<ASTNode*>& outRefs) {
    if (!node) return;
    if (node->conceptType == "VariableReference") {
        auto* vr = static_cast<VariableReference*>(node);
        if (vr->variableName == name) outRefs.push_back(node);
    }
    for (auto* child : node->allChildren()) {
        collectVariableRefs(child, name, outRefs);
    }
}

inline void collectVariables(ASTNode* node, const std::string& name,
                             std::vector<ASTNode*>& outVars) {
    if (!node) return;
    if (node->conceptType == "Variable") {
        auto* v = static_cast<Variable*>(node);
        if (v->name == name) outVars.push_back(node);
    }
    for (auto* child : node->allChildren()) {
        collectVariables(child, name, outVars);
    }
}

inline RefactorPlan buildRenameVariablePlan(Module* ast,
                                            const std::string& oldName,
                                            const std::string& newName) {
    RefactorPlan plan;
    if (!ast) { plan.error = "No AST"; return plan; }
    if (oldName.empty() || newName.empty()) {
        plan.error = "Variable names cannot be empty";
        return plan;
    }

    std::vector<ASTNode*> vars;
    std::vector<ASTNode*> refs;
    collectVariables(ast, oldName, vars);
    collectVariableRefs(ast, oldName, refs);
    if (vars.empty() && refs.empty()) {
        plan.error = "No matching variable or references";
        return plan;
    }

    for (auto* v : vars) {
        BatchMutationAPI::Mutation mut;
        mut.type = "setProperty";
        mut.nodeId = v->id;
        mut.property = "name";
        mut.value = newName;
        plan.mutations.push_back(mut);
    }
    for (auto* r : refs) {
        BatchMutationAPI::Mutation mut;
        mut.type = "setProperty";
        mut.nodeId = r->id;
        mut.property = "variableName";
        mut.value = newName;
        plan.mutations.push_back(mut);
    }

    plan.success = true;
    return plan;
}

inline RefactorPlan buildExtractFunctionPlan(Module* ast,
                                             const std::string& newFunctionName) {
    RefactorPlan plan;
    if (!ast) { plan.error = "No AST"; return plan; }
    if (newFunctionName.empty()) {
        plan.error = "Function name cannot be empty";
        return plan;
    }

    auto funcs = ast->getChildren("functions");
    if (funcs.empty()) { plan.error = "No functions to extract from"; return plan; }
    auto* srcFn = static_cast<Function*>(funcs.front());
    auto body = srcFn->getChildren("body");
    if (body.empty()) { plan.error = "Source function has no body"; return plan; }

    ASTNode* stmt = body.front();
    auto* newFn = new Function(nextRefactorId("fn"), newFunctionName);
    auto* call = new FunctionCall();
    call->id = nextRefactorId("call");
    call->functionName = newFunctionName;
    auto* callStmt = new ExpressionStatement();
    callStmt->id = nextRefactorId("stmt");
    callStmt->setChild("expression", call);

    BatchMutationAPI::Mutation insertFn;
    insertFn.type = "insertNode";
    insertFn.parentId = ast->id;
    insertFn.role = "functions";
    insertFn.newNode = newFn;
    plan.mutations.push_back(insertFn);

    BatchMutationAPI::Mutation deleteStmt;
    deleteStmt.type = "deleteNode";
    deleteStmt.nodeId = stmt->id;
    plan.mutations.push_back(deleteStmt);

    BatchMutationAPI::Mutation insertStmt;
    insertStmt.type = "insertNode";
    insertStmt.parentId = newFn->id;
    insertStmt.role = "body";
    insertStmt.newNode = stmt;
    plan.mutations.push_back(insertStmt);

    BatchMutationAPI::Mutation insertCall;
    insertCall.type = "insertNode";
    insertCall.parentId = srcFn->id;
    insertCall.role = "body";
    insertCall.newNode = callStmt;
    plan.mutations.push_back(insertCall);

    plan.success = true;
    return plan;
}

inline RefactorPlan buildInlineVariablePlan(Module* ast,
                                            const std::string& varName) {
    RefactorPlan plan;
    if (!ast) { plan.error = "No AST"; return plan; }
    if (varName.empty()) { plan.error = "Variable name cannot be empty"; return plan; }

    std::vector<ASTNode*> vars;
    collectVariables(ast, varName, vars);
    if (vars.empty()) {
        plan.error = "Variable not found";
        return plan;
    }

    BatchMutationAPI::Mutation del;
    del.type = "deleteNode";
    del.nodeId = vars.front()->id;
    plan.mutations.push_back(del);

    plan.success = true;
    return plan;
}
