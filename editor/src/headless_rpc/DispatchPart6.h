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
