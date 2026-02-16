// Step 378: WorkflowOrchestrator main loop (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "WorkflowOrchestrator.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name,
                         const std::string& workerType,
                         bool reviewRequired = false) {
    WorkItem w;
    w.id = id;
    w.nodeId = id + "_node";
    w.nodeName = name;
    w.nodeType = "Function";
    w.bufferId = "buf";
    w.contextWidth = "local";
    w.workerType = workerType;
    w.reviewRequired = reviewRequired;
    w.priority = "medium";
    w.status = WI_PENDING;
    w.createdAt = workItemTimestamp();
    return w;
}

int main() {
    int passed = 0;

    // Test 1: single deterministic/template task completes in one step
    {
        WorkflowState ws("p1");
        ws.queue.enqueue(makeItem("wi1", "getName", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);

        OrchestratorEvent ev = orch.step();
        assert(ev.type == "completed");
        assert(ws.queue.getByStatus(WI_COMPLETE).size() == 1);
        std::cout << "Test 1 PASSED: single template task completes in one step\n";
        passed++;
    }

    // Test 2: dependency cascade after completion
    {
        WorkflowState ws("p2");
        WorkItem a = makeItem("a", "getA", "template");
        WorkItem b = makeItem("b", "getB", "template");
        b.dependencies.push_back("a");
        ws.queue.enqueue(a);
        ws.queue.enqueue(b);

        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        assert(ws.queue.getByStatus(WI_COMPLETE).size() == 1);
        assert(ws.queue.getByStatus(WI_READY).size() == 1); // b promoted
        std::cout << "Test 2 PASSED: dependency cascade promotion\n";
        passed++;
    }

    // Test 3: human task blocks as needs-human
    {
        WorkflowState ws("p3");
        ws.queue.enqueue(makeItem("wi3", "hardTask", "human"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        auto blockers = orch.getBlockers();
        bool hasHuman = false;
        for (const auto& b : blockers) if (b.type == "needs-human") hasHuman = true;
        assert(hasHuman);
        std::cout << "Test 3 PASSED: human task blocks\n";
        passed++;
    }

    // Test 4: SLM task blocks as needs-external-model
    {
        WorkflowState ws("p4");
        ws.queue.enqueue(makeItem("wi4", "fileTask", "slm"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        auto blockers = orch.getBlockers();
        bool hasExternal = false;
        for (const auto& b : blockers) if (b.type == "needs-external-model") hasExternal = true;
        assert(hasExternal);
        std::cout << "Test 4 PASSED: agent task blocks for external model\n";
        passed++;
    }

    // Test 5: runToCompletion processes deterministic tasks
    {
        WorkflowState ws("p5");
        ws.queue.enqueue(makeItem("w1", "getOne", "template"));
        ws.queue.enqueue(makeItem("w2", "getTwo", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto stats = orch.runToCompletion();
        assert(stats.complete == 2);
        std::cout << "Test 5 PASSED: runToCompletion completes deterministic tasks\n";
        passed++;
    }

    // Test 6: advance emits stage events
    {
        WorkflowState ws("p6");
        ws.queue.enqueue(makeItem("wi6", "getX", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto events = orch.advance();
        bool routed = false, context = false, executed = false, completed = false;
        for (const auto& e : events) {
            if (e.type == "routed") routed = true;
            if (e.type == "context-assembled") context = true;
            if (e.type == "executed") executed = true;
            if (e.type == "completed") completed = true;
        }
        assert(routed && context && executed && completed);
        std::cout << "Test 6 PASSED: stage events emitted\n";
        passed++;
    }

    // Test 7: blockers report review items
    {
        WorkflowState ws("p7");
        ws.queue.enqueue(makeItem("wi7", "getY", "template", true)); // force review
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        auto blockers = orch.getBlockers();
        bool hasReview = false;
        for (const auto& b : blockers) if (b.type == "needs-review") hasReview = true;
        assert(hasReview);
        std::cout << "Test 7 PASSED: review blockers reported\n";
        passed++;
    }

    // Test 8: empty workflow returns immediately
    {
        WorkflowState ws("p8");
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto stats = orch.runToCompletion();
        assert(stats.total == 0);
        auto ev = orch.step();
        assert(ev.type == "blocked");
        std::cout << "Test 8 PASSED: empty workflow no-op\n";
        passed++;
    }

    // Test 9: mixed workflow advances what it can
    {
        WorkflowState ws("p9");
        ws.queue.enqueue(makeItem("d1", "getFast", "template"));
        ws.queue.enqueue(makeItem("h1", "manual", "human"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto stats = orch.runToCompletion();
        assert(stats.complete == 1);
        assert(!orch.getBlockers().empty());
        std::cout << "Test 9 PASSED: mixed workflow partially advances\n";
        passed++;
    }

    // Test 10: runUntil predicate can stop early
    {
        WorkflowState ws("p10");
        ws.queue.enqueue(makeItem("a1", "getA", "template"));
        ws.queue.enqueue(makeItem("a2", "getB", "template"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        auto stats = orch.runUntil([](const WorkflowState& s) {
            return s.getStats().complete >= 1;
        });
        assert(stats.complete >= 1);
        std::cout << "Test 10 PASSED: runUntil predicate stop\n";
        passed++;
    }

    // Test 11: blocked event includes blocker detail when applicable
    {
        WorkflowState ws("p11");
        ws.queue.enqueue(makeItem("h2", "manual2", "human"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step(); // process human task first
        OrchestratorEvent ev = orch.step(); // now no ready, returns blocked
        assert(ev.type == "blocked");
        assert(ev.detail.contains("blockers"));
        std::cout << "Test 11 PASSED: blocked event includes blocker detail\n";
        passed++;
    }

    // Test 12: agent task stores context payload in result
    {
        WorkflowState ws("p12");
        ws.queue.enqueue(makeItem("ll1", "llTask", "llm"));
        RoutingEngine routing;
        auto workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate review;
        WorkflowOrchestrator orch(ws, routing, workers, assembler, review);
        orch.step();
        auto item = ws.queue.getItem("ll1");
        assert(item.has_value());
        assert(item->result.astJson.is_object());
        assert(item->result.astJson.contains("workerType"));
        std::cout << "Test 12 PASSED: agent context payload preserved\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
