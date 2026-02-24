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
             {"annotationCount", res.annotationCount},
             {"semanticHashTablePath", res.semanticHashTablePath},
             {"semanticHashCount", res.semanticHashCount}});
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

    // --- saveSemanticHashTable ---
    if (method == "saveSemanticHashTable") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string path = params.value("path", "");
        if (path.empty() && state.activeBuffer) path = state.activeBuffer->path;
        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32602, "No buffer: " + path);
        Module* ast = it->second->sync.getAST();
        auto res = saveSemanticHashTable(state.workspaceRoot, path, ast);
        if (!res.success) return headlessRpcError(id, -32010, res.error);
        return headlessRpcResult(id, {
            {"success", true},
            {"semanticHashTablePath", res.path},
            {"semanticHashCount", res.entryCount}
        });
    }

    // --- getSemanticHashTable ---
    if (method == "getSemanticHashTable") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string path = params.value("path", "");
        if (path.empty() && state.activeBuffer) path = state.activeBuffer->path;
        bool refresh = params.value("refresh", false);
        auto it = state.bufferStates.find(path);
        if (it == state.bufferStates.end())
            return headlessRpcError(id, -32602, "No buffer: " + path);
        if (refresh) {
            Module* ast = it->second->sync.getAST();
            auto save = saveSemanticHashTable(state.workspaceRoot, path, ast);
            if (!save.success) return headlessRpcError(id, -32010, save.error);
        }
        auto res = loadSemanticHashTable(state.workspaceRoot, path);
        if (!res.success) return headlessRpcError(id, -32010, res.error);
        return headlessRpcResult(id, {
            {"success", true},
            {"semanticHashTablePath", res.path},
            {"table", res.table}
        });
    }

    // --- listSemanticHashTables ---
    if (method == "listSemanticHashTables") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto files = listSemanticHashTables(state.workspaceRoot);
        return headlessRpcResult(id, {
            {"files", files},
            {"count", (int)files.size()}
        });
    }

    // --- setSemanticHashLock ---
    if (method == "setSemanticHashLock") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return headlessRpcError(id, -32602, "nodeId required");
        bool locked = params.value("locked", true);
        std::string reason = params.value("reason", "");
        Module* ast = state.activeAST();
        if (!ast) return headlessRpcError(id, -32602, "No AST available");
        ASTNode* node = findNodeById(ast, nodeId);
        if (!node) return headlessRpcError(id, -32602, "Node not found: " + nodeId);
        node->semanticHashLockState = locked ? "locked" : "unlocked";
        node->semanticHashLockReason = reason;
        return headlessRpcResult(id, {
            {"success", true},
            {"nodeId", node->id},
            {"semanticHash", node->semanticHash},
            {"semanticHashLockState", node->semanticHashLockState},
            {"semanticHashLockReason", node->semanticHashLockReason}
        });
    }

    // --- getSemanticHashLock ---
    if (method == "getSemanticHashLock") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string nodeId = params.value("nodeId", "");
        if (nodeId.empty())
            return headlessRpcError(id, -32602, "nodeId required");
        Module* ast = state.activeAST();
        if (!ast) return headlessRpcError(id, -32602, "No AST available");
        ASTNode* node = findNodeById(ast, nodeId);
        if (!node) return headlessRpcError(id, -32602, "Node not found: " + nodeId);
        return headlessRpcResult(id, {
            {"nodeId", node->id},
            {"semanticHash", node->semanticHash},
            {"semanticHashLockState", node->semanticHashLockState.empty() ? "unlocked" : node->semanticHashLockState},
            {"semanticHashLockReason", node->semanticHashLockReason}
        });
    }

    // --- setSemanticAnnotation ---
    if (method == "setSemanticAnnotation") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return headlessRpcError(id, -32031, "Role not permitted");
        auto params = request.contains("params") ? request["params"]
                                                   : json::object();
        std::string nodeId = params.value("nodeId", "");
        std::string type = params.value("type", "");
        std::string hashLockMode = params.value("hashLockMode", "warn");  // warn|error
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
        std::string hashLockWarning = ASTMutationAPI::semanticHashLockWarning(target);
        if (!hashLockWarning.empty() && hashLockMode == "error") {
            return headlessRpcError(id, -32012, "Semantic hash lock violation: " + hashLockWarning);
        }

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

        return headlessRpcResult(id, {
            {"success", true},
            {"type", type},
            {"hashLockWarning", hashLockWarning}
        });
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
