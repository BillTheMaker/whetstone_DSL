#pragma once
// Step 245: Headless Agent RPC Handler
//
// Mirrors AgentRPCHandler.h but operates on HeadlessEditorState.
// Uses the same agentRpcError/agentRpcResult helpers and supports
// the same JSON-RPC methods for the agent-critical path:
//   getAST, parseSource, generateFromAST, runPipeline, projectLanguage,
//   applyMutation, applyBatch, getInScopeSymbols, getCallHierarchy,
//   getDependencyGraph, generateCode, setAgentRole,
//   getAnnotationSuggestions, applyAnnotationSuggestion,
//   recordAnnotationFeedback, startWorkflowRecording,
//   stopWorkflowRecording, getWorkflowRecording, replayWorkflow, ping
//
// Deliberately omits GUI-only methods. Stays under 600 lines.

struct HeadlessEditorState;
#include "HeadlessOrchestratorRPC.h"

// Re-use the response helpers (same signature as AgentRPCHandler.h)
static inline json headlessRpcError(const json& id, int code,
                                     const std::string& msg) {
    return {{"jsonrpc", "2.0"}, {"id", id},
            {"error", {{"code", code}, {"message", msg}}}};
}

static inline json headlessRpcResult(const json& id, const json& result) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

static inline json headlessRequireAST(HeadlessEditorState& state,
                                       const json& id) {
    if (!state.active() || !state.isStructured())
        return headlessRpcError(id, -32000, "No structured buffer");
    if (!state.activeAST())
        return headlessRpcError(id, -32001, "AST unavailable");
    return json();
}

static inline json headlessRequireMutable(HeadlessEditorState& state,
                                           const json& id) {
    if (!state.active() || !state.isStructured())
        return headlessRpcError(id, -32000, "No structured buffer");
    if (!state.mutationAST())
        return headlessRpcError(id, -32001, "AST unavailable");
    return json();
}

// -----------------------------------------------------------------------
//  Main dispatch
// -----------------------------------------------------------------------
inline json handleHeadlessAgentRequest(HeadlessEditorState& state,
                                        const json& request,
                                        const std::string& sessionId) {
    json id = request.contains("id") ? request["id"] : json(nullptr);
    std::string method = request.value("method", "");
    AgentRole role = state.getAgentRole(sessionId);

    // --- ping ---
    if (method == "ping")
        return headlessRpcResult(id, "pong");

    // --- getAST ---
    if (method == "getAST") {
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        bool compact = params.value("compact", false);
        json result;
        if (compact) {
            json nodes = toJsonCompactSummary(state.activeAST());
            result = {{"nodes", nodes}, {"nodeCount", (int)nodes.size()},
                      {"totalNodes", (int)toJsonCompactTree(
                          state.activeAST()).size()}};
        } else {
            result = {
                {"ast", toJson(state.activeAST())},
                {"annotationCount",
                 countAnnotationNodes(state.activeAST())},
                {"diagnostics", state.buildDiagnosticsJson()}
            };
        }
        result["version"] = state.active()->versionTracker.version;
        result["tokenEstimate"] = tokenEstimate(result);
        int budget = params.value("budget", 0);
        if (budget > 0) {
            auto br = applyBudget(result, budget);
            return headlessRpcResult(id, br.result);
        }
        return headlessRpcResult(id, result);
    }

    // --- parseSource ---
    if (method == "parseSource") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string source = params.value("source", "");
        std::string language = params.value("language", "");
        if (source.empty() || language.empty())
            return headlessRpcError(id, -32602,
                                     "Missing source or language");
        Pipeline pipeline;
        std::vector<ParseDiagnostic> diags;
        auto mod = pipeline.parse(source, language, diags);
        json diagArr = json::array();
        for (const auto& d : diags)
            diagArr.push_back({{"line", d.line}, {"column", d.column},
                               {"message", d.message},
                               {"severity", d.severity}});
        json result = {{"diagnostics", diagArr}};
        if (mod) result["ast"] = toJson(mod.get());
        return headlessRpcResult(id, result);
    }

    // --- generateFromAST ---
    if (method == "generateFromAST") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string language = params.value("language",
                                             state.active()->language);
        Pipeline pipeline;
        std::string code = pipeline.generate(state.activeAST(), language);
        return headlessRpcResult(id, {{"code", code},
                                       {"language", language}});
    }

    // --- runPipeline ---
    if (method == "runPipeline") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string source = params.value("source", "");
        std::string srcLang = params.value("sourceLanguage", "");
        std::string tgtLang = params.value("targetLanguage", "");
        if (source.empty() || srcLang.empty() || tgtLang.empty())
            return headlessRpcError(id, -32602,
                "Missing source, sourceLanguage, or targetLanguage");
        Pipeline pipeline;
        auto pr = pipeline.run(source, srcLang, tgtLang);
        json diagArr = json::array();
        for (const auto& d : pr.parseDiags)
            diagArr.push_back({{"line", d.line}, {"column", d.column},
                               {"message", d.message},
                               {"severity", d.severity}});
        json valArr = json::array();
        for (const auto& d : pr.validationDiags)
            valArr.push_back({{"severity", d.severity},
                              {"message", d.message},
                              {"nodeId", d.nodeId}});
        json violArr = json::array();
        for (const auto& v : pr.violations)
            violArr.push_back({{"severity", v.severity},
                               {"category", v.category},
                               {"message", v.message},
                               {"nodeId", v.nodeId}});
        json suggArr = json::array();
        for (const auto& s : pr.suggestions)
            suggArr.push_back({{"nodeId", s.nodeId},
                               {"annotationType", s.annotationType},
                               {"strategy", s.strategy},
                               {"reason", s.reason},
                               {"confidence", s.confidence}});
        json result = {
            {"success", pr.success}, {"generatedCode", pr.generatedCode},
            {"parseDiagnostics", diagArr},
            {"validationDiagnostics", valArr},
            {"violations", violArr}, {"suggestions", suggArr},
            {"foldCount", pr.foldResult.nodesModified},
            {"dceCount", pr.dceResult.nodesModified}
        };
        if (pr.ast) result["ast"] = toJson(pr.ast.get());
        return headlessRpcResult(id, result);
    }

    // --- projectLanguage ---
    if (method == "projectLanguage") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string targetLanguage = params.value("targetLanguage", "");
        if (targetLanguage.empty())
            return headlessRpcError(id, -32602, "Missing targetLanguage");
        CrossLanguageProjector projector;
        auto projected = projector.project(state.activeAST(),
                                            targetLanguage);
        if (!projected)
            return headlessRpcError(id, -32020, "Projection failed");
        Pipeline pipeline;
        std::string code = pipeline.generate(projected.get(),
                                              targetLanguage);
        return headlessRpcResult(id, {
            {"ast", toJson(projected.get())}, {"generatedCode", code},
            {"sourceLanguage", state.active()->language},
            {"targetLanguage", targetLanguage}
        });
    }

    // --- setAgentRole ---
    if (method == "setAgentRole") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string roleName = params.value("role", "linter");
        AgentRole newRole =
            AgentPermissionPolicy::roleFromString(roleName);
        state.setAgentRole(sessionId, newRole);
        return headlessRpcResult(id,
            {{"role", AgentPermissionPolicy::roleLabel(newRole)}});
    }

    // --- generateCode ---
    if (method == "generateCode") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string spec = params.value("spec", "");
        bool preferImports = params.value("preferImports", true);
        state.library.primitives.setRoot(state.activeAST());
        state.library.primitives.setLanguage(state.active()->language);
        AgentCodeGen gen;
        state.library.primitives.setContextTags(
            state.library.semanticTags.inferTagsFromText(spec));
        AgentCodeGenResult genRes = gen.generate(
            spec, state.library.primitives,
            state.active()->language, preferImports);
        if (!genRes.node)
            return headlessRpcError(id, -32020, "Code generation failed");
        json nodeJson = toJson(genRes.node);
        deleteTree(genRes.node);
        return headlessRpcResult(id, {
            {"node", nodeJson}, {"note", genRes.note},
            {"usedSymbols", genRes.usedSymbols},
            {"language", state.active()->language}
        });
    }

    // --- applyMutation ---
    if (method == "applyMutation") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.active() || !state.isStructured())
            return headlessRpcError(id, -32000, "No structured buffer");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string type = params.value("type", "");
        bool preferImports = params.value("preferImports", false);
        bool strictMode = params.value("strictMode", false);
        Module* ast = state.mutationAST();
        if (!ast) return headlessRpcError(id, -32001, "AST unavailable");
        ASTMutationAPI mut;
        mut.setRoot(ast);
        ASTMutationAPI::MutationResult res;
        LibraryPolicyResult policy;
        std::vector<std::string> affectedIds;
        if (type == "setProperty") {
            std::string nodeId = params.value("nodeId", "");
            if (!nodeId.empty()) affectedIds.push_back(nodeId);
            res = mut.setProperty(params.value("nodeId", ""),
                                  params.value("property", ""),
                                  params.value("value", ""));
        } else if (type == "updateNode") {
            std::map<std::string, std::string> props;
            if (params.contains("properties")) {
                for (auto& [k, v] : params["properties"].items())
                    props[k] = v.get<std::string>();
            }
            std::string nodeId = params.value("nodeId", "");
            if (!nodeId.empty()) affectedIds.push_back(nodeId);
            res = mut.updateNode(params.value("nodeId", ""), props);
        } else if (type == "deleteNode") {
            std::string nodeId = params.value("nodeId", "");
            if (!nodeId.empty()) affectedIds.push_back(nodeId);
            res = mut.deleteNode(params.value("nodeId", ""));
        } else if (type == "insertNode") {
            ASTNode* node = nullptr;
            std::string insertedId;
            if (params.contains("node")) {
                node = fromJson(params["node"]);
                if (node) insertedId = node->id;
            }
            if (node) {
                policy = checkMutationLibraryPolicy(
                    node, ast, preferImports, strictMode);
                if (!policy.ok) {
                    deleteTree(node);
                    return headlessRpcError(id, -32011, policy.error);
                }
            }
            res = mut.insertNode(params.value("parentId", ""),
                                 params.value("role", ""), node);
            if (!res.success && node) {
                deleteTree(node);
            } else if (res.success && !insertedId.empty()) {
                affectedIds.push_back(insertedId);
            }
        } else {
            return headlessRpcError(id, -32602, "Unknown mutation type");
        }
        if (!res.success)
            return headlessRpcError(id, -32010, res.error);
        state.applyOrchestratorToActive();
        if (state.active()) {
            state.active()->incrementalOptimizer.setRoot(
                state.active()->sync.getAST());
            state.active()->incrementalOptimizer.recordExternalTransform(
                "agent-mutation:" + type, affectedIds,
                state.agentActorLabel(sessionId));
            state.active()->versionTracker.recordMutation(affectedIds);
            // Record post-mutation state for undo
            state.active()->undoStack.record(
                state.active()->editBuf, state.activeAST());
        }
        return headlessRpcResult(id, {
            {"success", true}, {"warning", res.warning},
            {"libraryWarning", policy.warning},
            {"unknownFunctions", policy.unknownFunctions},
            {"version", state.active()
                ? state.active()->versionTracker.version : 0}
        });
    }

    // --- applyBatch ---
    if (method == "applyBatch") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto errChk = headlessRequireMutable(state, id);
        if (!errChk.is_null()) return errChk;
        Module* ast = state.mutationAST();
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        if (!params.contains("mutations") ||
            !params["mutations"].is_array())
            return headlessRpcError(id, -32602,
                                     "Missing mutations array");
        BatchMutationAPI batch;
        batch.setRoot(ast);
        std::vector<BatchMutationAPI::Mutation> mutations;
        std::vector<ASTNode*> ownedNodes;
        for (const auto& m : params["mutations"]) {
            BatchMutationAPI::Mutation mut;
            mut.type = m.value("type", "");
            mut.nodeId = m.value("nodeId", "");
            mut.property = m.value("property", "");
            mut.value = m.value("value", "");
            mut.parentId = m.value("parentId", "");
            mut.role = m.value("role", "");
            if (m.contains("node")) {
                mut.newNode = fromJson(m["node"]);
                if (mut.newNode) ownedNodes.push_back(mut.newNode);
            }
            mutations.push_back(mut);
        }
        auto batchRes = batch.applySequence(mutations);
        if (!batchRes.success) {
            for (auto* n : ownedNodes) {
                if (n->parent == nullptr) deleteTree(n);
            }
            return headlessRpcError(id, -32010, batchRes.error);
        }
        state.applyOrchestratorToActive();
        if (state.active()) {
            state.active()->incrementalOptimizer.setRoot(
                state.active()->sync.getAST());
            state.active()->incrementalOptimizer.recordExternalTransform(
                "agent-batch", {},
                state.agentActorLabel(sessionId));
            std::vector<std::string> batchIds;
            for (const auto& m : mutations)
                if (!m.nodeId.empty()) batchIds.push_back(m.nodeId);
            state.active()->versionTracker.recordMutation(batchIds);
            // Record post-mutation state for undo
            state.active()->undoStack.record(
                state.active()->editBuf, state.activeAST());
        }
        return headlessRpcResult(id,
            {{"success", true}, {"appliedCount", batchRes.appliedCount},
             {"version", state.active()
                 ? state.active()->versionTracker.version : 0}});
    }

    // --- getInScopeSymbols ---
    if (method == "getInScopeSymbols") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return headlessRpcError(id, -32602,
                                     "Missing nodeId parameter");
        bool detailed = params.value("detailed", false);
        bool crossFile = params.value("crossFile", false);
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto symbols = ctx.getInScopeSymbols(nodeId);
        json arr = json::array();
        for (const auto& s : symbols) {
            if (detailed) {
                ASTNode* node = findNodeById(state.activeAST(), s.nodeId);
                json entry = {{"name", s.name}, {"kind", s.kind},
                              {"nodeId", s.nodeId}};
                if (node) entry["node"] = toJson(node);
                arr.push_back(entry);
            } else {
                arr.push_back({{"name", s.name}, {"kind", s.kind},
                               {"nodeId", s.nodeId}});
            }
        }
        // Cross-file: add exported symbols from other open buffers
        if (crossFile) {
            std::string activePath = state.activeBuffer
                ? state.activeBuffer->path : "";
            for (const auto& [path, buf] : state.bufferStates) {
                if (path == activePath) continue;
                Module* otherAST = buf->sync.getAST();
                if (!otherAST) continue;
                auto exports = collectExportedSymbols(otherAST, path);
                for (const auto& ex : exports) {
                    json entry = {{"name", ex.name}, {"kind", ex.kind},
                                  {"nodeId", ex.nodeId},
                                  {"file", ex.filePath}};
                    if (detailed) {
                        ASTNode* node = findNodeById(otherAST, ex.nodeId);
                        if (node) entry["node"] = toJson(node);
                    }
                    arr.push_back(entry);
                }
            }
        }
        json result = {{"symbols", arr},
                        {"count", (int)arr.size()},
                        {"mode", detailed ? "detailed" : "symbols"}};
        if (crossFile) result["crossFile"] = true;
        return headlessRpcResult(id, result);
    }

    // --- getCallHierarchy ---
    if (method == "getCallHierarchy") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string functionId = params.value("functionId", "");
        if (functionId.empty())
            return headlessRpcError(id, -32602,
                                     "Missing functionId parameter");
        bool detailed = params.value("detailed", false);
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto info = ctx.getCallHierarchy(functionId);
        json result;
        if (detailed) {
            // Full mode: include node JSON for each caller/callee
            json callers = json::array();
            for (const auto& cid : info.callerIds) {
                ASTNode* n = findNodeById(state.activeAST(), cid);
                json entry = {{"nodeId", cid}};
                if (n) { entry["name"] = getNodeName(n); entry["node"] = toJson(n); }
                callers.push_back(entry);
            }
            json callees = json::array();
            for (const auto& cid : info.calleeIds) {
                ASTNode* n = findNodeById(state.activeAST(), cid);
                json entry = {{"nodeId", cid}};
                if (n) { entry["name"] = getNodeName(n); entry["node"] = toJson(n); }
                callees.push_back(entry);
            }
            result = {{"functionId", info.functionId},
                      {"functionName", info.functionName},
                      {"callers", callers}, {"callees", callees},
                      {"mode", "detailed"}};
        } else {
            // Lean mode: names and IDs only
            json callerNames = json::array();
            for (const auto& cid : info.callerIds) {
                ASTNode* n = findNodeById(state.activeAST(), cid);
                callerNames.push_back(n ? getNodeName(n) : cid);
            }
            json calleeNames = json::array();
            for (const auto& cid : info.calleeIds) {
                ASTNode* n = findNodeById(state.activeAST(), cid);
                calleeNames.push_back(n ? getNodeName(n) : cid);
            }
            result = {{"functionId", info.functionId},
                      {"functionName", info.functionName},
                      {"callerIds", info.callerIds},
                      {"calleeIds", info.calleeIds},
                      {"callerNames", callerNames},
                      {"calleeNames", calleeNames},
                      {"mode", "symbols"}};
        }
        return headlessRpcResult(id, result);
    }

    // --- getDependencyGraph ---
    if (method == "getDependencyGraph") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return headlessRpcError(id, -32602,
                                     "Missing nodeId parameter");
        bool detailed = params.value("detailed", false);
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto deps = ctx.getDependencyGraph(nodeId);
        json result;
        if (detailed) {
            // Full mode: resolve each dependency ID to node JSON
            json depArr = json::array();
            for (const auto& did : deps) {
                ASTNode* n = findNodeById(state.activeAST(), did);
                json entry = {{"nodeId", did}};
                if (n) { entry["name"] = getNodeName(n); entry["node"] = toJson(n); }
                depArr.push_back(entry);
            }
            result = {{"dependencies", depArr}, {"mode", "detailed"}};
        } else {
            // Lean mode: just nodeId list
            json idList = json::array();
            for (const auto& did : deps)
                idList.push_back(did);
            result = {{"dependencyIds", idList},
                      {"count", (int)idList.size()},
                      {"mode", "symbols"}};
        }
        return headlessRpcResult(id, result);
    }

    // --- getAnnotationSuggestions ---
    if (method == "getAnnotationSuggestions") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        Module* ast = state.activeAST();
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (!nodeId.empty()) {
            if (!findNodeById(ast, nodeId))
                return headlessRpcError(id, -32002,
                    "Node not found: " + nodeId);
        }
        AgentAnnotationAssistant assistant;
        auto result = assistant.suggest(ast, nodeId);
        json suggArr = json::array();
        for (const auto& s : result.suggestions)
            suggArr.push_back({
                {"nodeId", s.nodeId},
                {"annotationType", s.annotationType},
                {"strategy", s.strategy}, {"reason", s.reason},
                {"confidence", s.confidence}
            });
        json diagArr = json::array();
        for (const auto& d : result.diagnostics)
            diagArr.push_back({{"severity", d.severity},
                               {"message", d.message},
                               {"nodeId", d.nodeId}});
        return headlessRpcResult(id, {{"scopeId", result.scopeNodeId},
                                       {"suggestions", suggArr},
                                       {"diagnostics", diagArr}});
    }

    // --- applyAnnotationSuggestion ---
    if (method == "applyAnnotationSuggestion") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireMutable(state, id);
        if (!err.is_null()) return err;
        Module* ast = state.mutationAST();
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        MemoryStrategyInference::Suggestion suggestion;
        suggestion.nodeId = params.value("nodeId", "");
        suggestion.annotationType = params.value("annotationType", "");
        suggestion.strategy = params.value("strategy", "");
        suggestion.reason = params.value("reason", "");
        suggestion.confidence = params.value("confidence", 0.0);
        AgentAnnotationAssistant assistant;
        auto res = assistant.applySuggestion(ast, suggestion);
        if (!res.success)
            return headlessRpcError(id, -32010, res.error);
        if (params.contains("accepted")) {
            bool accepted = params.value("accepted", true);
            assistant.recordFeedback(suggestion, accepted);
        }
        state.applyOrchestratorToActive();
        return headlessRpcResult(id,
            {{"success", true}, {"warning", res.warning}});
    }

    // --- recordAnnotationFeedback ---
    if (method == "recordAnnotationFeedback") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        MemoryStrategyInference::Suggestion suggestion;
        suggestion.nodeId = params.value("nodeId", "");
        suggestion.annotationType = params.value("annotationType", "");
        suggestion.strategy = params.value("strategy", "");
        suggestion.reason = params.value("reason", "");
        suggestion.confidence = params.value("confidence", 0.0);
        bool accepted = params.value("accepted", false);
        AgentAnnotationAssistant assistant;
        assistant.recordFeedback(suggestion, accepted);
        return headlessRpcResult(id, true);
    }

    // --- startWorkflowRecording ---
    if (method == "startWorkflowRecording") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string name = params.value("name", "workflow");
        state.agent.workflowRecorder.startRecording(
            name, sessionId, WorkflowRecorder::RecordingConfig{},
            state.buildSessionMetadata(false));
        return headlessRpcResult(id,
            {{"recording", true}, {"name", name}});
    }

    // --- stopWorkflowRecording ---
    if (method == "stopWorkflowRecording")
        return headlessRpcResult(id,
            state.agent.workflowRecorder.stopRecording());

    // --- getWorkflowRecording ---
    if (method == "getWorkflowRecording")
        return headlessRpcResult(id,
            state.agent.workflowRecorder.exportWorkflow());

    // --- replayWorkflow ---
    if (method == "replayWorkflow") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        if (!params.contains("workflow"))
            return headlessRpcError(id, -32602,
                                     "Missing workflow payload");
        WorkflowRecorder temp;
        if (!temp.loadWorkflow(params["workflow"]))
            return headlessRpcError(id, -32602,
                                     "Invalid workflow payload");
        auto requests = temp.buildReplayRequests();
        json results = json::array();
        state.agent.workflowRecorder.setReplaying(true);
        for (auto& req : requests)
            results.push_back(
                handleHeadlessAgentRequest(state, req, sessionId));
        state.agent.workflowRecorder.setReplaying(false);
        return headlessRpcResult(id,
            {{"count", results.size()}, {"responses", results}});
    }

    // --- getSessionInfo ---
    if (method == "getSessionInfo") {
        return headlessRpcResult(id, {
            {"mode", "headless"},
            {"workspace", state.workspaceRoot},
            {"language", state.defaultLanguage},
            {"bufferCount", (int)state.bufferStates.size()},
            {"activeBuffer", state.active() ? state.active()->path : ""},
            {"role", AgentPermissionPolicy::roleLabel(role)}
        });
    }

    // --- getDiagnostics ---
    if (method == "getDiagnostics") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        auto diags = collectAllDiagnostics(state.activeAST());
        // Optional severity filter
        if (params.contains("severity")) {
            std::string sevStr = params.value("severity", "");
            DiagnosticSeverity maxSev = severityFromStr(sevStr);
            diags = filterBySeverity(diags, maxSev);
        }
        // Optional source filter
        if (params.contains("source")) {
            std::string src = params.value("source", "");
            diags = filterBySource(diags, src);
        }
        // Record snapshot for delta tracking
        auto allDiagsUnfiltered = collectAllDiagnostics(state.activeAST());
        state.active()->diagTracker.recordSnapshot(allDiagsUnfiltered);
        json diagJson = diagnosticsToJson(diags);
        sortDiagnosticsByPriority(diagJson);
        json result = {
            {"diagnostics", diagJson},
            {"count", (int)diags.size()},
            {"version", state.active()->diagTracker.version}
        };
        int budget = params.value("budget", 0);
        if (budget > 0) {
            auto br = applyBudget(result, budget);
            return headlessRpcResult(id, br.result);
        }
        return headlessRpcResult(id, result);
    }

    // --- getDiagnosticsDelta ---
    if (method == "getDiagnosticsDelta") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        int sinceVersion = params.value("sinceVersion", 0);
        auto currentDiags = collectAllDiagnostics(state.activeAST());
        state.active()->diagTracker.recordSnapshot(currentDiags);
        auto delta = state.active()->diagTracker.getDelta(
            currentDiags, sinceVersion);
        return headlessRpcResult(id, deltaToJson(delta));
    }

    // --- getQuickFixes ---
    if (method == "getQuickFixes") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string nodeId = params.value("nodeId", "");
        std::vector<QuickFix> fixes;
        if (nodeId.empty())
            fixes = getQuickFixesAll(state.activeAST());
        else
            fixes = getQuickFixesForNode(state.activeAST(), nodeId);
        json arr = json::array();
        for (const auto& f : fixes)
            arr.push_back(quickFixToJson(f));
        return headlessRpcResult(id, {
            {"fixes", arr}, {"count", (int)fixes.size()}
        });
    }

    // --- applyQuickFix ---
    if (method == "applyQuickFix") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto err = headlessRequireMutable(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string diagCode = params.value("diagCode", "");
        std::string nodeId = params.value("nodeId", "");
        if (diagCode.empty() || nodeId.empty())
            return headlessRpcError(id, -32602,
                "Missing diagCode or nodeId");
        auto fix = findQuickFix(state.activeAST(), diagCode, nodeId);
        if (fix.mutation.is_null())
            return headlessRpcError(id, -32002,
                "No fix found for " + diagCode + " on " + nodeId);
        // Apply the fix mutation via the existing mutation path
        json mutRequest = {
            {"jsonrpc", "2.0"}, {"id", id},
            {"method", "applyMutation"}, {"params", fix.mutation}
        };
        json mutResp = handleHeadlessAgentRequest(
            state, mutRequest, sessionId);
        if (mutResp.contains("error"))
            return mutResp;
        // Re-check diagnostics to see if it cleared
        auto remainingDiags = collectAllDiagnostics(state.activeAST());
        bool cleared = true;
        for (const auto& d : remainingDiags) {
            if (d.code == diagCode && d.nodeId == nodeId) {
                cleared = false;
                break;
            }
        }
        json result = mutResp["result"];
        result["fixApplied"] = fix.id;
        result["diagnosticCleared"] = cleared;
        result["remainingDiagnostics"] = (int)remainingDiags.size();
        return headlessRpcResult(id, result);
    }

    // --- getASTSubtree ---
    if (method == "getASTSubtree") {
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return headlessRpcError(id, -32602,
                                     "Missing nodeId parameter");
        json subtree = toJsonSubtree(state.activeAST(), nodeId);
        if (subtree.is_null())
            return headlessRpcError(id, -32002,
                "Node not found: " + nodeId);
        json result = {{"subtree", subtree}};
        result["version"] = state.active()->versionTracker.version;
        result["tokenEstimate"] = tokenEstimate(result);
        return headlessRpcResult(id, result);
    }

    // --- getASTDiff ---
    if (method == "getASTDiff") {
        auto err = headlessRequireAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        int sinceVersion = params.value("sinceVersion", 0);
        json diff = state.active()->versionTracker.buildDiff(
            state.activeAST(), sinceVersion);
        diff["tokenEstimate"] = tokenEstimate(diff);
        return headlessRpcResult(id, diff);
    }

    // --- fileRead ---
    if (method == "fileRead") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        auto [ok, resolved] =
            fileOpsResolvePath(state.workspaceRoot, path);
        if (!ok)
            return headlessRpcError(id, -32040, resolved);
        int startLine = params.value("startLine", 0);
        int endLine = params.value("endLine", 0);
        auto [success, content, lineCount] =
            fileOpsRead(resolved, startLine, endLine);
        if (!success)
            return headlessRpcError(id, -32041, content);
        return headlessRpcResult(id, {
            {"content", content}, {"lineCount", lineCount},
            {"path", resolved}
        });
    }

    // --- fileWrite ---
    if (method == "fileWrite") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        std::string content = params.value("content", "");
        auto [ok, resolved] =
            fileOpsResolvePath(state.workspaceRoot, path);
        if (!ok)
            return headlessRpcError(id, -32040, resolved);
        auto [success, msg, bytes] = fileOpsWrite(resolved, content);
        if (!success)
            return headlessRpcError(id, -32041, msg);
        return headlessRpcResult(id, {
            {"success", true}, {"path", resolved},
            {"bytesWritten", bytes}
        });
    }

    // --- fileCreate ---
    if (method == "fileCreate") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        auto [ok, resolved] =
            fileOpsResolvePath(state.workspaceRoot, path);
        if (!ok)
            return headlessRpcError(id, -32040, resolved);
        std::string language = params.value("language", "");
        std::string tmpl = params.value("template", "");
        auto [success, msg] = fileOpsCreate(resolved, language, tmpl);
        if (!success)
            return headlessRpcError(id, -32041, msg);
        return headlessRpcResult(id, {
            {"success", true}, {"path", resolved}
        });
    }

    // --- fileDiff ---
    if (method == "fileDiff") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        std::string bufContent;
        std::string diskPath;
        if (!path.empty()) {
            auto [ok, resolved] =
                fileOpsResolvePath(state.workspaceRoot, path);
            if (!ok)
                return headlessRpcError(id, -32040, resolved);
            diskPath = resolved;
            auto it = state.bufferStates.find(path);
            if (it != state.bufferStates.end())
                bufContent = it->second->editBuf;
            else {
                auto [rok, content, lc] = fileOpsRead(resolved);
                if (rok) bufContent = content;
            }
        } else if (state.active()) {
            diskPath = state.active()->path;
            bufContent = state.active()->editBuf;
            auto [ok2, resolved2] =
                fileOpsResolvePath(state.workspaceRoot, diskPath);
            if (ok2) diskPath = resolved2;
        } else {
            return headlessRpcError(id, -32000, "No active buffer");
        }
        auto [diffText, added, removed] =
            fileOpsDiff(bufContent, diskPath);
        return headlessRpcResult(id, {
            {"diff", diffText}, {"linesAdded", added},
            {"linesRemoved", removed}
        });
    }

    // --- workspaceList ---
    if (method == "workspaceList") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string glob = params.value("glob", "*");
        auto entries = fileOpsListWorkspace(state.workspaceRoot, glob);
        json files = json::array();
        for (const auto& e : entries)
            files.push_back({
                {"path", e.path}, {"size", e.size},
                {"isDir", e.isDir}
            });
        return headlessRpcResult(id, {{"files", files}});
    }

    // --- openFile ---
    if (method == "openFile") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        std::string content = params.value("content", "");
        std::string language = params.value("language", "");
        // Auto-detect language from extension if not specified
        if (language.empty())
            language = detectLanguage(path);
        if (language.empty())
            language = state.defaultLanguage;
        // Read from disk if no content provided and workspace is set
        if (content.empty() && !state.workspaceRoot.empty()) {
            auto [ok, resolved] =
                fileOpsResolvePath(state.workspaceRoot, path);
            if (ok) {
                auto [rok, fileContent, lc] = fileOpsRead(resolved);
                if (rok) content = fileContent;
                path = resolved;
            }
        }
        auto* buf = state.openBuffer(path, content, language);
        if (!buf)
            return headlessRpcError(id, -32041, "Failed to open buffer");
        // Update import graph from the new buffer's AST + source
        Module* bufAST = buf->sync.getAST();
        if (bufAST)
            state.project.importGraph.updateFromAST(path, bufAST);
        // Fallback: scan source text for imports not captured by parser
        if (!content.empty())
            state.project.importGraph.updateFromSource(path, content);
        return headlessRpcResult(id, {
            {"path", path}, {"language", language},
            {"bufferCount", (int)state.bufferStates.size()}
        });
    }

    // --- closeFile ---
    if (method == "closeFile") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32002, "Buffer not found: " + path);
        state.project.importGraph.clearFile(path);
        state.closeBuffer(path);
        return headlessRpcResult(id, {
            {"closed", path},
            {"bufferCount", (int)state.bufferStates.size()}
        });
    }

    // --- listBuffers ---
    if (method == "listBuffers") {
        json buffers = json::array();
        for (const auto& [path, buf] : state.bufferStates) {
            bool isActive = (buf.get() == state.activeBuffer);
            buffers.push_back({
                {"path", buf->path},
                {"language", buf->language},
                {"modified", buf->modified},
                {"active", isActive}
            });
        }
        return headlessRpcResult(id, {
            {"buffers", buffers},
            {"count", (int)buffers.size()},
            {"activeBuffer", state.activeBuffer
                ? state.activeBuffer->path : ""}
        });
    }

    // --- setActiveBuffer ---
    if (method == "setActiveBuffer") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");
        if (path.empty())
            return headlessRpcError(id, -32602, "Missing path parameter");
        if (!state.setActiveBuffer(path))
            return headlessRpcError(id, -32002, "Buffer not found: " + path);
        return headlessRpcResult(id, {
            {"activeBuffer", path},
            {"language", state.activeBuffer->language}
        });
    }

    // --- indexWorkspace ---
    if (method == "indexWorkspace") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string root = params.value("root", state.workspaceRoot);
        if (root.empty())
            return headlessRpcError(id, -32602,
                "No workspace root (set via --workspace or root param)");
        state.project.scanWorkspace(root);
        state.workspaceRoot = root;
        const auto& idx = state.project.index;
        return headlessRpcResult(id, {
            {"root", root},
            {"fileCount", idx.fileCount()},
            {"dirCount", idx.dirCount()},
            {"totalEntries", (int)idx.files().size()}
        });
    }

    // --- getImportGraph ---
    if (method == "getImportGraph") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string filePath = params.value("file", "");
        const auto& graph = state.project.importGraph;
        if (!filePath.empty()) {
            // Imports for a specific file
            auto imports = graph.importsOf(filePath);
            json importArr = json::array();
            for (const auto& m : imports) importArr.push_back(m);
            auto importers = graph.importedBy(
                fs::path(filePath).stem().string());
            json importerArr = json::array();
            for (const auto& f : importers) importerArr.push_back(f);
            return headlessRpcResult(id, {
                {"file", filePath},
                {"imports", importArr},
                {"importedBy", importerArr}
            });
        }
        // Full import graph
        json edges = json::object();
        for (const auto& [file, imports] : graph.edges()) {
            json importArr = json::array();
            for (const auto& m : imports) importArr.push_back(m);
            edges[file] = importArr;
        }
        return headlessRpcResult(id, {
            {"edges", edges},
            {"fileCount", (int)graph.edges().size()}
        });
    }

    // --- searchProject ---
    if (method == "searchProject") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string name = params.value("name", "");
        std::string nodeId = params.value("nodeId", "");

        // If nodeId given, resolve name from the active buffer
        if (!name.empty()) {
            // use name as-is
        } else if (!nodeId.empty()) {
            // Search all buffers for a node with this ID
            for (const auto& [path, buf] : state.bufferStates) {
                Module* ast = buf->sync.getAST();
                if (!ast) continue;
                ASTNode* node = findNodeById(ast, nodeId);
                if (node) {
                    name = getNodeName(node);
                    break;
                }
            }
            if (name.empty())
                return headlessRpcError(id, -32002,
                    "Node not found: " + nodeId);
        } else {
            return headlessRpcError(id, -32602,
                "Missing name or nodeId parameter");
        }

        json results = json::array();
        for (const auto& [path, buf] : state.bufferStates) {
            Module* ast = buf->sync.getAST();
            if (!ast) continue;
            auto refs = collectSymbolReferences(ast, name, path);
            for (const auto& ref : refs) {
                results.push_back({
                    {"file", ref.file}, {"line", ref.line},
                    {"col", ref.col}, {"nodeId", ref.nodeId},
                    {"kind", ref.kind}, {"context", ref.context}
                });
            }
        }

        std::set<std::string> searchFiles;
        for (const auto& r : results)
            searchFiles.insert(r.value("file", ""));

        return headlessRpcResult(id, {
            {"name", name},
            {"references", results},
            {"count", (int)results.size()},
            {"fileCount", (int)searchFiles.size()}
        });
    }

    // --- renameSymbol ---
    if (method == "renameSymbol") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string oldName = params.value("oldName", "");
        std::string newName = params.value("newName", "");
        bool preview = params.value("preview", false);

        if (oldName.empty() || newName.empty())
            return headlessRpcError(id, -32602,
                "Missing oldName or newName parameter");


        // Collect changes across all open buffers
        std::vector<RenameChange> allChanges;
        for (const auto& [path, buf] : state.bufferStates) {
            Module* ast = buf->sync.getAST();
            if (!ast) continue;
            auto changes = buildRenameChanges(
                ast, oldName, newName, path);
            allChanges.insert(allChanges.end(),
                              changes.begin(), changes.end());
        }

        if (allChanges.empty())
            return headlessRpcError(id, -32002,
                "No references found for '" + oldName + "'");

        // Build preview JSON
        json changesJson = json::array();
        std::set<std::string> affectedFiles;
        for (const auto& c : allChanges) {
            changesJson.push_back({
                {"file", c.file}, {"nodeId", c.nodeId},
                {"property", c.property},
                {"oldValue", c.oldValue},
                {"newValue", c.newValue},
                {"kind", c.kind}
            });
            affectedFiles.insert(c.file);
        }

        if (preview) {
            return headlessRpcResult(id, {
                {"preview", true},
                {"changes", changesJson},
                {"changeCount", (int)allChanges.size()},
                {"fileCount", (int)affectedFiles.size()}
            });
        }

        // Apply changes: use setProperty on each node
        int applied = 0;
        std::vector<std::string> errors;
        for (const auto& c : allChanges) {
            auto it = state.bufferStates.find(c.file);
            if (it == state.bufferStates.end()) continue;
            Module* ast = it->second->sync.getAST();
            if (!ast) continue;
            ASTNode* node = findNodeById(ast, c.nodeId);
            if (!node) {
                errors.push_back("Node " + c.nodeId +
                                 " not found in " + c.file);
                continue;
            }
            // Apply the property change directly
            if (node->conceptType == "Function" &&
                c.property == "name") {
                static_cast<Function*>(node)->name = c.newValue;
                ++applied;
            } else if (node->conceptType == "FunctionCall" &&
                       c.property == "functionName") {
                static_cast<FunctionCall*>(node)->functionName =
                    c.newValue;
                ++applied;
            } else if (node->conceptType == "Variable" &&
                       c.property == "name") {
                static_cast<Variable*>(node)->name = c.newValue;
                ++applied;
            } else if (node->conceptType == "VariableReference" &&
                       c.property == "variableName") {
                static_cast<VariableReference*>(node)->variableName =
                    c.newValue;
                ++applied;
            } else if (node->conceptType == "Parameter" &&
                       c.property == "name") {
                static_cast<Parameter*>(node)->name = c.newValue;
                ++applied;
            }
        }

        // Mark affected buffers as modified, regenerate editBuf, record undo
        for (const auto& f : affectedFiles) {
            auto it = state.bufferStates.find(f);
            if (it != state.bufferStates.end()) {
                it->second->modified = true;
                // Regenerate code from the modified AST
                Module* bufAST = it->second->sync.getAST();
                if (bufAST) {
                    Pipeline pipeline;
                    it->second->editBuf = pipeline.generate(
                        bufAST, it->second->language);
                    // Record post-rename state for undo
                    it->second->undoStack.record(
                        it->second->editBuf, bufAST);
                }
            }
        }

        json result = {
            {"applied", applied},
            {"changes", changesJson},
            {"changeCount", (int)allChanges.size()},
            {"fileCount", (int)affectedFiles.size()}
        };
        if (!errors.empty()) {
            json errArr = json::array();
            for (const auto& e : errors) errArr.push_back(e);
            result["errors"] = errArr;
        }
        return headlessRpcResult(id, result);
    }

    // --- getProjectDiagnostics ---
    if (method == "getProjectDiagnostics") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        // Optional severity filter
        std::string sevFilter = params.value("severity", "");
        // Optional file path glob filter (simple suffix match)
        std::string fileGlob = params.value("fileGlob", "");

        // Collect module names from open buffers for cross-file checks
        std::set<std::string> openModuleNames;
        for (const auto& [path, buf] : state.bufferStates) {
            std::string stem = fs::path(path).stem().string();
            if (!stem.empty()) openModuleNames.insert(stem);
        }

        json filesDiags = json::object();
        int totalCount = 0;

        for (const auto& [path, buf] : state.bufferStates) {
            // Apply file glob filter (simple suffix/extension match)
            if (!fileGlob.empty()) {
                // Match *.ext or exact name
                if (fileGlob[0] == '*') {
                    std::string suffix = fileGlob.substr(1);
                    if (path.size() < suffix.size() ||
                        path.substr(path.size() - suffix.size()) != suffix)
                        continue;
                } else if (path.find(fileGlob) == std::string::npos) {
                    continue;
                }
            }

            Module* bufAST = buf->sync.getAST();
            std::vector<StructuredDiagnostic> diags;

            // Per-file diagnostics (annotation + strategy)
            if (bufAST) {
                auto fileDiags = collectAllDiagnostics(bufAST);
                diags.insert(diags.end(), fileDiags.begin(),
                             fileDiags.end());
                // Cross-file: undefined imports from AST
                auto crossDiags = collectCrossFileDiagnostics(
                    bufAST, path, openModuleNames);
                diags.insert(diags.end(), crossDiags.begin(),
                             crossDiags.end());
            }

            // Cross-file: undefined imports from source text
            if (!buf->editBuf.empty()) {
                auto srcCross = collectCrossFileDiagnosticsFromSource(
                    buf->editBuf, path, openModuleNames);
                // Deduplicate: only add source-based if no AST-based
                // cross-file diags exist for the same line
                std::set<int> astCrossLines;
                for (const auto& d : diags) {
                    if (d.source == "cross-file")
                        astCrossLines.insert(d.line);
                }
                for (auto& d : srcCross) {
                    if (astCrossLines.find(d.line) == astCrossLines.end())
                        diags.push_back(std::move(d));
                }
            }

            // Apply severity filter
            if (!sevFilter.empty()) {
                DiagnosticSeverity maxSev = severityFromStr(sevFilter);
                diags = filterBySeverity(diags, maxSev);
            }

            if (!diags.empty()) {
                json diagArr = diagnosticsToJson(diags);
                sortDiagnosticsByPriority(diagArr);
                filesDiags[path] = diagArr;
                totalCount += (int)diags.size();
            }
        }

        return headlessRpcResult(id, {
            {"files", filesDiags},
            {"fileCount", (int)filesDiags.size()},
            {"totalDiagnostics", totalCount}
        });
    }

    // --- undo ---
    if (method == "undo") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32000, "No active buffer");
        auto& stack = state.activeBuffer->undoStack;
        if (!stack.canUndo())
            return headlessRpcError(id, -32050, "Nothing to undo");
        const auto& snap = stack.undo();
        // Restore AST from JSON
        if (!snap.astJson.is_null()) {
            ASTNode* node = fromJson(snap.astJson);
            if (node && node->conceptType == "Module") {
                state.activeBuffer->sync.setAST(
                    std::unique_ptr<Module>(
                        static_cast<Module*>(node)));
                state.activeBuffer->orchestratorDirty = true;
            } else {
                deleteTree(node);
            }
        }
        state.activeBuffer->editBuf = snap.text;
        state.activeBuffer->modified = true;
        return headlessRpcResult(id, {
            {"success", true},
            {"undoDepth", stack.undoDepth()},
            {"redoDepth", stack.redoDepth()}
        });
    }

    // --- redo ---
    if (method == "redo") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32000, "No active buffer");
        auto& stack = state.activeBuffer->undoStack;
        if (!stack.canRedo())
            return headlessRpcError(id, -32050, "Nothing to redo");
        const auto& snap = stack.redo();
        if (!snap.astJson.is_null()) {
            ASTNode* node = fromJson(snap.astJson);
            if (node && node->conceptType == "Module") {
                state.activeBuffer->sync.setAST(
                    std::unique_ptr<Module>(
                        static_cast<Module*>(node)));
                state.activeBuffer->orchestratorDirty = true;
            } else {
                deleteTree(node);
            }
        }
        state.activeBuffer->editBuf = snap.text;
        state.activeBuffer->modified = true;
        return headlessRpcResult(id, {
            {"success", true},
            {"undoDepth", stack.undoDepth()},
            {"redoDepth", stack.redoDepth()}
        });
    }

    // --- saveBuffer ---
    if (method == "saveBuffer") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        std::string path = params.value("path", "");

        // Default to active buffer if no path specified
        if (path.empty()) {
            if (!state.activeBuffer)
                return headlessRpcError(id, -32000, "No active buffer");
            path = state.activeBuffer->path;
        }

        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32002,
                "Buffer not found: " + path);

        auto& buf = it->second;

        // Resolve path against workspace root
        std::string writePath = path;
        if (!state.workspaceRoot.empty()) {
            auto [ok, resolved] =
                fileOpsResolvePath(state.workspaceRoot, path);
            if (!ok)
                return headlessRpcError(id, -32040, resolved);
            writePath = resolved;
        }

        // Create parent directories if needed
        fs::path parentDir = fs::path(writePath).parent_path();
        if (!parentDir.empty()) {
            std::error_code ec;
            fs::create_directories(parentDir, ec);
        }

        // Write editBuf to disk
        auto [success, msg, bytes] = fileOpsWrite(writePath, buf->editBuf);
        if (!success)
            return headlessRpcError(id, -32041, msg);

        buf->modified = false;
        return headlessRpcResult(id, {
            {"success", true}, {"path", writePath},
            {"bytesWritten", bytes}
        });
    }

    // --- saveAllBuffers ---
    if (method == "saveAllBuffers") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");

        json savedPaths = json::array();
        int savedCount = 0;
        int skippedCount = 0;

        for (auto& [path, buf] : state.bufferStates) {
            if (!buf->modified) {
                ++skippedCount;
                continue;
            }

            std::string writePath = path;
            if (!state.workspaceRoot.empty()) {
                auto [ok, resolved] =
                    fileOpsResolvePath(state.workspaceRoot, path);
                if (!ok) {
                    ++skippedCount;
                    continue;
                }
                writePath = resolved;
            }

            fs::path parentDir = fs::path(writePath).parent_path();
            if (!parentDir.empty()) {
                std::error_code ec;
                fs::create_directories(parentDir, ec);
            }

            auto [success, msg, bytes] =
                fileOpsWrite(writePath, buf->editBuf);
            if (success) {
                buf->modified = false;
                savedPaths.push_back(writePath);
                ++savedCount;
            } else {
                ++skippedCount;
            }
        }

        return headlessRpcResult(id, {
            {"savedCount", savedCount},
            {"skippedCount", skippedCount},
            {"saved", savedPaths}
        });
    }

    // --- batchQuery ---
    if (method == "batchQuery") {
        auto params = request.contains("params") ? request["params"]
                                                  : json::object();
        if (!params.contains("queries") || !params["queries"].is_array())
            return headlessRpcError(id, -32602, "Missing queries array");
        json results = json::array();
        for (const auto& q : params["queries"]) {
            std::string subMethod = q.value("method", "");
            json subParams = q.contains("params") ? q["params"]
                                                    : json::object();
            json subReq = {{"jsonrpc", "2.0"}, {"id", 1},
                           {"method", subMethod}, {"params", subParams}};
            json subResp = handleHeadlessAgentRequest(
                state, subReq, sessionId);
            // Extract result or error from the sub-response
            if (subResp.contains("result"))
                results.push_back({{"method", subMethod},
                                    {"result", subResp["result"]}});
            else if (subResp.contains("error"))
                results.push_back({{"method", subMethod},
                                    {"error", subResp["error"]}});
            else
                results.push_back({{"method", subMethod},
                                    {"error", "Unknown response"}});
        }
        return headlessRpcResult(id,
            {{"results", results}, {"count", (int)results.size()}});
    }

    // --- saveAnnotatedAST ---
    if (method == "saveAnnotatedAST") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string path = params.value("path", "");
        if (path.empty() && state.activeBuffer)
            path = state.activeBuffer->path;
        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32602, "No buffer: " + path);
        Module* ast = it->second->sync.getAST();
        auto res = saveSidecarAST(state.workspaceRoot, path, ast);
        if (!res.success)
            return headlessRpcError(id, -32010, res.error);
        return headlessRpcResult(id,
            {{"success", true},
             {"sidecarPath", res.sidecarPath},
             {"annotationCount", res.annotationCount}});
    }

    // --- loadAnnotatedAST ---
    if (method == "loadAnnotatedAST") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string path = params.value("path", "");
        if (path.empty() && state.activeBuffer)
            path = state.activeBuffer->path;
        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32602, "No buffer: " + path);
        Module* ast = it->second->sync.getAST();
        auto res = loadSidecarAST(state.workspaceRoot, path, ast);
        if (!res.success)
            return headlessRpcError(id, -32010, res.error);
        return headlessRpcResult(id,
            {{"success", true},
             {"mergedCount", res.mergedCount},
             {"staleCount", res.staleCount}});
    }

    // --- listAnnotatedFiles ---
    if (method == "listAnnotatedFiles") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto files = listSidecarFiles(state.workspaceRoot);
        return headlessRpcResult(id,
            {{"files", files}, {"count", (int)files.size()}});
    }

    // --- setSemanticAnnotation ---
    if (method == "setSemanticAnnotation") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string nodeId = params.value("nodeId", "");
        std::string type = params.value("type", "");
        json fields = params.contains("fields") ? params["fields"]
                                                  : json::object();
        if (nodeId.empty() || type.empty())
            return headlessRpcError(id, -32602, "nodeId and type required");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");
        ASTNode* target = findNodeById(ast, nodeId);
        if (!target)
            return headlessRpcError(id, -32602, "Node not found: " + nodeId);

        // Map type string to conceptType
        std::string conceptType;
        if (type == "intent") conceptType = "IntentAnnotation";
        else if (type == "complexity") conceptType = "ComplexityAnnotation";
        else if (type == "risk") conceptType = "RiskAnnotation";
        else if (type == "contract") conceptType = "ContractAnnotation";
        else if (type == "tags") conceptType = "SemanticTagAnnotation";
        // Type System (Steps 272-273)
        else if (type == "bitWidth") conceptType = "BitWidthAnnotation";
        else if (type == "endian") conceptType = "EndianAnnotation";
        else if (type == "layout") conceptType = "LayoutAnnotation";
        else if (type == "nullability") conceptType = "NullabilityAnnotation";
        else if (type == "variance") conceptType = "VarianceAnnotation";
        else if (type == "identity") conceptType = "IdentityAnnotation";
        else if (type == "mut") conceptType = "MutAnnotation";
        else if (type == "typeState") conceptType = "TypeStateAnnotation";
        // Concurrency (Step 274)
        else if (type == "atomic") conceptType = "AtomicAnnotation";
        else if (type == "sync") conceptType = "SyncAnnotation";
        else if (type == "threadModel") conceptType = "ThreadModelAnnotation";
        else if (type == "memoryBarrier") conceptType = "MemoryBarrierAnnotation";
        // Async / Error Handling (Step 275)
        else if (type == "exec") conceptType = "ExecAnnotation";
        else if (type == "blocking") conceptType = "BlockingAnnotation";
        else if (type == "parallel") conceptType = "ParallelAnnotation";
        else if (type == "trap") conceptType = "TrapAnnotation";
        else if (type == "exception") conceptType = "ExceptionAnnotation";
        else if (type == "panic") conceptType = "PanicAnnotation";
        // Scope & Namespace (Step 276)
        else if (type == "binding") conceptType = "BindingAnnotation";
        else if (type == "lookup") conceptType = "LookupAnnotation";
        else if (type == "capture") conceptType = "CaptureAnnotation";
        else if (type == "visibility") conceptType = "VisibilityAnnotation";
        else if (type == "namespace") conceptType = "NamespaceAnnotation";
        else if (type == "scope") conceptType = "ScopeAnnotation";
        // Shim & Escape Hatch (Step 278)
        else if (type == "intrinsic") conceptType = "IntrinsicAnnotation";
        else if (type == "raw") conceptType = "RawAnnotation";
        else if (type == "callingConv") conceptType = "CallingConvAnnotation";
        else if (type == "link") conceptType = "LinkAnnotation";
        else if (type == "shim") conceptType = "ShimAnnotation";
        else if (type == "pointerArithmetic") conceptType = "PointerArithmeticAnnotation";
        else if (type == "opaque") conceptType = "OpaqueAnnotation";
        // Platform & Provenance (Step 279)
        else if (type == "target") conceptType = "TargetAnnotation";
        else if (type == "feature") conceptType = "FeatureAnnotation";
        else if (type == "original") conceptType = "OriginalAnnotation";
        else if (type == "mapping") conceptType = "MappingAnnotation";
        // Optimization Completion (Step 280)
        else if (type == "tailCall") conceptType = "TailCallAnnotation";
        else if (type == "loop") conceptType = "LoopAnnotation";
        else if (type == "dataHint") conceptType = "DataAnnotation";
        else if (type == "align") conceptType = "AlignAnnotation";
        else if (type == "pack") conceptType = "PackAnnotation";
        else if (type == "boundsCheck") conceptType = "BoundsCheckAnnotation";
        else if (type == "overflow") conceptType = "OverflowAnnotation";
        // Meta-Programming (Step 281)
        else if (type == "meta") conceptType = "MetaAnnotation";
        else if (type == "symbolMode") conceptType = "SymbolAnnotation";
        else if (type == "evaluate") conceptType = "EvaluateAnnotation";
        else if (type == "template") conceptType = "TemplateAnnotation";
        else if (type == "synthetic") conceptType = "SyntheticAnnotation";
        // Strategy & Policy (Step 282)
        else if (type == "policy") conceptType = "PolicyAnnotation";
        else if (type == "ambiguity") conceptType = "AmbiguityAnnotation";
        else if (type == "candidate") conceptType = "CandidateAnnotation";
        else if (type == "tradeoff") conceptType = "TradeoffAnnotation";
        else if (type == "choice") conceptType = "ChoiceAnnotation";
        else if (type == "decision") conceptType = "DecisionAnnotation";
        // Subject 9: Workflow Routing (Step 315)
        else if (type == "contextWidth") conceptType = "ContextWidthAnnotation";
        else if (type == "review") conceptType = "ReviewAnnotation";
        else if (type == "automatability") conceptType = "AutomatabilityAnnotation";
        else if (type == "priority") conceptType = "PriorityAnnotation";
        else if (type == "implementationStatus") conceptType = "ImplementationStatusAnnotation";
        // Environment Layer (Step 285)
        else if (type == "capabilityRequirement") conceptType = "CapabilityRequirement";
        else return headlessRpcError(id, -32602, "Unknown annotation type: " + type);

        // Remove existing annotation of same type (update semantics)
        auto annos = target->getChildren("annotations");
        for (auto* a : annos) {
            if (a->conceptType == conceptType) {
                target->removeChild(a);
                break;
            }
        }

        // Create new annotation node
        json nodeJson = {{"concept", conceptType}, {"properties", fields}};
        ASTNode* newAnno = fromJson(nodeJson);
        if (!newAnno)
            return headlessRpcError(id, -32010, "Failed to create annotation");
        target->addChild("annotations", newAnno);

        return headlessRpcResult(id, {{"success", true}, {"type", type}});
    }

    // --- getSemanticAnnotations ---
    if (method == "getSemanticAnnotations") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        if (!nodeId.empty()) {
            // Return annotations for a specific node
            ASTNode* target = findNodeById(ast, nodeId);
            if (!target)
                return headlessRpcError(id, -32602, "Node not found: " + nodeId);
            json annos = json::array();
            for (auto* a : target->getChildren("annotations")) {
                if (!isSemanticAnnotation(a->conceptType)) continue;
                json entry;
                if (a->conceptType == "IntentAnnotation") {
                    auto* ia = static_cast<IntentAnnotation*>(a);
                    entry = {{"type", "intent"}, {"summary", ia->summary},
                             {"category", ia->category}};
                } else if (a->conceptType == "ComplexityAnnotation") {
                    auto* ca = static_cast<ComplexityAnnotation*>(a);
                    entry = {{"type", "complexity"},
                             {"timeComplexity", ca->timeComplexity},
                             {"cognitiveComplexity", ca->cognitiveComplexity},
                             {"linesOfLogic", ca->linesOfLogic}};
                } else if (a->conceptType == "RiskAnnotation") {
                    auto* ra = static_cast<RiskAnnotation*>(a);
                    entry = {{"type", "risk"}, {"level", ra->level},
                             {"reason", ra->reason},
                             {"dependentCount", ra->dependentCount}};
                } else if (a->conceptType == "ContractAnnotation") {
                    auto* ca = static_cast<ContractAnnotation*>(a);
                    entry = {{"type", "contract"},
                             {"preconditions", ca->preconditions},
                             {"postconditions", ca->postconditions},
                             {"returnShape", ca->returnShape},
                             {"sideEffects", ca->sideEffects}};
                } else if (a->conceptType == "SemanticTagAnnotation") {
                    auto* ta = static_cast<SemanticTagAnnotation*>(a);
                    entry = {{"type", "tags"}, {"tags", ta->tags}};
                } else {
                    // Generic fallback for all other semantic annotations
                    entry = {{"type", a->conceptType}};
                    json props = propertiesToJson(a);
                    for (auto& [k, v] : props.items())
                        entry[k] = v;
                }
                if (!entry.empty()) annos.push_back(entry);
            }
            return headlessRpcResult(id,
                {{"nodeId", nodeId}, {"annotations", annos}});
        } else {
            // Return all annotated nodes
            json nodes = json::array();
            for (auto* child : ast->allChildren()) {
                auto annos = child->getChildren("annotations");
                json annoList = json::array();
                for (auto* a : annos) {
                    if (!isSemanticAnnotation(a->conceptType)) continue;
                    json entry;
                    if (a->conceptType == "IntentAnnotation") {
                        auto* ia = static_cast<IntentAnnotation*>(a);
                        entry = {{"type", "intent"}, {"summary", ia->summary}};
                    } else if (a->conceptType == "ComplexityAnnotation") {
                        entry = {{"type", "complexity"}};
                    } else if (a->conceptType == "RiskAnnotation") {
                        auto* ra = static_cast<RiskAnnotation*>(a);
                        entry = {{"type", "risk"}, {"level", ra->level}};
                    } else if (a->conceptType == "ContractAnnotation") {
                        entry = {{"type", "contract"}};
                    } else if (a->conceptType == "SemanticTagAnnotation") {
                        auto* ta = static_cast<SemanticTagAnnotation*>(a);
                        entry = {{"type", "tags"}, {"tags", ta->tags}};
                    } else {
                        entry = {{"type", a->conceptType}};
                        json props = propertiesToJson(a);
                        for (auto& [k, v] : props.items())
                            entry[k] = v;
                    }
                    if (!entry.empty()) annoList.push_back(entry);
                }
                if (!annoList.empty()) {
                    nodes.push_back({
                        {"nodeId", child->id},
                        {"name", getNodeName(child)},
                        {"kind", child->conceptType},
                        {"annotations", annoList}
                    });
                }
            }
            return headlessRpcResult(id, {{"nodes", nodes}});
        }
    }

    // --- removeSemanticAnnotation ---
    if (method == "removeSemanticAnnotation") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string nodeId = params.value("nodeId", "");
        std::string type = params.value("type", "");
        if (nodeId.empty() || type.empty())
            return headlessRpcError(id, -32602, "nodeId and type required");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");
        ASTNode* target = findNodeById(ast, nodeId);
        if (!target)
            return headlessRpcError(id, -32602, "Node not found: " + nodeId);

        std::string conceptType;
        if (type == "intent") conceptType = "IntentAnnotation";
        else if (type == "complexity") conceptType = "ComplexityAnnotation";
        else if (type == "risk") conceptType = "RiskAnnotation";
        else if (type == "contract") conceptType = "ContractAnnotation";
        else if (type == "tags") conceptType = "SemanticTagAnnotation";
        else return headlessRpcError(id, -32602, "Unknown type: " + type);

        bool removed = false;
        for (auto* a : target->getChildren("annotations")) {
            if (a->conceptType == conceptType) {
                target->removeChild(a);
                removed = true;
                break;
            }
        }

        return headlessRpcResult(id, {{"removed", removed}, {"type", type}});
    }

    // --- getUnannotatedNodes ---
    if (method == "getUnannotatedNodes") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string filterType = params.value("type", "");
        bool includeHints = params.value("includeHints", false);
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        // Map filter type to conceptType
        std::string filterConcept;
        if (filterType == "intent") filterConcept = "IntentAnnotation";
        else if (filterType == "complexity") filterConcept = "ComplexityAnnotation";
        else if (filterType == "risk") filterConcept = "RiskAnnotation";
        else if (filterType == "contract") filterConcept = "ContractAnnotation";
        else if (filterType == "tags") filterConcept = "SemanticTagAnnotation";

        json nodes = json::array();
        for (auto* child : ast->allChildren()) {
            // Only report Function and Variable nodes
            if (child->conceptType != "Function" &&
                child->conceptType != "Variable")
                continue;

            auto annos = child->getChildren("annotations");
            bool hasTarget = false;
            if (filterConcept.empty()) {
                // Check for any semantic annotation
                for (auto* a : annos) {
                    if (isSemanticAnnotation(a->conceptType)) {
                        hasTarget = true;
                        break;
                    }
                }
            } else {
                for (auto* a : annos) {
                    if (a->conceptType == filterConcept) {
                        hasTarget = true;
                        break;
                    }
                }
            }

            if (!hasTarget) {
                json entry = {
                    {"nodeId", child->id},
                    {"name", getNodeName(child)},
                    {"kind", child->conceptType}
                };
                // Add static analysis hints if requested
                if (includeHints) {
                    json hints;
                    // Body statement count
                    auto body = child->getChildren("body");
                    hints["bodyStatements"] = (int)body.size();
                    // Side effect heuristic: check for io/network calls
                    bool hasSideEffects = false;
                    // Check body statements and their descendants
                    std::vector<const ASTNode*> toCheck;
                    for (auto* stmt : body) {
                        toCheck.push_back(stmt);
                        for (auto* desc : stmt->allChildren())
                            toCheck.push_back(desc);
                    }
                    for (auto* node : toCheck) {
                        if (node->conceptType == "FunctionCall" ||
                            node->conceptType == "MethodCall") {
                            std::string callee = getNodeName(node);
                            if (callee.find("print") != std::string::npos ||
                                callee.find("write") != std::string::npos ||
                                callee.find("send") != std::string::npos ||
                                callee.find("read") != std::string::npos ||
                                callee.find("open") != std::string::npos)
                                hasSideEffects = true;
                        }
                    }
                    hints["sideEffectHint"] = hasSideEffects
                        ? "possible" : "none";
                    entry["hints"] = hints;
                }
                nodes.push_back(entry);
            }
        }

        return headlessRpcResult(id,
            {{"nodes", nodes}, {"count", (int)nodes.size()}});
    }

    // --- setEnvironment ---
    if (method == "setEnvironment") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        // Remove existing EnvironmentSpec if any
        for (auto* child : ast->getChildren("environment")) {
            ast->removeChild(child);
            break;
        }

        auto* env = new EnvironmentSpec();
        envSpecFromJson(env, params);
        ast->addChild("environment", env);

        return headlessRpcResult(id, {{"success", true},
            {"envId", env->envId}});
    }

    // --- getEnvironment ---
    if (method == "getEnvironment") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        auto envChildren = ast->getChildren("environment");
        if (envChildren.empty())
            return headlessRpcResult(id, {{"hasEnvironment", false}});

        auto* env = static_cast<EnvironmentSpec*>(envChildren[0]);
        json envJson = envSpecToJson(env);
        envJson["hasEnvironment"] = true;
        return headlessRpcResult(id, envJson);
    }

    // --- validateEnvironment ---
    if (method == "validateEnvironment") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        auto envChildren = ast->getChildren("environment");
        if (envChildren.empty())
            return headlessRpcError(id, -32602, "No EnvironmentSpec set on module");

        auto* env = static_cast<EnvironmentSpec*>(envChildren[0]);
        auto capDiags = validateCapabilities(ast, env);
        auto annoDiags = validateEnvAnnotations(ast, env);

        json diagsJson = json::array();
        for (const auto& d : capDiags) {
            diagsJson.push_back({{"severity", d.severity},
                {"message", d.message}, {"nodeId", d.nodeId},
                {"capability", d.capability}});
        }
        for (const auto& d : annoDiags) {
            diagsJson.push_back({{"severity", d.severity},
                {"message", d.message}, {"nodeId", d.nodeId}});
        }

        return headlessRpcResult(id, {
            {"diagnostics", diagsJson},
            {"count", (int)diagsJson.size()}
        });
    }

    // --- getLoweringHints ---
    if (method == "getLoweringHints") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        auto envChildren = ast->getChildren("environment");
        if (envChildren.empty())
            return headlessRpcError(id, -32602, "No EnvironmentSpec set");

        auto* env = static_cast<EnvironmentSpec*>(envChildren[0]);
        ASTNode* target = nodeId.empty() ? ast : findNodeById(ast, nodeId);
        if (!target)
            return headlessRpcError(id, -32602, "Node not found: " + nodeId);

        auto hints = getLoweringHints(target, env);
        json hintsJson = json::array();
        for (const auto& h : hints) {
            hintsJson.push_back({{"pattern", h.pattern},
                {"description", h.description}});
        }
        return headlessRpcResult(id, {{"hints", hintsJson},
            {"count", (int)hintsJson.size()}});
    }

    // --- Step 317: Workflow Annotation Foundation ---

    // createSkeleton — create a new skeleton module
    if (method == "createSkeleton") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string name = params.value("name", "skeleton");
        std::string language = params.value("language", "python");

        // Create skeleton module using openBuffer with empty content
        std::string bufPath = "skel_" + name;
        auto* buf = state.openBuffer(bufPath, "", language);
        // Replace the default empty module with a properly named skeleton
        auto* rawMod = createSkeletonModule(name, language);
        buf->sync.setAST(std::unique_ptr<Module>(rawMod));
        buf->modified = true;
        return headlessRpcResult(id, {{"bufferId", bufPath},
            {"name", name}, {"language", language}});
    }

    // addSkeletonNode — add function/class skeleton with annotations
    if (method == "addSkeletonNode") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        std::string nodeType = params.value("nodeType", "function");
        std::string name = params.value("name", "unnamed");
        std::vector<std::string> paramNames;
        if (params.contains("parameters") && params["parameters"].is_array()) {
            for (const auto& p : params["parameters"])
                if (p.is_string()) paramNames.push_back(p.get<std::string>());
        }

        // Build annotations from params
        std::vector<ASTNode*> annos;
        if (params.contains("contextWidth")) {
            auto* cw = new ContextWidthAnnotation();
            static int cwC = 0;
            cw->id = "rpc_cw_" + std::to_string(++cwC);
            cw->width = params["contextWidth"].get<std::string>();
            annos.push_back(cw);
        }
        if (params.contains("automatability")) {
            auto* aa = new AutomatabilityAnnotation();
            static int aaC = 0;
            aa->id = "rpc_aa_" + std::to_string(++aaC);
            aa->strategy = params["automatability"].get<std::string>();
            annos.push_back(aa);
        }
        if (params.contains("priority")) {
            auto* pa = new PriorityAnnotation();
            static int paC = 0;
            pa->id = "rpc_pa_" + std::to_string(++paC);
            pa->level = params["priority"].get<std::string>();
            if (params.contains("blockedBy") && params["blockedBy"].is_array()) {
                for (const auto& b : params["blockedBy"])
                    if (b.is_string()) pa->blockedBy.push_back(b.get<std::string>());
            }
            annos.push_back(pa);
        }

        std::string nodeId;
        if (nodeType == "class") {
            std::vector<std::string> methods;
            if (params.contains("methods") && params["methods"].is_array()) {
                for (const auto& m : params["methods"])
                    if (m.is_string()) methods.push_back(m.get<std::string>());
            }
            auto* cls = addSkeletonClass(ast, name, methods, {});
            nodeId = cls->id;
        } else {
            auto* fn = addSkeletonFunction(ast, name, paramNames, "", annos);
            nodeId = fn->id;
        }

        state.activeBuffer->modified = true;
        return headlessRpcResult(id, {{"nodeId", nodeId}, {"name", name},
            {"nodeType", nodeType}});
    }

    // getProjectModel — skeleton summary + task list
    if (method == "getProjectModel") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        auto summary = getSkeletonSummary(ast);
        auto tasks = skeletonToTaskList(ast);

        json tasksJson = json::array();
        for (const auto& t : tasks) {
            json tj;
            tj["nodeId"] = t.nodeId;
            tj["nodeName"] = t.nodeName;
            tj["nodeType"] = t.nodeType;
            tj["contextWidth"] = t.contextWidth;
            tj["automatability"] = t.automatability;
            tj["priority"] = t.priority;
            tj["reviewRequired"] = t.reviewRequired;
            tj["status"] = t.status;
            if (!t.dependencies.empty())
                tj["dependencies"] = t.dependencies;
            tasksJson.push_back(tj);
        }

        return headlessRpcResult(id, {
            {"totalNodes", summary.totalNodes},
            {"skeletonNodes", summary.skeletonNodes},
            {"implementedNodes", summary.implementedNodes},
            {"tasks", tasksJson}
        });
    }

    // inferAnnotations — run AnnotationInference on active buffer
    if (method == "inferAnnotations") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.activeBuffer)
            return headlessRpcError(id, -32602, "No active buffer");
        Module* ast = state.activeAST();
        if (!ast)
            return headlessRpcError(id, -32602, "No AST available");

        AnnotationInference infEngine;
        auto inferred = infEngine.inferAll(ast);

        json suggestionsJson = json::array();
        for (const auto& inf : inferred) {
            json sj;
            sj["nodeId"] = inf.nodeId;
            sj["annotationType"] = inf.annotationType;
            if (!inf.key.empty()) sj["key"] = inf.key;
            if (!inf.value.empty()) sj["value"] = inf.value;
            sj["reason"] = inf.reason;
            sj["confidence"] = inf.confidence;
            suggestionsJson.push_back(sj);
        }

        return headlessRpcResult(id, {
            {"suggestions", suggestionsJson},
            {"count", (int)suggestionsJson.size()}
        });
    }

    // --- createWorkflow ---
    if (method == "createWorkflow") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string projectName = params.value("projectName", "");
        if (projectName.empty())
            return headlessRpcError(id, -32602, "Missing projectName");
        if (!state.activeBuffer || !state.activeAST())
            return headlessRpcError(id, -32000, "No active buffer with AST");

        state.workflow = WorkflowState(projectName);
        std::string bufferId = state.activeBuffer->path;
        int count = state.workflow->populateFromSkeleton(state.activeAST(), bufferId);
        state.workflowProgress = WorkflowProgress(count);
        state.eventStream.emit({
            "workflow.created",
            "",
            {{"projectName", projectName}, {"itemCount", count}},
            workItemTimestamp()
        });

        auto stats = state.workflow->getStats();
        return headlessRpcResult(id, {
            {"itemCount", count},
            {"phase", workflowPhaseToString(state.workflow->getPhase())},
            {"stats", stats.toJson()}
        });
    }

    // --- getWorkflowState ---
    if (method == "getWorkflowState") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto stats = state.workflow->getStats();
        return headlessRpcResult(id, {
            {"phase", workflowPhaseToString(state.workflow->getPhase())},
            {"stats", stats.toJson()},
            {"readyCount", state.workflow->queue.readyCount()},
            {"blockedCount", state.workflow->queue.blockedCount()}
        });
    }

    // --- getReadyTasks ---
    if (method == "getReadyTasks") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto ready = state.workflow->queue.getReady();
        json items = json::array();
        for (const auto& wi : ready) {
            items.push_back(workItemToJson(wi));
        }
        return headlessRpcResult(id, {{"items", items}, {"count", (int)items.size()}});
    }

    // --- getWorkItem ---
    if (method == "getWorkItem") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        return headlessRpcResult(id, workItemToJson(*item));
    }

    // --- assignTask ---
    if (method == "assignTask") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        std::string assignee = params.value("assignee", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        WorkItem updated = *item;
        bool ok = transitionWorkItem(updated, WI_ASSIGNED);
        if (!ok)
            return headlessRpcError(id, -32000, "Cannot assign item in status: " + updated.status);
        updated.assignee = assignee;
        state.workflow->queue.updateItem(itemId, updated);
        state.workflow->recordChange(itemId, item->status, WI_ASSIGNED,
                                     "agent:" + sessionId);

        return headlessRpcResult(id, {{"success", true}, {"item", workItemToJson(updated)}});
    }

    // --- completeTask ---
    if (method == "completeTask") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        // Attach result
        WorkItem updated = *item;
        if (params.contains("result")) {
            auto r = params["result"];
            updated.result.generatedCode = r.value("generatedCode", "");
            updated.result.confidence = r.value("confidence", 0.0f);
            updated.result.reasoning = r.value("reasoning", "");
        }

        // Transition: must be in-progress (or review) to complete
        if (updated.status == WI_IN_PROGRESS || updated.status == WI_REVIEW) {
            transitionWorkItem(updated, WI_COMPLETE);
        } else {
            return headlessRpcError(id, -32000,
                "Cannot complete item in status: " + updated.status);
        }
        state.workflow->queue.updateItem(itemId, updated);
        state.workflow->recordChange(itemId, item->status, WI_COMPLETE,
                                     "agent:" + sessionId);

        // Re-evaluate dependencies via complete()
        // (already handled internally since we set status to complete)
        // Check for newly ready items
        auto ready = state.workflow->queue.getReady();
        json newlyReady = json::array();
        for (const auto& wi : ready) {
            newlyReady.push_back(workItemToJson(wi));
        }

        return headlessRpcResult(id, {
            {"success", true},
            {"newlyReady", newlyReady}
        });
    }

    // --- rejectTask ---
    if (method == "rejectTask") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        std::string reason = params.value("reason", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        if (item->status != WI_REVIEW)
            return headlessRpcError(id, -32000,
                "Cannot reject item in status: " + item->status);

        bool ok = state.workflow->queue.reject(itemId, reason);
        if (!ok)
            return headlessRpcError(id, -32000, "Reject failed");

        state.workflow->recordChange(itemId, "review", WI_READY,
                                     "human:" + sessionId, reason);

        return headlessRpcResult(id, {{"success", true}});
    }

    // --- saveWorkflow ---
    if (method == "saveWorkflow") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        std::string root = state.workspaceRoot.empty() ? "." : state.workspaceRoot;
        auto sr = ::saveWorkflow(root, *state.workflow);
        return headlessRpcResult(id, {
            {"success", sr.success},
            {"path", sr.path},
            {"bytesWritten", sr.bytesWritten}
        });
    }

    // --- routeTask ---
    if (method == "routeTask") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        auto decision = state.routingEngine.route(*item);

        // Apply routing to the item
        WorkItem updated = *item;
        updated.workerType = decision.workerType;
        updated.reviewRequired = decision.reviewRequired;
        state.workflow->queue.updateItem(itemId, updated);

        return headlessRpcResult(id, {{"decision", decision.toJson()}});
    }

    // --- routeAllReady ---
    if (method == "routeAllReady") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto ready = state.workflow->queue.getReady();
        json decisions = json::array();
        int routed = 0;

        for (const auto& wi : ready) {
            auto decision = state.routingEngine.route(wi);
            WorkItem updated = wi;
            updated.workerType = decision.workerType;
            updated.reviewRequired = decision.reviewRequired;
            state.workflow->queue.updateItem(wi.id, updated);

            json entry = decision.toJson();
            entry["itemId"] = wi.id;
            decisions.push_back(entry);
            routed++;
        }

        return headlessRpcResult(id, {{"routed", routed}, {"decisions", decisions}});
    }

    // --- executeTask ---
    if (method == "executeTask") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        // Get worker for this item's type
        auto* worker = state.workerRegistry.getWorker(item->workerType);
        if (!worker)
            return headlessRpcError(id, -32000,
                "No worker for type: " + item->workerType);

        // Build context
        std::map<std::string, BufferInfo> bufferInfos;
        for (const auto& [path, buf] : state.bufferStates) {
            BufferInfo bi;
            bi.path = path;
            bi.content = buf->editBuf;
            bi.compactAst = json::array();
            bufferInfos[path] = bi;
        }

        WorkerContext ctx = state.contextAssembler.assembleContext(
            *item, bufferInfos, item->contextWidth);

        // Execute
        auto result = worker->execute(*item, ctx);

        // Update item with result
        WorkItem updated = *item;
        updated.result = result;

        bool autoApproved = false;
        // Deterministic/template with high confidence → auto-approve
        if ((item->workerType == "deterministic" || item->workerType == "template") &&
            result.confidence >= 0.8f && !item->reviewRequired) {
            autoApproved = true;
        }

        state.workflow->queue.updateItem(itemId, updated);

        return headlessRpcResult(id, {
            {"result", result.toJson()},
            {"workerType", item->workerType},
            {"autoApproved", autoApproved}
        });
    }

    // --- getRoutingExplanation ---
    if (method == "getRoutingExplanation") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        auto decision = state.routingEngine.route(*item);

        json annotationsUsed = json::array();
        if (!item->workerType.empty()) annotationsUsed.push_back("@Automatability");
        if (!item->contextWidth.empty()) annotationsUsed.push_back("@ContextWidth");
        if (item->reviewRequired) annotationsUsed.push_back("@Review");

        json rulesApplied = json::array();
        if (decision.reasoning.find("Explicit") != std::string::npos)
            rulesApplied.push_back("explicit-override");
        if (decision.reasoning.find("etter") != std::string::npos)
            rulesApplied.push_back("getter-setter-pattern");
        if (decision.reasoning.find("Cross") != std::string::npos ||
            decision.reasoning.find("cross") != std::string::npos)
            rulesApplied.push_back("context-width-escalation");
        if (decision.reasoning.find("default") != std::string::npos ||
            decision.reasoning.find("Default") != std::string::npos)
            rulesApplied.push_back("default-routing");

        return headlessRpcResult(id, {
            {"reasoning", decision.reasoning},
            {"annotationsUsed", annotationsUsed},
            {"rulesApplied", rulesApplied},
            {"decision", decision.toJson()}
        });
    }

    // --- getReviewQueue ---
    if (method == "getReviewQueue") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto reviewItems = state.workflow->queue.getByStatus(WI_REVIEW);
        json arr = json::array();
        for (const auto& wi : reviewItems) {
            arr.push_back({
                {"itemId", wi.id},
                {"nodeName", wi.nodeName},
                {"nodeType", wi.nodeType},
                {"bufferId", wi.bufferId},
                {"workerType", wi.workerType},
                {"priority", wi.priority},
                {"confidence", wi.result.confidence},
                {"reviewRequired", wi.reviewRequired},
                {"status", wi.status}
            });
        }

        return headlessRpcResult(id, {
            {"items", arr},
            {"count", (int)arr.size()}
        });
    }

    // --- getReviewContext ---
    if (method == "getReviewContext") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");

        std::string summary;
        summary += "Review Item: " + item->id + "\n";
        summary += "Node: " + item->nodeName + " (" + item->nodeType + ")\n";
        summary += "Worker: " + item->workerType + ", Priority: " + item->priority + "\n";
        summary += "Status: " + item->status + ", ReviewRequired: ";
        summary += item->reviewRequired ? "true\n" : "false\n";
        summary += "Confidence: " + std::to_string(item->result.confidence) + "\n";
        summary += "Reasoning: " + item->result.reasoning + "\n";
        summary += "Generated Code:\n" + item->result.generatedCode + "\n";
        if (!item->rejectionFeedback.empty()) {
            summary += "Previous Feedback: " + item->rejectionFeedback + "\n";
        }

        return headlessRpcResult(id, {
            {"item", workItemToJson(*item)},
            {"generatedCode", item->result.generatedCode},
            {"confidence", item->result.confidence},
            {"reasoning", item->result.reasoning},
            {"dependencies", item->dependencies},
            {"humanSummary", summary}
        });
    }

    // --- approveReviewItem ---
    if (method == "approveReviewItem") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        std::string feedback = params.value("feedback", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");
        if (item->status != WI_REVIEW)
            return headlessRpcError(id, -32000,
                "Cannot approve item in status: " + item->status);

        WorkItem updated = *item;
        bool ok = transitionWorkItem(updated, WI_COMPLETE);
        if (!ok)
            return headlessRpcError(id, -32000, "Approve transition failed");
        if (!feedback.empty()) updated.result.reasoning += "\nReviewer Note: " + feedback;

        state.workflow->queue.updateItem(itemId, updated);
        state.workflow->recordChange(itemId, WI_REVIEW, WI_COMPLETE,
                                     "human:" + sessionId, feedback);

        return headlessRpcResult(id, {
            {"success", true},
            {"item", workItemToJson(updated)}
        });
    }

    // --- rejectReviewItem ---
    if (method == "rejectReviewItem") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return headlessRpcError(id, -32000, "No active workflow");

        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        std::string feedback = params.value("feedback", "");
        if (itemId.empty())
            return headlessRpcError(id, -32602, "Missing itemId");
        if (feedback.empty())
            return headlessRpcError(id, -32602, "Missing feedback");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return headlessRpcError(id, -32602, "Work item not found");
        if (item->status != WI_REVIEW)
            return headlessRpcError(id, -32000,
                "Cannot reject item in status: " + item->status);

        bool ok = state.workflow->queue.reject(itemId, feedback);
        if (!ok)
            return headlessRpcError(id, -32000, "Reject failed");
        state.workflow->recordChange(itemId, WI_REVIEW, WI_READY,
                                     "human:" + sessionId, feedback);

        auto updated = state.workflow->queue.getItem(itemId);
        return headlessRpcResult(id, {
            {"success", true},
            {"item", updated ? workItemToJson(*updated) : json::object()}
        });
    }

    // --- setReviewPolicy ---
    if (method == "setReviewPolicy") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        if (params.contains("policy")) {
            state.reviewPolicy = ReviewPolicy::fromJson(params["policy"]);
        }
        return headlessRpcResult(id, {{"success", true}, {"policy", state.reviewPolicy.toJson()}});
    }

    // --- getReviewPolicy ---
    if (method == "getReviewPolicy") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        return headlessRpcResult(id, state.reviewPolicy.toJson());
    }

    if (auto routed = tryHandleHeadlessOrchestratorRPC(
            state, request, id, method, role); routed.has_value()) {
        return routed.value();
    }

    return headlessRpcError(id, -32601, "Method not found");
}
