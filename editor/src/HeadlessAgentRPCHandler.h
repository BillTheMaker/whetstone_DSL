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
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto symbols = ctx.getInScopeSymbols(nodeId);
        json arr = json::array();
        for (const auto& s : symbols)
            arr.push_back({{"name", s.name}, {"kind", s.kind},
                           {"nodeId", s.nodeId}});
        return headlessRpcResult(id, {{"symbols", arr}});
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
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto info = ctx.getCallHierarchy(functionId);
        return headlessRpcResult(id, {
            {"functionId", info.functionId},
            {"functionName", info.functionName},
            {"callerIds", info.callerIds},
            {"calleeIds", info.calleeIds}
        });
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
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto deps = ctx.getDependencyGraph(nodeId);
        return headlessRpcResult(id, {{"dependencies", deps}});
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

    return headlessRpcError(id, -32601, "Method not found");
}
