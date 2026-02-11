#pragma once
// Step 245: Headless EditorState — agent API surface without ImGui/SDL.
//
// Provides the same method interface that AgentRPCHandler.h expects,
// but with zero GUI dependencies. Uses std::chrono for timestamps,
// stderr for logging, and holds its own lightweight buffer map.

// --- Core AST and pipeline (no ImGui) ---
#include "ast/ASTNode.h"
#include "ast/Serialization.h"
#include "ast/Generator.h"
#include "ast/Annotation.h"
#include "ASTUtils.h"
#include "Pipeline.h"
#include "ASTMutationAPI.h"
#include "BatchMutationAPI.h"
#include "ContextAPI.h"
#include "CrossLanguageProjector.h"
#include "MemoryStrategyInference.h"
#include "AgentAnnotationAssistant.h"
#include "AgentPermissionPolicy.h"
#include "AgentCodeGen.h"
#include "AgentLibraryPolicy.h"
#include "WorkflowRecorder.h"
#include "FileOperations.h"
#include "CompactAST.h"
#include "StructuredDiagnostics.h"
#include "Orchestrator.h"
#include "IncrementalOptimizer.h"
#include "TextASTSync.h"
#include "BufferManager.h"
#include "EditorModePolicy.h"
#include "PrimitivesRegistry.h"
#include "SemanticTags.h"

#include <nlohmann/json.hpp>
#include <string>
#include <map>
#include <memory>
#include <chrono>
#include <iostream>

using json = nlohmann::json;

// -----------------------------------------------------------------------
//  Headless agent state — no WebSocket, no AgentMarketplace (ImGui dep)
// -----------------------------------------------------------------------
struct HeadlessAgentState {
    std::map<std::string, AgentRole> roles;
    AgentRole defaultRole = AgentRole::Linter;
    WorkflowRecorder workflowRecorder;
};

// -----------------------------------------------------------------------
//  Headless library state — no ImGui panels
// -----------------------------------------------------------------------
struct HeadlessLibraryState {
    PrimitivesRegistry primitives;
    SemanticTags        semanticTags;
};

// -----------------------------------------------------------------------
//  HeadlessBufferState — lightweight buffer without ImGui widgets
// -----------------------------------------------------------------------
struct HeadlessBufferState {
    TextASTSync          sync;
    Orchestrator         orchestrator;
    IncrementalOptimizer incrementalOptimizer;
    ASTVersionTracker    versionTracker;
    std::string          language    = "python";
    std::string          path        = "(untitled)";
    std::string          editBuf;
    BufferManager::BufferMode bufferMode =
        BufferManager::BufferMode::Structured;
    bool                 orchestratorDirty = true;
    bool                 modified    = false;
};

// -----------------------------------------------------------------------
//  HeadlessEditorState — agent-facing state, no GUI
// -----------------------------------------------------------------------
struct HeadlessEditorState {
    std::map<std::string, std::unique_ptr<HeadlessBufferState>>
        bufferStates;
    HeadlessBufferState* activeBuffer = nullptr;
    HeadlessAgentState   agent;
    HeadlessLibraryState library;
    std::string          workspaceRoot;
    std::string          defaultLanguage = "python";
    bool                 verbose = false;

    // --- Buffer access ---

    HeadlessBufferState* active() { return activeBuffer; }

    bool isStructured() const {
        return activeBuffer &&
               allowStructuredFeatures(activeBuffer->bufferMode);
    }

    Module* activeAST() {
        if (!activeBuffer) return nullptr;
        if (!allowStructuredFeatures(activeBuffer->bufferMode))
            return nullptr;
        return activeBuffer->sync.getAST();
    }

    Module* mutationAST() {
        if (!activeBuffer) return nullptr;
        if (!allowStructuredFeatures(activeBuffer->bufferMode))
            return nullptr;
        if (activeBuffer->orchestratorDirty ||
            !activeBuffer->orchestrator.getAST()) {
            syncOrchestratorFromActive();
        }
        return activeBuffer->orchestrator.getAST();
    }

    void syncOrchestratorFromActive() {
        if (!activeBuffer) return;
        Module* ast = activeBuffer->sync.getAST();
        if (ast) {
            activeBuffer->orchestrator.setAST(cloneModule(ast));
            activeBuffer->orchestratorDirty = false;
        }
    }

    void applyOrchestratorToActive() {
        if (!activeBuffer) return;
        if (!isStructured()) return;
        Module* ast = activeBuffer->orchestrator.getAST();
        if (!ast) return;
        activeBuffer->sync.setAST(cloneModule(ast));
        activeBuffer->incrementalOptimizer.setRoot(
            activeBuffer->sync.getAST());
        Pipeline pipeline;
        std::string code = pipeline.generate(
            activeBuffer->sync.getAST(), activeBuffer->language);
        activeBuffer->editBuf = code;
        activeBuffer->orchestratorDirty = false;
    }

    // --- Agent role management ---

    AgentRole getAgentRole(const std::string& sessionId) const {
        auto it = agent.roles.find(sessionId);
        if (it != agent.roles.end()) return it->second;
        return agent.defaultRole;
    }

    void setAgentRole(const std::string& sessionId, AgentRole role) {
        agent.roles[sessionId] = role;
    }

    std::string agentActorLabel(const std::string& sessionId) const {
        AgentRole role = getAgentRole(sessionId);
        return std::string(AgentPermissionPolicy::roleLabel(role)) +
               ":" + sessionId;
    }

    // --- Diagnostics ---

    json buildDiagnosticsJson() const {
        return json::array();
    }

    json buildSessionMetadata(bool /*autoRecording*/) const {
        return {
            {"mode", "headless"},
            {"workspace", workspaceRoot},
            {"language", defaultLanguage},
            {"timestamp",
             std::chrono::duration_cast<std::chrono::seconds>(
                 std::chrono::system_clock::now().time_since_epoch())
                 .count()}
        };
    }

    // --- Buffer operations ---

    HeadlessBufferState* openBuffer(const std::string& filePath,
                                     const std::string& content,
                                     const std::string& lang) {
        auto buf = std::make_unique<HeadlessBufferState>();
        buf->path = filePath;
        buf->language = lang.empty() ? defaultLanguage : lang;
        buf->editBuf = content;
        buf->bufferMode = BufferManager::BufferMode::Structured;
        if (!content.empty()) {
            Pipeline pipeline;
            std::vector<ParseDiagnostic> diags;
            auto mod = pipeline.parse(content, buf->language, diags);
            if (mod) {
                buf->sync.setAST(std::move(mod));
                buf->orchestratorDirty = true;
            }
        } else {
            auto mod = std::make_unique<Module>(
                "root", filePath, buf->language);
            buf->sync.setAST(std::move(mod));
            buf->orchestratorDirty = true;
        }
        HeadlessBufferState* raw = buf.get();
        bufferStates[filePath] = std::move(buf);
        if (!activeBuffer) activeBuffer = raw;
        return raw;
    }

    bool setActiveBuffer(const std::string& path) {
        auto it = bufferStates.find(path);
        if (it == bufferStates.end()) return false;
        activeBuffer = it->second.get();
        return true;
    }

    void closeBuffer(const std::string& path) {
        auto it = bufferStates.find(path);
        if (it == bufferStates.end()) return;
        if (activeBuffer == it->second.get())
            activeBuffer = nullptr;
        bufferStates.erase(it);
        if (!activeBuffer && !bufferStates.empty())
            activeBuffer = bufferStates.begin()->second.get();
    }

    // --- RPC entry point ---
    json processAgentRequest(const json& request,
                             const std::string& sessionId);

    // --- Logging ---
    void log(const std::string& msg) const {
        if (verbose)
            std::cerr << "[whetstone-headless] " << msg << "\n";
    }
};

// Include the headless RPC handler and wire up processAgentRequest
#include "HeadlessAgentRPCHandler.h"

inline json HeadlessEditorState::processAgentRequest(
    const json& request, const std::string& sessionId) {
    return handleHeadlessAgentRequest(*this, request, sessionId);
}
