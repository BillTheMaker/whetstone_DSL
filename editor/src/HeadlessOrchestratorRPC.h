#pragma once
// Step 382: Orchestrator RPC surface for headless agent requests.

#include "HeadlessEditorState.h"
#include <optional>

static inline json orchestratorRpcError(const json& id, int code,
                                        const std::string& msg) {
    return {{"jsonrpc", "2.0"}, {"id", id},
            {"error", {{"code", code}, {"message", msg}}}};
}

static inline json orchestratorRpcResult(const json& id, const json& result) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

static inline json orchestratorEventToJson(const OrchestratorEvent& event) {
    return {
        {"type", event.type},
        {"itemId", event.itemId},
        {"detail", event.detail},
        {"timestamp", event.timestamp}
    };
}

static inline json orchestratorEventsToJson(
    const std::vector<OrchestratorEvent>& events) {
    json arr = json::array();
    for (const auto& event : events) arr.push_back(orchestratorEventToJson(event));
    return arr;
}

static inline json orchestratorBlockersToJson(const std::vector<BlockerInfo>& blockers) {
    json arr = json::array();
    for (const auto& b : blockers) {
        arr.push_back({
            {"type", b.type},
            {"itemIds", b.itemIds},
            {"description", b.description}
        });
    }
    return arr;
}

static inline std::map<std::string, BufferInfo> collectOrchestratorBufferInfos(
    HeadlessEditorState& state) {
    std::map<std::string, BufferInfo> infos;
    for (const auto& [path, buf] : state.bufferStates) {
        BufferInfo bi;
        bi.path = path;
        bi.content = buf->editBuf;
        Module* ast = buf->sync.getAST();
        bi.compactAst = ast ? toJsonCompactSummary(ast) : json::array();
        bi.importGraph = json::object();
        infos[path] = std::move(bi);
    }
    return infos;
}

inline std::optional<json> tryHandleHeadlessOrchestratorRPC(
    HeadlessEditorState& state,
    const json& request,
    const json& id,
    const std::string& method,
    AgentRole role) {
    if (method == "orchestrateStep") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return orchestratorRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return orchestratorRpcError(id, -32000, "No active workflow");

        WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                          state.workerRegistry, state.contextAssembler,
                                          state.reviewGate);
        orchestrator.setReviewPolicy(state.reviewPolicy);
        orchestrator.setBuffers(collectOrchestratorBufferInfos(state));

        OrchestratorEvent event = orchestrator.step();
        if (!state.workflowProgress) {
            state.workflowProgress = WorkflowProgress(state.workflow->getStats().total);
        }
        state.workflowProgress->recordEvent(event);
        return orchestratorRpcResult(id, {{"event", orchestratorEventToJson(event)}});
    }

    if (method == "orchestrateAdvance") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return orchestratorRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return orchestratorRpcError(id, -32000, "No active workflow");

        WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                          state.workerRegistry, state.contextAssembler,
                                          state.reviewGate);
        orchestrator.setReviewPolicy(state.reviewPolicy);
        orchestrator.setBuffers(collectOrchestratorBufferInfos(state));

        BatchResult batch = orchestrator.advanceBatch();
        if (!state.workflowProgress) {
            state.workflowProgress = WorkflowProgress(state.workflow->getStats().total);
        }
        for (const auto& event : batch.events) {
            state.workflowProgress->recordEvent(event);
        }

        return orchestratorRpcResult(id, {
            {"events", orchestratorEventsToJson(batch.events)},
            {"itemsAdvanced", batch.itemsAdvanced},
            {"itemsBlocked", batch.itemsBlocked},
            {"contextTokensSaved", batch.contextTokensSaved}
        });
    }

    if (method == "orchestrateRunDeterministic") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return orchestratorRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return orchestratorRpcError(id, -32000, "No active workflow");

        WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                          state.workerRegistry, state.contextAssembler,
                                          state.reviewGate);
        orchestrator.setReviewPolicy(state.reviewPolicy);
        orchestrator.setBuffers(collectOrchestratorBufferInfos(state));

        std::vector<OrchestratorEvent> allEvents;
        int totalAdvanced = 0;
        int totalBlocked = 0;
        int totalSaved = 0;
        while (true) {
            BatchResult batch = orchestrator.advanceBatch();
            if (batch.events.empty()) break;
            bool anyProgress = false;
            for (const auto& event : batch.events) {
                if (event.type != "blocked") anyProgress = true;
                allEvents.push_back(event);
            }
            totalAdvanced += batch.itemsAdvanced;
            totalBlocked += batch.itemsBlocked;
            totalSaved += batch.contextTokensSaved;
            if (!anyProgress) break;
        }

        if (!state.workflowProgress) {
            state.workflowProgress = WorkflowProgress(state.workflow->getStats().total);
        }
        for (const auto& event : allEvents) {
            state.workflowProgress->recordEvent(event);
        }

        return orchestratorRpcResult(id, {
            {"events", orchestratorEventsToJson(allEvents)},
            {"itemsAdvanced", totalAdvanced},
            {"itemsBlocked", totalBlocked},
            {"contextTokensSaved", totalSaved},
            {"stats", state.workflow->getStats().toJson()},
            {"blockers", orchestratorBlockersToJson(orchestrator.getBlockers())}
        });
    }

    if (method == "getBlockers") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return orchestratorRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return orchestratorRpcError(id, -32000, "No active workflow");

        WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                          state.workerRegistry, state.contextAssembler,
                                          state.reviewGate);
        orchestrator.setReviewPolicy(state.reviewPolicy);
        orchestrator.setBuffers(collectOrchestratorBufferInfos(state));
        auto blockers = orchestrator.getBlockers();
        return orchestratorRpcResult(id, {
            {"blockers", orchestratorBlockersToJson(blockers)},
            {"count", (int)blockers.size()}
        });
    }

    if (method == "getProgress") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return orchestratorRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return orchestratorRpcError(id, -32000, "No active workflow");

        if (!state.workflowProgress) {
            state.workflowProgress = WorkflowProgress(state.workflow->getStats().total);
        } else {
            state.workflowProgress->setTotalItems(state.workflow->getStats().total);
        }
        ProgressSnapshot snapshot = state.workflowProgress->getSnapshot();
        if (snapshot.blockers.empty()) {
            WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                              state.workerRegistry, state.contextAssembler,
                                              state.reviewGate);
            orchestrator.setReviewPolicy(state.reviewPolicy);
            orchestrator.setBuffers(collectOrchestratorBufferInfos(state));
            snapshot.blockers = orchestrator.getBlockers();
        }
        return orchestratorRpcResult(id, snapshot.toJson());
    }

    if (method == "submitExternalResult") {
        if (!AgentPermissionPolicy::canInvoke(role, method))
            return orchestratorRpcError(id, -32031, "Role not permitted");
        if (!state.workflow)
            return orchestratorRpcError(id, -32000, "No active workflow");
        auto params = request.contains("params") ? request["params"] : json::object();
        std::string itemId = params.value("itemId", "");
        if (itemId.empty())
            return orchestratorRpcError(id, -32602, "Missing itemId");
        if (!params.contains("result") || !params["result"].is_object())
            return orchestratorRpcError(id, -32602, "Missing result payload");

        auto item = state.workflow->queue.getItem(itemId);
        if (!item)
            return orchestratorRpcError(id, -32602, "Work item not found");
        if (!(item->workerType == "slm" || item->workerType == "llm"))
            return orchestratorRpcError(id, -32000, "submitExternalResult only supports slm/llm items");
        if (item->status != WI_IN_PROGRESS && item->status != WI_ASSIGNED)
            return orchestratorRpcError(id, -32000,
                "Item must be assigned or in-progress for external submission");

        WorkItem updated = *item;
        if (updated.status == WI_ASSIGNED) transitionWorkItem(updated, WI_IN_PROGRESS);

        const auto& result = params["result"];
        updated.result.generatedCode = result.value("generatedCode", "");
        updated.result.confidence = result.value("confidence", 0.0f);
        updated.result.reasoning = result.value("reasoning", "");
        updated.result.tokensGenerated = result.value("tokensGenerated", 0);
        updated.result.tokensBudget = result.value("tokensBudget",
                                                   estimateContextBudget(updated.contextWidth));
        if (result.contains("astJson")) updated.result.astJson = result["astJson"];
        if (result.contains("diagnostics") && result["diagnostics"].is_array()) {
            updated.result.diagnostics = result["diagnostics"].get<std::vector<json>>();
        }

        state.workflow->queue.updateItem(itemId, updated);

        std::vector<OrchestratorEvent> events;
        events.push_back({
            "executed",
            itemId,
            {{"workerType", updated.workerType},
             {"confidence", updated.result.confidence},
             {"tokensGenerated", updated.result.tokensGenerated}},
            workItemTimestamp()
        });

        ReviewDecision reviewDecision =
            state.reviewGate.shouldAutoApprove(updated, updated.result, state.reviewPolicy);
        if (reviewDecision.approved) {
            state.workflow->queue.complete(itemId);
            events.push_back({"auto-approved", itemId,
                              {{"rule", reviewDecision.ruleMatched}},
                              workItemTimestamp()});
            events.push_back({"completed", itemId, json::object(), workItemTimestamp()});
        } else {
            auto maybeUpdated = state.workflow->queue.getItem(itemId);
            if (!maybeUpdated) {
                return orchestratorRpcError(id, -32000, "Failed to reload item");
            }
            WorkItem reviewItem = *maybeUpdated;
            transitionWorkItem(reviewItem, WI_REVIEW);
            state.workflow->queue.updateItem(itemId, reviewItem);
            events.push_back({"sent-to-review", itemId,
                              {{"reason", reviewDecision.reasoning}},
                              workItemTimestamp()});
        }

        if (!state.workflowProgress) {
            state.workflowProgress = WorkflowProgress(state.workflow->getStats().total);
        }
        for (const auto& event : events) {
            state.workflowProgress->recordEvent(event);
        }

        auto latest = state.workflow->queue.getItem(itemId);
        return orchestratorRpcResult(id, {
            {"success", true},
            {"events", orchestratorEventsToJson(events)},
            {"item", latest ? workItemToJson(*latest) : json::object()}
        });
    }

    return std::nullopt;
}
