// Step 383: Phase 15a integration tests (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "MCPServer.h"
#include "WorkflowOrchestrator.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name,
                         const std::string& workerType = "",
                         const std::string& contextWidth = "local",
                         bool reviewRequired = false) {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = name;
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.workerType = workerType;
    item.contextWidth = contextWidth;
    item.reviewRequired = reviewRequired;
    item.priority = "medium";
    item.status = WI_PENDING;
    item.createdAt = workItemTimestamp();
    return item;
}

static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.openBuffer("main.py", "def a():\n    return 1\n", "python");
    state.setAgentRole("test-session", AgentRole::Generator);
    state.workflow = WorkflowState("phase15a");
    return state;
}

static json rpc(HeadlessEditorState& state, const std::string& method,
                const json& params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method},
                    {"params", params}};
    return handleHeadlessAgentRequest(state, request, "test-session");
}

int main() {
    int passed = 0;

    // Test 1: mixed 5-item workflow reaches partial deterministic completion
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("d1", "getName", "template"));
        state.workflow->queue.enqueue(makeItem("d2", "setValue", "template"));
        state.workflow->queue.enqueue(makeItem("a1", "planGraph", "", "project"));
        state.workflow->queue.enqueue(makeItem("a2", "buildIndex", "", "project"));
        state.workflow->queue.enqueue(makeItem("h1", "manualAudit", "human"));

        auto resp = rpc(state, "orchestrateRunDeterministic");
        assert(resp.contains("result"));
        auto stats = state.workflow->getStats();
        assert(stats.complete == 2);
        assert(stats.inProgress >= 2); // prepared agent/human tasks
        std::cout << "Test 1 PASSED: mixed workflow partial completion profile\n";
        passed++;
    }

    // Test 2: dependency chain respected
    {
        auto state = makeState();
        WorkItem a = makeItem("a", "getA", "template");
        WorkItem b = makeItem("b", "getB", "template");
        b.dependencies.push_back("a");
        state.workflow->queue.enqueue(a);
        state.workflow->queue.enqueue(b);

        auto first = rpc(state, "orchestrateAdvance");
        assert(first["result"]["itemsAdvanced"].get<int>() == 1);
        auto second = rpc(state, "orchestrateAdvance");
        assert(second["result"]["itemsAdvanced"].get<int>() == 1);
        std::cout << "Test 2 PASSED: dependency gating respected\n";
        passed++;
    }

    // Test 3: rejection escalation deterministic -> slm -> llm
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("r1", "getRisky", "template", "local", true));
        WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                          state.workerRegistry, state.contextAssembler,
                                          state.reviewGate);
        orchestrator.setBuffers({});
        orchestrator.setReviewPolicy(state.reviewPolicy);
        orchestrator.step(); // sent to review
        assert(orchestrator.rejectAndRequeue("r1", "attempt1"));
        orchestrator.step(); // slm blocked
        assert(orchestrator.rejectAndRequeue("r1", "attempt2"));
        orchestrator.step(); // llm blocked
        auto item = state.workflow->queue.getItem("r1");
        assert(item.has_value());
        assert(item->workerType == "llm");
        std::cout << "Test 3 PASSED: rejection escalation reaches llm\n";
        passed++;
    }

    // Test 4: shared project-context optimization yields token savings
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("p1", "task1", "", "project"));
        state.workflow->queue.enqueue(makeItem("p2", "task2", "", "project"));
        state.workflow->queue.enqueue(makeItem("p3", "task3", "", "project"));
        auto resp = rpc(state, "orchestrateAdvance");
        assert(resp["result"]["contextTokensSaved"].get<int>() > 0);
        std::cout << "Test 4 PASSED: shared-context optimization active\n";
        passed++;
    }

    // Test 5: progress snapshot reflects orchestrator events
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("d1", "getFast", "template"));
        state.workflow->queue.enqueue(makeItem("d2", "setFast", "template"));
        rpc(state, "orchestrateRunDeterministic");
        auto progress = rpc(state, "getProgress");
        assert(progress.contains("result"));
        assert(progress["result"]["completedItems"].get<int>() == 2);
        assert(progress["result"]["completionPercent"].get<float>() > 0.0f);
        std::cout << "Test 5 PASSED: progress tracking updates with orchestration\n";
        passed++;
    }

    // Test 6: submitExternalResult can complete an agent-prepared task
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        state.workflow->queue.enqueue(makeItem("x1", "buildGraph", "", "project"));
        rpc(state, "orchestrateStep"); // prepare llm task
        auto submit = rpc(state, "submitExternalResult", {
            {"itemId", "x1"},
            {"result", {
                {"generatedCode", "def buildGraph(d):\n    return d"},
                {"confidence", 0.9},
                {"reasoning", "model output"},
                {"tokensGenerated", 20}
            }}
        });
        assert(submit.contains("result"));
        auto item = state.workflow->queue.getItem("x1");
        assert(item.has_value());
        assert(item->status == WI_COMPLETE);
        std::cout << "Test 6 PASSED: external submission closes agent task\n";
        passed++;
    }

    // Test 7: blockers include both external-model and human categories
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("a1", "projectTask", "", "project"));
        state.workflow->queue.enqueue(makeItem("h1", "manualTask", "human"));
        rpc(state, "orchestrateRunDeterministic");
        auto blockers = rpc(state, "getBlockers");
        assert(blockers.contains("result"));
        bool hasHuman = false;
        bool hasExternal = false;
        for (const auto& b : blockers["result"]["blockers"]) {
            if (b["type"] == "needs-human") hasHuman = true;
            if (b["type"] == "needs-external-model") hasExternal = true;
        }
        assert(hasHuman && hasExternal);
        std::cout << "Test 7 PASSED: blocker categories surfaced\n";
        passed++;
    }

    // Test 8: MCP orchestration loop integration (run -> submit -> progress)
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        state.workflow->queue.enqueue(makeItem("d1", "getOne", "template"));
        state.workflow->queue.enqueue(makeItem("x1", "genTwo", "", "project"));

        MCPServer server;
        server.setRpcCallback([&state](const json& request) {
            return handleHeadlessAgentRequest(state, request, "test-session");
        });

        auto runResp = server.handleRequest({
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "tools/call"},
            {"params", {{"name", "whetstone_orchestrate_run_deterministic"},
                        {"arguments", json::object()}}}
        });
        assert(runResp["result"]["isError"].get<bool>() == false);

        server.handleRequest({
            {"jsonrpc", "2.0"},
            {"id", 2},
            {"method", "tools/call"},
            {"params", {{"name", "whetstone_submit_result"},
                        {"arguments", {{"itemId", "x1"},
                                       {"result", {{"generatedCode", "def genTwo():\n    return 2"},
                                                   {"confidence", 0.93},
                                                   {"reasoning", "llm"},
                                                   {"tokensGenerated", 16}}}}}}}
        });

        auto progResp = server.handleRequest({
            {"jsonrpc", "2.0"},
            {"id", 3},
            {"method", "tools/call"},
            {"params", {{"name", "whetstone_get_progress"},
                        {"arguments", json::object()}}}
        });
        assert(progResp["result"]["isError"].get<bool>() == false);
        std::cout << "Test 8 PASSED: MCP orchestrator end-to-end path\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
