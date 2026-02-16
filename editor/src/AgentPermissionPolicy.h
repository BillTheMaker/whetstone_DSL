#pragma once
// Step 157: Agent role permissions

#include <string>
#include <algorithm>

enum class AgentRole {
    Linter = 0,
    Refactor = 1,
    Generator = 2
};

struct AgentPermissionPolicy {
    static const char* roleLabel(AgentRole role) {
        switch (role) {
            case AgentRole::Linter: return "Linter";
            case AgentRole::Refactor: return "Refactor";
            case AgentRole::Generator: return "Generator";
        }
        return "Linter";
    }

    static AgentRole roleFromString(const std::string& role) {
        std::string lower = role;
        std::transform(lower.begin(), lower.end(), lower.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
        if (lower == "refactor") return AgentRole::Refactor;
        if (lower == "generator") return AgentRole::Generator;
        return AgentRole::Linter;
    }

    static bool canInvoke(AgentRole role, const std::string& method) {
        // Read-only methods: all roles
        if (method == "getAST" ||
            method == "getAnnotationSuggestions" ||
            method == "recordAnnotationFeedback" ||
            method == "setAgentRole" ||
            method == "getInScopeSymbols" ||
            method == "getCallHierarchy" ||
            method == "getDependencyGraph" ||
            method == "runPipeline" ||
            method == "parseSource" ||
            method == "generateFromAST" ||
            method == "projectLanguage" ||
            method == "fileRead" ||
            method == "workspaceList" ||
            method == "fileDiff" ||
            method == "getASTSubtree" ||
            method == "getASTDiff" ||
            method == "getDiagnostics" ||
            method == "getDiagnosticsDelta" ||
            method == "getQuickFixes" ||
            method == "batchQuery" ||
            method == "listBuffers" ||
            method == "setActiveBuffer" ||
            method == "indexWorkspace" ||
            method == "getImportGraph" ||
            method == "getProjectDiagnostics" ||
            method == "searchProject" ||
            method == "loadAnnotatedAST" ||
            method == "listAnnotatedFiles" ||
            method == "getSemanticAnnotations" ||
            method == "getUnannotatedNodes" ||
            method == "getEnvironment" ||
            method == "validateEnvironment" ||
            method == "getLoweringHints" ||
            method == "getProjectModel" ||
            method == "inferAnnotations" ||
            method == "getWorkflowState" ||
            method == "getReadyTasks" ||
            method == "getWorkItem" ||
            method == "getRoutingExplanation" ||
            method == "getReviewPolicy" ||
            method == "getBlockers" ||
            method == "getProgress" ||
            method == "getEventStream" ||
            method == "getRecentEvents") {
            return true;
        }

        // Mutation methods: Refactor and Generator only
        if (method == "generateCode" ||
            method == "applyMutation" ||
            method == "applyAnnotationSuggestion" ||
            method == "applyBatch" ||
            method == "fileWrite" ||
            method == "fileCreate" ||
            method == "applyQuickFix" ||
            method == "openFile" ||
            method == "closeFile" ||
            method == "renameSymbol" ||
            method == "saveBuffer" ||
            method == "saveAllBuffers" ||
            method == "undo" ||
            method == "redo" ||
            method == "saveAnnotatedAST" ||
            method == "setSemanticAnnotation" ||
            method == "removeSemanticAnnotation" ||
            method == "setEnvironment" ||
            method == "createSkeleton" ||
            method == "addSkeletonNode" ||
            method == "createWorkflow" ||
            method == "assignTask" ||
            method == "completeTask" ||
            method == "rejectTask" ||
            method == "saveWorkflow" ||
            method == "routeTask" ||
            method == "routeAllReady" ||
            method == "executeTask" ||
            method == "setReviewPolicy" ||
            method == "orchestrateStep" ||
            method == "orchestrateAdvance" ||
            method == "orchestrateRunDeterministic" ||
            method == "submitExternalResult") {
            return role == AgentRole::Refactor || role == AgentRole::Generator;
        }

        return true;
    }

    static bool canMutate(AgentRole role) {
        return role == AgentRole::Refactor || role == AgentRole::Generator;
    }
};
