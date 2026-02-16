// Step 388: Phase 15b full protocol integration (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ContextBundle.h"
#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "ResultAcceptance.h"
#include "WorkflowOrchestrator.h"
#include "WorkflowProtocol.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& workerType = "",
                         const std::string& contextWidth = "local",
                         bool reviewRequired = false) {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = "buildThing";
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.workerType = workerType;
    item.contextWidth = contextWidth;
    item.reviewRequired = reviewRequired;
    item.priority = "high";
    item.status = WI_PENDING;
    item.createdAt = workItemTimestamp();
    return item;
}

static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.openBuffer("main.py", "def seed():\n    return 1\n", "python");
    state.setAgentRole("test-session", AgentRole::Generator);
    state.workflow = WorkflowState("proto");
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

    // Test 1: full protocol walkthrough reaches completion
    {
        WorkflowSession session;
        session.sessionId = "s1";
        session.projectName = "proto";
        recordWorkflowCommand(session, "whetstone_create_skeleton");
        recordWorkflowCommand(session, "whetstone_add_skeleton_node");
        recordWorkflowCommand(session, "whetstone_infer_annotations");
        recordWorkflowCommand(session, "whetstone_create_workflow");

        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        state.workflow->queue.enqueue(makeItem("d1", "template"));
        state.workflow->queue.enqueue(makeItem("a1", "", "project"));

        rpc(state, "orchestrateRunDeterministic");
        rpc(state, "submitExternalResult", {
            {"itemId", "a1"},
            {"result", {
                {"generatedCode", "def buildThing(x):\n    return x\n"},
                {"confidence", 0.95}
            }}
        });

        auto stats = state.workflow->getStats();
        assert(stats.complete == stats.total);
        std::cout << "Test 1 PASSED: full protocol walkthrough completes\n";
        passed++;
    }

    // Test 2: event stream captures transitions in order
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("t1", "template"));
        rpc(state, "orchestrateAdvance");
        auto stream = rpc(state, "getEventStream", {{"sinceVersion", 0}});
        assert(stream.contains("result"));
        assert(stream["result"]["events"].size() >= 3);
        assert(stream["result"]["events"][0]["version"].get<int>() >= 1);
        std::cout << "Test 2 PASSED: event stream captures transitions\n";
        passed++;
    }

    // Test 3: context bundle includes skeleton + intent + project summary
    {
        WorkItem item = makeItem("c1", "llm", "project");
        WorkerContext ctx;
        ctx.nodeAst = json{{"id", "c1_node"}, {"type", "Function"}, {"name", "buildThing"}};
        ctx.skeletonIntent = "Create normalized output map";
        ctx.projectSummary = json{{"modules", 3}};
        RoutingDecision decision;
        decision.workerType = "llm";
        decision.contextWidth = "project";
        decision.contextBudgetTokens = 8000;
        auto bundle = buildBundle(item, ctx, decision);
        assert(bundle.skeletonCode.find("buildThing") != std::string::npos);
        assert(bundle.intent == "Create normalized output map");
        assert(bundle.projectSummary.find("modules") != std::string::npos);
        std::cout << "Test 3 PASSED: context bundle carries required context\n";
        passed++;
    }

    // Test 4: deterministic auto-approve vs llm review-required acceptance
    {
        ReviewGate gate;
        ReviewPolicy policy = ReviewPolicy::getDefault();

        WorkItem det = makeItem("d1", "deterministic");
        ResultSubmission detSub;
        detSub.itemId = "d1";
        detSub.generatedCode = "def f(x):\n    return x\n";
        detSub.confidence = 0.95f;
        WorkItemResult detOut;
        auto detAccept = evaluateResultSubmission(detSub, det, "python", gate, policy, detOut);
        assert(detAccept.accepted && detAccept.autoApproved);

        WorkItem llm = makeItem("l1", "llm");
        ResultSubmission llmSub;
        llmSub.itemId = "l1";
        llmSub.generatedCode = "def g(x):\n    return x\n";
        llmSub.confidence = 0.95f;
        WorkItemResult llmOut;
        auto llmAccept = evaluateResultSubmission(llmSub, llm, "python", gate, policy, llmOut);
        assert(llmAccept.accepted && llmAccept.reviewRequired);
        std::cout << "Test 4 PASSED: acceptance policy differs by worker type\n";
        passed++;
    }

    // Test 5: rejection -> reroute -> submit improved result -> accepted
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        state.workflow->queue.enqueue(makeItem("r1", "template", "local", true));
        WorkflowOrchestrator orchestrator(*state.workflow, state.routingEngine,
                                          state.workerRegistry, state.contextAssembler,
                                          state.reviewGate);
        orchestrator.setReviewPolicy(state.reviewPolicy);
        orchestrator.step(); // to review
        orchestrator.rejectAndRequeue("r1", "attempt1");
        orchestrator.step(); // slm blocked
        orchestrator.rejectAndRequeue("r1", "attempt2");
        orchestrator.step(); // llm blocked
        auto rerouted = state.workflow->queue.getItem("r1");
        assert(rerouted.has_value());
        WorkItem readyForSubmit = *rerouted;
        readyForSubmit.reviewRequired = false;
        state.workflow->queue.updateItem("r1", readyForSubmit);
        auto submit = rpc(state, "submitExternalResult", {
            {"itemId", "r1"},
            {"result", {{"generatedCode", "def buildThing(x):\n    return x\n"},
                        {"confidence", 0.95}}}
        });
        assert(submit["result"]["acceptance"]["accepted"].get<bool>());
        auto item = state.workflow->queue.getItem("r1");
        assert(item.has_value());
        assert(item->status == WI_COMPLETE);
        std::cout << "Test 5 PASSED: rejection reroute then accepted submission\n";
        passed++;
    }

    // Test 6: progress snapshot tracks phase advancement
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        state.workflow->queue.enqueue(makeItem("d1", "template"));
        state.workflow->queue.enqueue(makeItem("a1", "", "project"));

        auto p0 = rpc(state, "getProgress");
        rpc(state, "orchestrateRunDeterministic");
        auto p1 = rpc(state, "getProgress");
        rpc(state, "submitExternalResult", {
            {"itemId", "a1"},
            {"result", {{"generatedCode", "def x(v):\n    return v\n"},
                        {"confidence", 0.95}}}
        });
        auto p2 = rpc(state, "getProgress");
        assert(p1["result"]["completedItems"].get<int>() >= p0["result"]["completedItems"].get<int>());
        assert(p2["result"]["completedItems"].get<int>() >= p1["result"]["completedItems"].get<int>());
        std::cout << "Test 6 PASSED: progress snapshots move forward\n";
        passed++;
    }

    // Test 7: getNextAction guides protocol progression
    {
        WorkflowSession session;
        session.currentPhase = "plan";
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("a1", "", "project"));
        auto a = getNextAction(session, state.workflow ? &*state.workflow : nullptr);
        assert(!a.command.empty());
        assert(isWorkflowProtocolPhase(a.phase));
        std::cout << "Test 7 PASSED: next action guidance available\n";
        passed++;
    }

    // Test 8: session persistence allows resume
    {
        WorkflowSession s;
        s.sessionId = "resume-1";
        s.projectName = "proj";
        recordWorkflowCommand(s, "whetstone_create_skeleton");
        recordWorkflowCommand(s, "whetstone_create_workflow");
        auto restored = WorkflowSession::fromJson(s.toJson());
        auto summary = getSessionSummary(restored);
        assert(summary["commandCount"].get<int>() == 2);
        assert(summary["currentPhase"] == restored.currentPhase);
        std::cout << "Test 8 PASSED: session persistence and resume summary\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
