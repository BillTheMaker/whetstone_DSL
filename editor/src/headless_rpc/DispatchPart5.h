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
