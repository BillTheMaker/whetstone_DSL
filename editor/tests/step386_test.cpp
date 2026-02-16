// Step 386: Result acceptance protocol (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "ResultAcceptance.h"

static WorkItem makeAgentItem(const std::string& id,
                              const std::string& workerType = "llm") {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = "buildTask";
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.contextWidth = "project";
    item.workerType = workerType;
    item.priority = "high";
    item.status = WI_IN_PROGRESS;
    item.createdAt = workItemTimestamp();
    return item;
}

static json rpc(HeadlessEditorState& state, const std::string& method,
                const json& params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method},
                    {"params", params}};
    return handleHeadlessAgentRequest(state, request, "test-session");
}

static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.openBuffer("main.py", "def seed():\n    return 1\n", "python");
    state.setAgentRole("test-session", AgentRole::Generator);
    state.workflow = WorkflowState("accept");
    return state;
}

int main() {
    int passed = 0;

    // Test 1: valid code submission accepted
    {
        WorkItem item = makeAgentItem("a1");
        ReviewGate gate;
        ReviewPolicy policy;
        policy.defaultAction = "auto-approve";
        ResultSubmission sub;
        sub.itemId = "a1";
        sub.generatedCode = "def buildTask(x):\n    return x\n";
        sub.confidence = 0.9f;
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(acceptance.accepted);
        assert(acceptance.validationPassed);
        std::cout << "Test 1 PASSED: valid submission accepted\n";
        passed++;
    }

    // Test 2: syntax error rejected with validation feedback
    {
        WorkItem item = makeAgentItem("a2");
        ReviewGate gate;
        ReviewPolicy policy;
        ResultSubmission sub;
        sub.itemId = "a2";
        sub.generatedCode = "def broken(:\n";
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(!acceptance.validationPassed);
        assert(!acceptance.validationErrors.empty());
        std::cout << "Test 2 PASSED: syntax errors rejected\n";
        passed++;
    }

    // Test 3: suggested annotation validation flags malformed entries
    {
        WorkItem item = makeAgentItem("a3");
        ReviewGate gate;
        ReviewPolicy policy;
        ResultSubmission sub;
        sub.itemId = "a3";
        sub.generatedCode = "def ok():\n    return 1\n";
        sub.suggestedAnnotations.push_back(json{{"foo", "bar"}});
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(!acceptance.validationPassed);
        assert(acceptance.diagnosticCount >= 1);
        std::cout << "Test 3 PASSED: malformed suggested annotations flagged\n";
        passed++;
    }

    // Test 4: diagnostics prevent auto-approval
    {
        WorkItem item = makeAgentItem("a4");
        ReviewGate gate;
        ReviewPolicy policy;
        policy.defaultAction = "auto-approve";
        ResultSubmission sub;
        sub.itemId = "a4";
        sub.generatedCode = "def bad(:\n";
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(!acceptance.autoApproved);
        assert(!acceptance.accepted);
        std::cout << "Test 4 PASSED: diagnostics block auto-approval\n";
        passed++;
    }

    // Test 5: low-confidence valid result goes to review under strict policy
    {
        WorkItem item = makeAgentItem("a5");
        ReviewGate gate;
        ReviewPolicy policy;
        policy.defaultAction = "require-review";
        ResultSubmission sub;
        sub.itemId = "a5";
        sub.generatedCode = "def ok(v):\n    return v\n";
        sub.confidence = 0.1f;
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(acceptance.accepted);
        assert(acceptance.reviewRequired);
        std::cout << "Test 5 PASSED: low-confidence result routed to review\n";
        passed++;
    }

    // Test 6: suggested annotations attached to output result payload
    {
        WorkItem item = makeAgentItem("a6");
        ReviewGate gate;
        ReviewPolicy policy;
        ResultSubmission sub;
        sub.itemId = "a6";
        sub.generatedCode = "def ok():\n    return 1\n";
        sub.suggestedAnnotations.push_back(
            json{{"nodeId", "n1"}, {"annotationType", "Intent"}});
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(acceptance.validationPassed);
        assert(out.astJson.contains("suggestedAnnotations"));
        std::cout << "Test 6 PASSED: suggested annotations preserved\n";
        passed++;
    }

    // Test 7: acceptance pipeline returns combined diagnostic counts
    {
        WorkItem item = makeAgentItem("a7");
        ReviewGate gate;
        ReviewPolicy policy;
        ResultSubmission sub;
        sub.itemId = "a7";
        sub.generatedCode = "def bad(:\n";
        sub.suggestedAnnotations.push_back(json{{"foo", "bar"}});
        WorkItemResult out;
        auto acceptance = evaluateResultSubmission(sub, item, "python", gate, policy, out);
        assert(acceptance.diagnosticCount >= 2);
        std::cout << "Test 7 PASSED: validation stages aggregate diagnostics\n";
        passed++;
    }

    // Test 8: partial acceptance (valid code but review required)
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeAgentItem("a8"));
        auto resp = rpc(state, "submitExternalResult", {
            {"itemId", "a8"},
            {"result", {
                {"generatedCode", "def buildTask(x):\n    return x\n"},
                {"confidence", 0.2},
                {"reasoning", "uncertain"}
            }}
        });
        assert(resp.contains("result"));
        assert(resp["result"]["acceptance"]["accepted"].get<bool>());
        assert(resp["result"]["acceptance"]["reviewRequired"].get<bool>());
        std::cout << "Test 8 PASSED: partial acceptance path works\n";
        passed++;
    }

    // Test 9: multiple submissions for same item keep latest result
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeAgentItem("a9"));
        rpc(state, "submitExternalResult", {
            {"itemId", "a9"},
            {"result", {
                {"generatedCode", "def buildTask(x):\n    return x + 1\n"},
                {"confidence", 0.3}
            }}
        });
        auto second = rpc(state, "submitExternalResult", {
            {"itemId", "a9"},
            {"result", {
                {"generatedCode", "def buildTask(x):\n    return x + 2\n"},
                {"confidence", 0.4}
            }}
        });
        assert(second.contains("result"));
        auto item = state.workflow->queue.getItem("a9");
        assert(item.has_value());
        assert(item->result.generatedCode.find("+ 2") != std::string::npos);
        std::cout << "Test 9 PASSED: latest submission wins\n";
        passed++;
    }

    // Test 10: acceptance completion cascades dependencies
    {
        auto state = makeState();
        state.reviewPolicy.defaultAction = "auto-approve";
        WorkItem a = makeAgentItem("a10");
        WorkItem b = makeAgentItem("b10");
        b.status = WI_PENDING;
        b.dependencies.push_back("a10");
        state.workflow->queue.enqueue(a);
        state.workflow->queue.enqueue(b);
        auto resp = rpc(state, "submitExternalResult", {
            {"itemId", "a10"},
            {"result", {
                {"generatedCode", "def buildTask(x):\n    return x\n"},
                {"confidence", 0.95}
            }}
        });
        assert(resp["result"]["acceptance"]["autoApproved"].get<bool>());
        auto dep = state.workflow->queue.getItem("b10");
        assert(dep.has_value());
        assert(dep->status == WI_READY);
        std::cout << "Test 10 PASSED: dependency cascade on acceptance\n";
        passed++;
    }

    // Test 11: submitExternalResult response includes acceptance payload
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeAgentItem("a11"));
        auto resp = rpc(state, "submitExternalResult", {
            {"itemId", "a11"},
            {"result", {
                {"generatedCode", "def buildTask(x):\n    return x\n"},
                {"confidence", 0.3}
            }}
        });
        assert(resp.contains("result"));
        assert(resp["result"].contains("acceptance"));
        assert(resp["result"]["acceptance"].contains("validationPassed"));
        std::cout << "Test 11 PASSED: acceptance payload exposed in RPC response\n";
        passed++;
    }

    // Test 12: non-agent worker submissions are rejected
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeAgentItem("a12", "template"));
        auto resp = rpc(state, "submitExternalResult", {
            {"itemId", "a12"},
            {"result", {{"generatedCode", "return 1"}}}
        });
        assert(resp.contains("error"));
        std::cout << "Test 12 PASSED: non-agent worker guard preserved\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
