// Step 382: Orchestrator RPC + MCP tools (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "MCPServer.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name,
                         const std::string& workerType = "",
                         const std::string& contextWidth = "local") {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = name;
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.workerType = workerType;
    item.contextWidth = contextWidth;
    item.priority = "medium";
    item.status = WI_PENDING;
    item.createdAt = workItemTimestamp();
    return item;
}

static HeadlessEditorState makeState(AgentRole role = AgentRole::Generator) {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.openBuffer("main.py", "def a():\n    return 1\n", "python");
    state.setAgentRole("test-session", role);
    state.workflow = WorkflowState("wf");
    return state;
}

static json rpc(HeadlessEditorState& state, const std::string& method,
                const json& params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method},
                    {"params", params}};
    return handleHeadlessAgentRequest(state, request, "test-session");
}

static json mcpCall(MCPServer& server, const std::string& method,
                    const json& params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method},
                    {"params", params}};
    return server.handleRequest(request);
}

int main() {
    int passed = 0;

    // Test 1: orchestrateStep advances one template task
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("t1", "getValue", "template"));
        auto resp = rpc(state, "orchestrateStep");
        assert(resp.contains("result"));
        assert(resp["result"]["event"]["type"] == "completed");
        std::cout << "Test 1 PASSED: orchestrateStep advances one task\n";
        passed++;
    }

    // Test 2: orchestrateAdvance processes a batch of ready items
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("a", "getA", "template"));
        state.workflow->queue.enqueue(makeItem("b", "getB", "template"));
        auto resp = rpc(state, "orchestrateAdvance");
        assert(resp.contains("result"));
        assert(resp["result"]["itemsAdvanced"].get<int>() == 2);
        std::cout << "Test 2 PASSED: orchestrateAdvance handles batches\n";
        passed++;
    }

    // Test 3: orchestrateRunDeterministic stops at blockers
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("d1", "getFast", "template"));
        state.workflow->queue.enqueue(makeItem("h1", "manual", "human"));
        auto resp = rpc(state, "orchestrateRunDeterministic");
        assert(resp.contains("result"));
        assert(resp["result"]["stats"]["complete"].get<int>() == 1);
        assert(!resp["result"]["blockers"].empty());
        std::cout << "Test 3 PASSED: runDeterministic stops at blockers\n";
        passed++;
    }

    // Test 4: getBlockers reports human items
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("h1", "manual", "human"));
        rpc(state, "orchestrateStep"); // move to blocked human state
        auto blockers = rpc(state, "getBlockers");
        assert(blockers.contains("result"));
        assert(blockers["result"]["count"].get<int>() >= 1);
        std::cout << "Test 4 PASSED: getBlockers reports workflow blockers\n";
        passed++;
    }

    // Test 5: getProgress returns valid snapshot fields
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("t1", "getV", "template"));
        rpc(state, "orchestrateStep");
        auto resp = rpc(state, "getProgress");
        assert(resp.contains("result"));
        assert(resp["result"].contains("completionPercent"));
        assert(resp["result"].contains("itemsPerMinute"));
        assert(resp["result"].contains("byWorkerType"));
        std::cout << "Test 5 PASSED: getProgress returns snapshot\n";
        passed++;
    }

    // Test 6: submitExternalResult accepts LLM output and advances task
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        WorkItem llm = makeItem("x1", "buildGraph", "", "project");
        state.workflow->queue.enqueue(llm);
        rpc(state, "orchestrateStep"); // routes to llm, blocks in-progress
        auto submit = rpc(state, "submitExternalResult", {
            {"itemId", "x1"},
            {"result", {
                {"generatedCode", "def buildGraph(data):\n    return {}"},
                {"confidence", 0.92},
                {"reasoning", "external model result"},
                {"tokensGenerated", 40}
            }}
        });
        assert(submit.contains("result"));
        assert(submit["result"]["success"].get<bool>());
        auto item = state.workflow->queue.getItem("x1");
        assert(item.has_value());
        assert(item->status == WI_COMPLETE);
        std::cout << "Test 6 PASSED: submitExternalResult advances agent task\n";
        passed++;
    }

    // Test 7: linter can read progress but cannot orchestrate
    {
        auto state = makeState(AgentRole::Linter);
        state.workflow->queue.enqueue(makeItem("l1", "getName", "template"));
        auto progress = rpc(state, "getProgress");
        auto step = rpc(state, "orchestrateStep");
        assert(progress.contains("result"));
        assert(step.contains("error"));
        std::cout << "Test 7 PASSED: linter read-only orchestration access\n";
        passed++;
    }

    // Test 8: MCP tool registration includes orchestrator tools
    {
        MCPServer server;
        const auto& tools = server.getTools();
        std::vector<std::string> needed = {
            "whetstone_orchestrate_step",
            "whetstone_orchestrate_advance",
            "whetstone_orchestrate_run_deterministic",
            "whetstone_get_blockers",
            "whetstone_get_progress",
            "whetstone_submit_result"
        };
        int found = 0;
        for (const auto& t : tools) {
            for (const auto& n : needed) {
                if (t.name == n) found++;
            }
        }
        assert(found == 6);
        std::cout << "Test 8 PASSED: MCP orchestrator tools registered\n";
        passed++;
    }

    // Test 9: MCP orchestrate_run_deterministic executes through callback
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("m1", "getOne", "template"));
        MCPServer server;
        server.setRpcCallback([&state](const json& request) {
            return handleHeadlessAgentRequest(state, request, "test-session");
        });
        auto resp = mcpCall(server, "tools/call", {
            {"name", "whetstone_orchestrate_run_deterministic"},
            {"arguments", json::object()}
        });
        assert(resp.contains("result"));
        assert(resp["result"]["isError"].get<bool>() == false);
        std::cout << "Test 9 PASSED: MCP run_deterministic tool call works\n";
        passed++;
    }

    // Test 10: combined flow updates progress after external submit
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        state.workflow->queue.enqueue(makeItem("d1", "getFast", "template"));
        state.workflow->queue.enqueue(makeItem("x1", "buildGraph", "", "project"));

        rpc(state, "orchestrateRunDeterministic");
        auto blockers = rpc(state, "getBlockers");
        assert(blockers["result"]["count"].get<int>() >= 1);
        rpc(state, "submitExternalResult", {
            {"itemId", "x1"},
            {"result", {
                {"generatedCode", "def buildGraph(data):\n    return data"},
                {"confidence", 0.88},
                {"reasoning", "llm output"},
                {"tokensGenerated", 24}
            }}
        });
        auto progress = rpc(state, "getProgress");
        assert(progress["result"]["completedItems"].get<int>() == 2);
        std::cout << "Test 10 PASSED: combined orchestrator flow tracked\n";
        passed++;
    }

    // Test 11: submitExternalResult rejects non-agent worker types
    {
        auto state = makeState();
        WorkItem t = makeItem("t1", "getName", "template");
        transitionWorkItem(t, WI_READY);
        transitionWorkItem(t, WI_ASSIGNED);
        transitionWorkItem(t, WI_IN_PROGRESS);
        state.workflow->queue.enqueue(t);
        auto resp = rpc(state, "submitExternalResult", {
            {"itemId", "t1"},
            {"result", {{"generatedCode", "return name"}}}
        });
        assert(resp.contains("error"));
        std::cout << "Test 11 PASSED: submitExternalResult enforces worker type\n";
        passed++;
    }

    // Test 12: orchestrator RPC methods require active workflow
    {
        HeadlessEditorState state;
        state.setAgentRole("test-session", AgentRole::Generator);
        auto resp = rpc(state, "orchestrateAdvance");
        assert(resp.contains("error"));
        std::cout << "Test 12 PASSED: workflow guards enforced\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
