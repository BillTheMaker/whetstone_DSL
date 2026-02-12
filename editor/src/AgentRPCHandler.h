#pragma once
// --- AgentRPCHandler.h ---
// Extracted from EditorState.h (Sprint 8, Step 235).
// All JSON-RPC method handlers for agent requests.
// Included from EditorState.h after the EditorState struct definition.

struct EditorState;

// --- RPC response helpers ---
static inline json agentRpcError(const json& id, int code, const std::string& msg) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"error", {{"code", code}, {"message", msg}}}};
}

static inline json agentRpcResult(const json& id, const json& result) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

// --- Precondition helpers ---
static inline json requireStructuredAST(EditorState& state, const json& id) {
    if (!state.active() || !state.isStructured())
        return agentRpcError(id, -32000, "No structured buffer");
    if (!state.activeAST())
        return agentRpcError(id, -32001, "AST unavailable");
    return json();
}

static inline json requireMutableAST(EditorState& state, const json& id) {
    if (!state.active() || !state.isStructured())
        return agentRpcError(id, -32000, "No structured buffer");
    if (!state.mutationAST())
        return agentRpcError(id, -32001, "AST unavailable");
    return json();
}

// --- Main RPC dispatch ---
inline json handleAgentRequest(EditorState& state, const json& request,
                               const std::string& sessionId) {
    json id = request.contains("id") ? request["id"] : json(nullptr);
    std::string method = request.value("method", "");
    AgentRole role = state.getAgentRole(sessionId);

    // --- getAST ---
    if (method == "getAST") {
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        return agentRpcResult(id, {
            {"ast", toJson(state.activeAST())},
            {"annotationCount", countAnnotationNodes(state.activeAST())},
            {"diagnostics", state.buildDiagnosticsJson()}
        });
    }

    // --- generateCode ---
    if (method == "generateCode") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string spec = params.value("spec", "");
        bool preferImports = params.value("preferImports", true);
        state.library.primitives.setRoot(state.activeAST());
        state.library.primitives.setLanguage(state.active()->language);
        AgentCodeGen gen;
        state.library.primitives.setContextTags(
            state.library.semanticTags.inferTagsFromText(spec));
        AgentCodeGenResult genRes = gen.generate(
            spec, state.library.primitives, state.active()->language, preferImports);
        if (!genRes.node)
            return agentRpcError(id, -32020, "Code generation failed");
        json nodeJson = toJson(genRes.node);
        deleteTree(genRes.node);
        return agentRpcResult(id, {
            {"node", nodeJson}, {"note", genRes.note},
            {"usedSymbols", genRes.usedSymbols}, {"language", state.active()->language}
        });
    }

    // --- setAgentRole ---
    if (method == "setAgentRole") {
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string roleName = params.value("role", "linter");
        AgentRole newRole = AgentPermissionPolicy::roleFromString(roleName);
        state.setAgentRole(sessionId, newRole);
        return agentRpcResult(id, {{"role", AgentPermissionPolicy::roleLabel(newRole)}});
    }

    // --- startWorkflowRecording ---
    if (method == "startWorkflowRecording") {
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string name = params.value("name", "workflow");
        state.agent.workflowRecorder.startRecording(
            name, sessionId, WorkflowRecorder::RecordingConfig{},
            state.buildSessionMetadata(false));
        return agentRpcResult(id, {{"recording", true}, {"name", name}});
    }

    // --- stopWorkflowRecording ---
    if (method == "stopWorkflowRecording") {
        return agentRpcResult(id, state.agent.workflowRecorder.stopRecording());
    }

    // --- getWorkflowRecording ---
    if (method == "getWorkflowRecording") {
        return agentRpcResult(id, state.agent.workflowRecorder.exportWorkflow());
    }

    // --- replayWorkflow ---
    if (method == "replayWorkflow") {
        auto params = request.contains("params") ? request["params"] : json::object();
        if (!params.contains("workflow"))
            return agentRpcError(id, -32602, "Missing workflow payload");
        WorkflowRecorder temp;
        if (!temp.loadWorkflow(params["workflow"]))
            return agentRpcError(id, -32602, "Invalid workflow payload");
        auto requests = temp.buildReplayRequests();
        json results = json::array();
        state.agent.workflowRecorder.setReplaying(true);
        for (auto& req : requests)
            results.push_back(handleAgentRequest(state, req, sessionId));
        state.agent.workflowRecorder.setReplaying(false);
        return agentRpcResult(id, {{"count", results.size()}, {"responses", results}});
    }

    // --- getAnnotationSuggestions ---
    if (method == "getAnnotationSuggestions") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        Module* ast = state.activeAST();
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string nodeId = params.value("nodeId", "");
        int line = params.value("line", -1);
        int col = params.value("col", -1);
        if (!nodeId.empty()) {
            if (!findNodeById(ast, nodeId))
                return agentRpcError(id, -32002, "Node not found: " + nodeId);
        } else if (line >= 0 && col >= 0) {
            ASTNode* posNode = findNodeAtPosition(ast, line, col);
            if (posNode) nodeId = posNode->id;
        }
        AgentAnnotationAssistant assistant;
        auto result = assistant.suggest(ast, nodeId);
        json suggArr = json::array();
        for (const auto& s : result.suggestions) {
            suggArr.push_back({
                {"nodeId", s.nodeId}, {"annotationType", s.annotationType},
                {"strategy", s.strategy}, {"reason", s.reason}, {"confidence", s.confidence}
            });
        }
        json diagArr = json::array();
        for (const auto& d : result.diagnostics) {
            diagArr.push_back({
                {"severity", d.severity}, {"message", d.message}, {"nodeId", d.nodeId}
            });
        }
        return agentRpcResult(id, {
            {"scopeId", result.scopeNodeId}, {"suggestions", suggArr},
            {"diagnostics", diagArr}
        });
    }

    // --- applyAnnotationSuggestion ---
    if (method == "applyAnnotationSuggestion") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireMutableAST(state, id);
        if (!err.is_null()) return err;
        Module* ast = state.mutationAST();
        auto params = request.contains("params") ? request["params"] : json::object();
        MemoryStrategyInference::Suggestion suggestion;
        suggestion.nodeId = params.value("nodeId", "");
        suggestion.annotationType = params.value("annotationType", "");
        suggestion.strategy = params.value("strategy", "");
        suggestion.reason = params.value("reason", "");
        suggestion.confidence = params.value("confidence", 0.0);
        AgentAnnotationAssistant assistant;
        auto res = assistant.applySuggestion(ast, suggestion);
        if (!res.success)
            return agentRpcError(id, -32010, res.error);
        if (params.contains("accepted")) {
            bool accepted = params.value("accepted", true);
            assistant.recordFeedback(suggestion, accepted);
        }
        state.applyOrchestratorToActive();
        if (state.active()) {
            std::vector<std::string> affected;
            if (!suggestion.nodeId.empty()) affected.push_back(suggestion.nodeId);
            if (!suggestion.annotationType.empty() && !suggestion.nodeId.empty())
                affected.push_back("anno_" + suggestion.annotationType + "_" + suggestion.nodeId);
            state.active()->incrementalOptimizer.setRoot(state.active()->sync.getAST());
            state.active()->incrementalOptimizer.recordExternalTransform(
                "agent-annotation", affected, state.agentActorLabel(sessionId));
        }
        return agentRpcResult(id, {{"success", true}, {"warning", res.warning}});
    }

    // --- recordAnnotationFeedback ---
    if (method == "recordAnnotationFeedback") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        MemoryStrategyInference::Suggestion suggestion;
        suggestion.nodeId = params.value("nodeId", "");
        suggestion.annotationType = params.value("annotationType", "");
        suggestion.strategy = params.value("strategy", "");
        suggestion.reason = params.value("reason", "");
        suggestion.confidence = params.value("confidence", 0.0);
        bool accepted = params.value("accepted", false);
        AgentAnnotationAssistant assistant;
        assistant.recordFeedback(suggestion, accepted);
        return agentRpcResult(id, true);
    }

    // --- applyMutation ---
    if (method == "applyMutation") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        if (!state.active() || !state.isStructured())
            return agentRpcError(id, -32000, "No structured buffer");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string type = params.value("type", "");
        bool preferImports = params.value("preferImports", false);
        bool strictMode = params.value("strictMode", false);
        Module* ast = state.mutationAST();
        if (!ast) return agentRpcError(id, -32001, "AST unavailable");
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
                policy = checkMutationLibraryPolicy(node, ast, preferImports, strictMode);
                if (!policy.ok) {
                    deleteTree(node);
                    return agentRpcError(id, -32011, policy.error);
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
            return agentRpcError(id, -32602, "Unknown mutation type");
        }
        if (!res.success) return agentRpcError(id, -32010, res.error);
        state.applyOrchestratorToActive();
        if (state.active()) {
            if (affectedIds.empty() && !params.value("nodeId", "").empty())
                affectedIds.push_back(params.value("nodeId", ""));
            state.active()->incrementalOptimizer.setRoot(state.active()->sync.getAST());
            state.active()->incrementalOptimizer.recordExternalTransform(
                "agent-mutation:" + type, affectedIds, state.agentActorLabel(sessionId));
        }
        return agentRpcResult(id, {
            {"success", true}, {"warning", res.warning},
            {"libraryWarning", policy.warning}, {"unknownFunctions", policy.unknownFunctions}
        });
    }

    // --- getInScopeSymbols ---
    if (method == "getInScopeSymbols") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return agentRpcError(id, -32602, "Missing nodeId parameter");
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto symbols = ctx.getInScopeSymbols(nodeId);
        json arr = json::array();
        for (const auto& s : symbols)
            arr.push_back({{"name", s.name}, {"kind", s.kind}, {"nodeId", s.nodeId}});
        return agentRpcResult(id, {{"symbols", arr}});
    }

    // --- getCallHierarchy ---
    if (method == "getCallHierarchy") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string functionId = params.value("functionId", "");
        if (functionId.empty())
            return agentRpcError(id, -32602, "Missing functionId parameter");
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto info = ctx.getCallHierarchy(functionId);
        return agentRpcResult(id, {
            {"functionId", info.functionId}, {"functionName", info.functionName},
            {"callerIds", info.callerIds}, {"calleeIds", info.calleeIds}
        });
    }

    // --- getDependencyGraph ---
    if (method == "getDependencyGraph") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return agentRpcError(id, -32602, "Missing nodeId parameter");
        ContextAPI ctx;
        ctx.setRoot(state.activeAST());
        auto deps = ctx.getDependencyGraph(nodeId);
        return agentRpcResult(id, {{"dependencies", deps}});
    }

    // --- applyBatch ---
    if (method == "applyBatch") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto errChk = requireMutableAST(state, id);
        if (!errChk.is_null()) return errChk;
        Module* ast = state.mutationAST();
        auto params = request.contains("params") ? request["params"] : json::object();
        if (!params.contains("mutations") || !params["mutations"].is_array())
            return agentRpcError(id, -32602, "Missing mutations array");
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
            return agentRpcError(id, -32010, batchRes.error);
        }
        state.applyOrchestratorToActive();
        if (state.active()) {
            state.active()->incrementalOptimizer.setRoot(state.active()->sync.getAST());
            state.active()->incrementalOptimizer.recordExternalTransform(
                "agent-batch", {}, state.agentActorLabel(sessionId));
        }
        return agentRpcResult(id, {{"success", true}, {"appliedCount", batchRes.appliedCount}});
    }

    // --- runPipeline ---
    if (method == "runPipeline") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string source = params.value("source", "");
        std::string srcLang = params.value("sourceLanguage", "");
        std::string tgtLang = params.value("targetLanguage", "");
        if (source.empty() || srcLang.empty() || tgtLang.empty())
            return agentRpcError(id, -32602,
                                 "Missing source, sourceLanguage, or targetLanguage");
        Pipeline pipeline;
        auto pr = pipeline.run(source, srcLang, tgtLang);
        json diagArr = json::array();
        for (const auto& d : pr.parseDiags)
            diagArr.push_back({{"line", d.line}, {"column", d.column},
                               {"message", d.message}, {"severity", d.severity}});
        json valDiagArr = json::array();
        for (const auto& d : pr.validationDiags)
            valDiagArr.push_back({{"severity", d.severity}, {"message", d.message},
                                  {"nodeId", d.nodeId}});
        json violArr = json::array();
        for (const auto& v : pr.violations)
            violArr.push_back({{"severity", v.severity}, {"category", v.category}, {"message", v.message},
                               {"nodeId", v.nodeId}});
        json suggArr = json::array();
        for (const auto& s : pr.suggestions)
            suggArr.push_back({{"nodeId", s.nodeId}, {"annotationType", s.annotationType},
                               {"strategy", s.strategy}, {"reason", s.reason},
                               {"confidence", s.confidence}});
        json result = {
            {"success", pr.success}, {"generatedCode", pr.generatedCode},
            {"parseDiagnostics", diagArr}, {"validationDiagnostics", valDiagArr},
            {"violations", violArr}, {"suggestions", suggArr},
            {"foldCount", pr.foldResult.nodesModified},
            {"dceCount", pr.dceResult.nodesModified}
        };
        if (pr.ast) result["ast"] = toJson(pr.ast.get());
        return agentRpcResult(id, result);
    }

    // --- parseSource ---
    if (method == "parseSource") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string source = params.value("source", "");
        std::string language = params.value("language", "");
        if (source.empty() || language.empty())
            return agentRpcError(id, -32602, "Missing source or language");
        Pipeline pipeline;
        std::vector<ParseDiagnostic> diags;
        auto mod = pipeline.parse(source, language, diags);
        json diagArr = json::array();
        for (const auto& d : diags)
            diagArr.push_back({{"line", d.line}, {"column", d.column},
                               {"message", d.message}, {"severity", d.severity}});
        json result = {{"diagnostics", diagArr}};
        if (mod) result["ast"] = toJson(mod.get());
        return agentRpcResult(id, result);
    }

    // --- generateFromAST ---
    if (method == "generateFromAST") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string language = params.value("language", state.active()->language);
        Pipeline pipeline;
        std::string code = pipeline.generate(state.activeAST(), language);
        return agentRpcResult(id, {{"code", code}, {"language", language}});
    }

    // --- projectLanguage ---
    if (method == "projectLanguage") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return agentRpcError(id, -32031, "Role not permitted");
        auto err = requireStructuredAST(state, id);
        if (!err.is_null()) return err;
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string targetLanguage = params.value("targetLanguage", "");
        if (targetLanguage.empty())
            return agentRpcError(id, -32602, "Missing targetLanguage");
        CrossLanguageProjector projector;
        auto projected = projector.project(state.activeAST(), targetLanguage);
        if (!projected)
            return agentRpcError(id, -32020, "Projection failed");
        Pipeline pipeline;
        std::string code = pipeline.generate(projected.get(), targetLanguage);
        return agentRpcResult(id, {
            {"ast", toJson(projected.get())}, {"generatedCode", code},
            {"sourceLanguage", state.active()->language},
            {"targetLanguage", targetLanguage}
        });
    }

    return agentRpcError(id, -32601, "Method not found");
}
