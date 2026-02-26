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
        auto q = GenerationQualityGates::evaluate(pr.generatedCode, tgtLang, "");
        json qj = GenerationQualityGates::toJson(q);
        result["quality"] = qj["quality"];
        result["gates"] = qj["gates"];
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
        Pipeline pipeline;
        std::string generatedCode = pipeline.generate(genRes.node, state.active()->language);
        auto q = GenerationQualityGates::evaluate(
            generatedCode, state.active()->language, genRes.note);
        json qj = GenerationQualityGates::toJson(q);
        json nodeJson = toJson(genRes.node);
        deleteTree(genRes.node);
        json out = {
            {"node", nodeJson}, {"note", genRes.note},
            {"usedSymbols", genRes.usedSymbols},
            {"language", state.active()->language},
            {"generatedCode", generatedCode}
        };
        out["quality"] = qj["quality"];
        out["gates"] = qj["gates"];
        return headlessRpcResult(id, out);
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
        std::string hashLockMode = params.value("hashLockMode", "warn");  // warn|error
        Module* ast = state.mutationAST();
        if (!ast) return headlessRpcError(id, -32001, "AST unavailable");
        ASTMutationAPI mut;
        mut.setRoot(ast);
        ASTMutationAPI::MutationResult res;
        LibraryPolicyResult policy;
        std::vector<std::string> affectedIds;
        std::string hashLockWarning;
        auto handleHashLock = [&](ASTNode* n, const std::string& context) -> json {
            if (!n) return json();
            std::string w = ASTMutationAPI::semanticHashLockWarning(n);
            if (w.empty()) return json();
            hashLockWarning = w;
            if (hashLockMode == "error") {
                return headlessRpcError(id, -32012, "Semantic hash lock violation (" + context + "): " + w);
            }
            return json();
        };
        if (type == "setProperty") {
            std::string nodeId = params.value("nodeId", "");
            if (!nodeId.empty()) affectedIds.push_back(nodeId);
            json lockErr = handleHashLock(findNodeById(ast, nodeId), "setProperty");
            if (!lockErr.is_null()) return lockErr;
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
            json lockErr = handleHashLock(findNodeById(ast, nodeId), "updateNode");
            if (!lockErr.is_null()) return lockErr;
            res = mut.updateNode(params.value("nodeId", ""), props);
        } else if (type == "deleteNode") {
            std::string nodeId = params.value("nodeId", "");
            if (!nodeId.empty()) affectedIds.push_back(nodeId);
            json lockErr = handleHashLock(findNodeById(ast, nodeId), "deleteNode");
            if (!lockErr.is_null()) return lockErr;
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
            json lockErr = handleHashLock(findNodeById(ast, params.value("parentId", "")), "insertNode");
            if (!lockErr.is_null()) {
                if (node) deleteTree(node);
                return lockErr;
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
            {"hashLockWarning", hashLockWarning},
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
        std::string hashLockMode = params.value("hashLockMode", "warn");  // warn|error
        if (!params.contains("mutations") ||
            !params["mutations"].is_array())
            return headlessRpcError(id, -32602,
                                     "Missing mutations array");
        BatchMutationAPI batch;
        batch.setRoot(ast);
        std::vector<BatchMutationAPI::Mutation> mutations;
        std::vector<ASTNode*> ownedNodes;
        std::vector<std::string> hashLockWarnings;
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
            ASTNode* guard = nullptr;
            if (mut.type == "insertNode") guard = findNodeById(ast, mut.parentId);
            else guard = findNodeById(ast, mut.nodeId);
            std::string w = ASTMutationAPI::semanticHashLockWarning(guard);
            if (!w.empty()) hashLockWarnings.push_back(w);
            mutations.push_back(mut);
        }
        if (!hashLockWarnings.empty() && hashLockMode == "error") {
            for (auto* n : ownedNodes) {
                if (n->parent == nullptr) deleteTree(n);
            }
            return headlessRpcError(id, -32012, "Semantic hash lock violation in batch: " + hashLockWarnings.front());
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
             {"hashLockWarnings", hashLockWarnings},
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
