// Step 379: Parallel dispatch + batching (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "WorkflowOrchestrator.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name,
                         const std::string& workerType,
                         const std::string& contextWidth = "local",
                         const std::string& priority = "medium") {
    WorkItem w;
    w.id = id;
    w.nodeId = id + "_node";
    w.nodeName = name;
    w.nodeType = "Function";
    w.bufferId = "buf";
    w.contextWidth = contextWidth;
    w.workerType = workerType;
    w.reviewRequired = false;
    w.priority = priority;
    w.status = WI_PENDING;
    w.createdAt = workItemTimestamp();
    return w;
}

int main() {
    int passed = 0;

    // Test 1: three independent tasks advance in one batch
    {
        WorkflowState ws("b1");
        ws.queue.enqueue(makeItem("a", "getA", "template"));
        ws.queue.enqueue(makeItem("b", "getB", "template"));
        ws.queue.enqueue(makeItem("c", "getC", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        assert(r.itemsAdvanced == 3);
        assert(ws.queue.getByStatus(WI_COMPLETE).size() == 3);
        std::cout << "Test 1 PASSED: independent tasks advanced in one batch\n";
        passed++;
    }

    // Test 2: shared project context reports token savings
    {
        WorkflowState ws("b2");
        ws.queue.enqueue(makeItem("p1", "getP1", "template", "project"));
        ws.queue.enqueue(makeItem("p2", "getP2", "template", "project"));
        ws.queue.enqueue(makeItem("p3", "getP3", "template", "project"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);

        std::map<std::string, BufferInfo> buffers;
        BufferInfo bi;
        bi.path = "buf";
        bi.content = "function a(){} function b(){}";
        bi.compactAst = json::array({json{{"id","p1_node"},{"type","Function"},{"name","getP1"}}});
        bi.importGraph = json::object();
        buffers["buf"] = bi;
        orch.setBuffers(buffers);

        auto r = orch.advanceBatch();
        assert(r.contextTokensSaved > 0);
        std::cout << "Test 2 PASSED: shared project context token savings\n";
        passed++;
    }

    // Test 3: batch counts blocked items
    {
        WorkflowState ws("b3");
        ws.queue.enqueue(makeItem("h", "manual", "human"));
        ws.queue.enqueue(makeItem("t", "getT", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        assert(r.itemsAdvanced == 2);
        assert(r.itemsBlocked >= 1);
        std::cout << "Test 3 PASSED: batch blocked count\n";
        passed++;
    }

    // Test 4: dependent task waits until dependency complete
    {
        WorkflowState ws("b4");
        WorkItem a = makeItem("a1", "getA", "template");
        WorkItem b = makeItem("b1", "getB", "template");
        b.dependencies.push_back("a1");
        ws.queue.enqueue(a);
        ws.queue.enqueue(b);
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r1 = orch.advanceBatch();
        assert(r1.itemsAdvanced == 1);
        assert(ws.queue.getByStatus(WI_READY).size() == 1);
        std::cout << "Test 4 PASSED: dependency waits in batch\n";
        passed++;
    }

    // Test 5: priority ordering respected in event stream
    {
        WorkflowState ws("b5");
        ws.queue.enqueue(makeItem("l", "getLow", "template", "local", "low"));
        ws.queue.enqueue(makeItem("h", "getHigh", "template", "local", "high"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        std::string firstRouted;
        for (const auto& e : r.events) {
            if (e.type == "routed") {
                firstRouted = e.itemId;
                break;
            }
        }
        assert(firstRouted == "h");
        std::cout << "Test 5 PASSED: priority ordering in batch\n";
        passed++;
    }

    // Test 6: empty batch result
    {
        WorkflowState ws("b6");
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        assert(r.itemsAdvanced == 0);
        assert(r.events.empty());
        std::cout << "Test 6 PASSED: empty batch\n";
        passed++;
    }

    // Test 7: single-item batch
    {
        WorkflowState ws("b7");
        ws.queue.enqueue(makeItem("x1", "getX", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        assert(r.itemsAdvanced == 1);
        std::cout << "Test 7 PASSED: single-item batch\n";
        passed++;
    }

    // Test 8: mixed independent + blocked tasks
    {
        WorkflowState ws("b8");
        WorkItem a = makeItem("a", "getA", "template");
        WorkItem b = makeItem("b", "getB", "template");
        b.dependencies.push_back("a");
        WorkItem c = makeItem("c", "manual", "human");
        ws.queue.enqueue(a);
        ws.queue.enqueue(b);
        ws.queue.enqueue(c);
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        assert(r.itemsAdvanced == 2); // a + c ready in first pass
        assert(r.itemsBlocked >= 1);
        std::cout << "Test 8 PASSED: mixed independent/dependent batch behavior\n";
        passed++;
    }

    // Test 9: advance() mirrors advanceBatch() event output
    {
        WorkflowState ws("b9");
        ws.queue.enqueue(makeItem("a", "getA", "template"));
        ws.queue.enqueue(makeItem("b", "getB", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto events = orch.advance();
        assert(!events.empty());
        std::cout << "Test 9 PASSED: advance returns batched event stream\n";
        passed++;
    }

    // Test 10: shared-context marker appears in events
    {
        WorkflowState ws("b10");
        ws.queue.enqueue(makeItem("p1", "getP1", "template", "project"));
        ws.queue.enqueue(makeItem("p2", "getP2", "template", "project"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        bool foundShared = false;
        for (const auto& e : r.events) {
            if (e.type == "context-assembled" &&
                e.detail.value("sharedProjectContext", false)) {
                foundShared = true;
            }
        }
        assert(foundShared);
        std::cout << "Test 10 PASSED: shared-context event marker\n";
        passed++;
    }

    // Test 11: review-required item remains review and counted blocked
    {
        WorkflowState ws("b11");
        ws.queue.enqueue(makeItem("r1", "getR", "template", "local"));
        auto maybe = ws.queue.getItem("r1");
        assert(maybe.has_value());
        WorkItem it = maybe.value();
        it.reviewRequired = true;
        ws.queue.updateItem("r1", it);
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto r = orch.advanceBatch();
        assert(!ws.queue.getByStatus(WI_REVIEW).empty());
        assert(r.itemsBlocked == 0); // review is post-execution, not blocked-in-exec
        std::cout << "Test 11 PASSED: review path in batch\n";
        passed++;
    }

    // Test 12: runToCompletion with only blocked tasks stops
    {
        WorkflowState ws("b12");
        ws.queue.enqueue(makeItem("h1", "manual", "human"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto stats = orch.runToCompletion();
        assert(stats.complete == 0);
        assert(!orch.getBlockers().empty());
        std::cout << "Test 12 PASSED: runToCompletion stops on blockers\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
