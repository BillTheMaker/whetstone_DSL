#pragma once
// Step 245: Headless Agent RPC Handler
//
// Mirrors AgentRPCHandler.h but operates on HeadlessEditorState.
// Dispatch logic is split into smaller headers under headless_rpc/ to
// enforce architecture size constraints while preserving behavior.

struct HeadlessEditorState;
#include "HeadlessOrchestratorRPC.h"
#include "GenerationQualityGates.h"

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

inline json handleHeadlessAgentRequest(HeadlessEditorState& state,
                                       const json& request,
                                       const std::string& sessionId) {
    json id = request.contains("id") ? request["id"] : json(nullptr);
    std::string method = request.value("method", "");
    AgentRole role = state.getAgentRole(sessionId);

#include "headless_rpc/DispatchPart1.h"
#include "headless_rpc/DispatchPart2.h"
#include "headless_rpc/DispatchPart3.h"
#include "headless_rpc/DispatchPart4.h"
#include "headless_rpc/DispatchPart5.h"
#include "headless_rpc/DispatchPart6.h"

    if (auto routed = tryHandleHeadlessOrchestratorRPC(
            state, request, id, method, role); routed.has_value()) {
        return routed.value();
    }

    return headlessRpcError(id, -32601, "Method not found");
}
