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
