// Step 380: Feedback loop rejection re-routing (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "WorkflowOrchestrator.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name,
                         const std::string& workerType,
                         bool reviewRequired = false,
                         const std::string& contextWidth = "local") {
    WorkItem w;
    w.id = id;
    w.nodeId = id + "_node";
    w.nodeName = name;
    w.nodeType = "Function";
    w.bufferId = "buf";
    w.contextWidth = contextWidth;
    w.workerType = workerType;
    w.reviewRequired = reviewRequired;
    w.priority = "medium";
    w.status = WI_PENDING;
    w.createdAt = workItemTimestamp();
    return w;
}

int main() {
    int passed = 0;

    // Test 1: review rejection re-enters ready queue
    {
        WorkflowState ws("r1");
        ws.queue.enqueue(makeItem("w1", "getName", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step(); // sent-to-review
        assert(ws.queue.getByStatus(WI_REVIEW).size() == 1);
        assert(orch.rejectAndRequeue("w1", "fix style", "alice"));
        auto item = ws.queue.getItem("w1");
        assert(item.has_value());
        assert(item->status == WI_READY);
        std::cout << "Test 1 PASSED: rejected item re-enters ready queue\n";
        passed++;
    }

    // Test 2: feedback is included in next context bundle
    {
        WorkflowState ws("r2");
        ws.queue.enqueue(makeItem("w2", "getData", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w2", "needs edge-case handling"));
        orch.step(); // escalated to slm
        auto item = ws.queue.getItem("w2");
        assert(item.has_value());
        assert(item->workerType == "slm");
        assert(item->result.astJson.value("feedbackFromRejection", "") ==
               "needs edge-case handling");
        std::cout << "Test 2 PASSED: feedback carried into next context\n";
        passed++;
    }

    // Test 3: deterministic/template rejection escalates to SLM
    {
        WorkflowState ws("r3");
        ws.queue.enqueue(makeItem("w3", "getX", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w3", "not robust enough"));
        orch.step();
        auto item = ws.queue.getItem("w3");
        assert(item.has_value());
        assert(item->workerType == "slm");
        std::cout << "Test 3 PASSED: first escalation is template->slm\n";
        passed++;
    }

    // Test 4: second rejection escalates SLM to LLM
    {
        WorkflowState ws("r4");
        ws.queue.enqueue(makeItem("w4", "getY", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w4", "attempt 1"));
        orch.step(); // slm blocked in-progress
        assert(orch.rejectAndRequeue("w4", "attempt 2"));
        orch.step(); // llm blocked in-progress
        auto item = ws.queue.getItem("w4");
        assert(item.has_value());
        assert(item->workerType == "llm");
        std::cout << "Test 4 PASSED: second escalation is slm->llm\n";
        passed++;
    }

    // Test 5: third rejection escalates LLM to human
    {
        WorkflowState ws("r5");
        ws.queue.enqueue(makeItem("w5", "getZ", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w5", "attempt 1"));
        orch.step();
        assert(orch.rejectAndRequeue("w5", "attempt 2"));
        orch.step();
        assert(orch.rejectAndRequeue("w5", "attempt 3"));
        orch.step();
        auto item = ws.queue.getItem("w5");
        assert(item.has_value());
        assert(item->workerType == "human");
        std::cout << "Test 5 PASSED: third escalation is llm->human\n";
        passed++;
    }

    // Test 6: rejection history preserves worker/result/feedback metadata
    {
        WorkflowState ws("r6");
        WorkItem i = makeItem("w6", "getMeta", "template", true);
        i.result.generatedCode = "return self.meta";
        ws.queue.enqueue(i);
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w6", "metadata missing", "reviewer-1"));
        auto item = ws.queue.getItem("w6");
        assert(item.has_value());
        assert(item->rejectionHistory.size() == 1);
        assert(item->rejectionHistory[0].workerType == "template");
        assert(item->rejectionHistory[0].feedback == "metadata missing");
        assert(item->rejectionHistory[0].rejectedBy == "reviewer-1");
        std::cout << "Test 6 PASSED: rejection history metadata preserved\n";
        passed++;
    }

    // Test 7: multiple rejections accumulate in history
    {
        WorkflowState ws("r7");
        ws.queue.enqueue(makeItem("w7", "getAcc", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w7", "first"));
        orch.step();
        assert(orch.rejectAndRequeue("w7", "second"));
        auto item = ws.queue.getItem("w7");
        assert(item.has_value());
        assert(item->rejectionHistory.size() == 2);
        std::cout << "Test 7 PASSED: multiple rejection attempts accumulate\n";
        passed++;
    }

    // Test 8: reviewer annotation hints apply to queued item
    {
        WorkflowState ws("r8");
        ws.queue.enqueue(makeItem("w8", "getHints", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w8", "context=project worker=llm"));
        auto item = ws.queue.getItem("w8");
        assert(item.has_value());
        assert(item->contextWidth == "project");
        assert(item->workerType == "llm");
        std::cout << "Test 8 PASSED: feedback hints applied to item annotations\n";
        passed++;
    }

    // Test 9: rerouted item widens context over original
    {
        WorkflowState ws("r9");
        ws.queue.enqueue(makeItem("w9", "getWide", "template", true, "local"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w9", "needs broader context"));
        orch.step();
        auto item = ws.queue.getItem("w9");
        assert(item.has_value());
        assert(item->contextWidth == "file");
        std::cout << "Test 9 PASSED: context width escalates after rejection\n";
        passed++;
    }

    // Test 10: escalation does not skip levels
    {
        WorkflowState ws("r10");
        ws.queue.enqueue(makeItem("w10", "getNoSkip", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(orch.rejectAndRequeue("w10", "attempt 1"));
        orch.step();
        auto afterFirst = ws.queue.getItem("w10");
        assert(afterFirst.has_value());
        assert(afterFirst->workerType == "slm");
        assert(orch.rejectAndRequeue("w10", "attempt 2"));
        orch.step();
        auto afterSecond = ws.queue.getItem("w10");
        assert(afterSecond.has_value());
        assert(afterSecond->workerType == "llm");
        std::cout << "Test 10 PASSED: escalation remains stepwise\n";
        passed++;
    }

    // Test 11: rejected human task remains human
    {
        WorkflowState ws("r11");
        ws.queue.enqueue(makeItem("w11", "manualTask", "human"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step(); // human blocked
        assert(orch.rejectAndRequeue("w11", "still needs manual intervention"));
        orch.step();
        auto item = ws.queue.getItem("w11");
        assert(item.has_value());
        assert(item->workerType == "human");
        std::cout << "Test 11 PASSED: human rejection remains human\n";
        passed++;
    }

    // Test 12: rejection count tracked and invalid reject state denied
    {
        WorkflowState ws("r12");
        ws.queue.enqueue(makeItem("w12", "getCount", "template", true));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        // Not in review/in-progress yet
        assert(!orch.rejectAndRequeue("w12", "too early"));
        orch.step();
        assert(orch.rejectAndRequeue("w12", "now reject"));
        auto item = ws.queue.getItem("w12");
        assert(item.has_value());
        assert(static_cast<int>(item->rejectionHistory.size()) == 1);
        std::cout << "Test 12 PASSED: rejection count tracked and status guarded\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
