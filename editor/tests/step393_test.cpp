// Step 393: Phase 15c Integration + Sprint 15 Summary (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "RoutingRules.h"
#include "CostEstimator.h"
#include "WorkflowTemplates.h"
#include "DependencyGraph.h"
#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "WorkflowOrchestrator.h"
#include "WorkflowProtocol.h"
#include "EventStream.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name = "doWork",
                         const std::string& workerType = "",
                         const std::string& contextWidth = "local",
                         const std::string& priority = "medium",
                         const std::vector<std::string>& deps = {}) {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = name;
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.workerType = workerType;
    item.contextWidth = contextWidth;
    item.priority = priority;
    item.status = WI_READY;
    item.createdAt = workItemTimestamp();
    item.dependencies = deps;
    return item;
}

int main() {
    int passed = 0;

    // Test 1: Custom routing rules override default to use template for simple functions
    {
        RulesEngine engine;
        // Add custom rule: all local functions → template
        RoutingRule custom;
        custom.name = "local-template";
        custom.priority = 1;
        custom.conditions.push_back({"contextWidth", "", "==", "local"});
        custom.action.workerType = "template";
        engine.addRule(custom);

        auto item = makeItem("t1", "processData", "", "local");
        auto decision = engine.evaluate(item);
        assert(decision.workerType == "template");
        assert(decision.reasoning.find("local-template") != std::string::npos);
        std::cout << "PASS: test 1 — custom routing rules override\n";
        passed++;
    }

    // Test 2: Cost estimation for 10-function workflow with optimization suggestions
    {
        WorkflowState wf("test");
        // 3 getters in project context (should suggest template)
        wf.queue.enqueue(makeItem("t1", "getName", "", "project"));
        wf.queue.enqueue(makeItem("t2", "getValue", "", "project"));
        wf.queue.enqueue(makeItem("t3", "isActive", "", "project"));
        // 7 normal functions
        for (int i = 4; i <= 10; ++i) {
            wf.queue.enqueue(makeItem("t" + std::to_string(i), "func" + std::to_string(i), "", "local"));
        }

        RoutingEngine routing;
        CostEstimator estimator;
        auto est = estimator.estimate(wf, routing);

        assert(est.totalContextTokens > 0);
        assert(est.totalOutputTokens > 0);

        // Should have use-template suggestion for the getters
        bool hasTemplateSuggestion = false;
        for (const auto& s : est.optimizationSuggestions) {
            if (s.action == "use-template") {
                hasTemplateSuggestion = true;
            }
        }
        assert(hasTemplateSuggestion);
        std::cout << "PASS: test 2 — cost estimation with suggestions\n";
        passed++;
    }

    // Test 3: CRUD template → route → deterministic completes
    {
        TemplateParams params;
        params.entityName = "Task";
        auto wf = WorkflowTemplates::applyTemplate("crud-api", params);

        RoutingEngine routing;
        WorkerRegistry workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate gate;

        WorkflowOrchestrator orchestrator(wf, routing, workers, assembler, gate);
        auto stats = orchestrator.runToCompletion();

        // Template (getTask) and deterministic should auto-complete
        // LLM/SLM tasks will block at needs-external-model
        assert(stats.complete > 0 || stats.inProgress > 0);
        std::cout << "PASS: test 3 — CRUD template workflow execution\n";
        passed++;
    }

    // Test 4: Dependency graph 5-function chain → critical path = 5
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "step1"));
        wf.queue.enqueue(makeItem("t2", "step2", "", "local", "medium", {"t1"}));
        wf.queue.enqueue(makeItem("t3", "step3", "", "local", "medium", {"t2"}));
        wf.queue.enqueue(makeItem("t4", "step4", "", "local", "medium", {"t3"}));
        wf.queue.enqueue(makeItem("t5", "step5", "", "local", "medium", {"t4"}));

        auto graph = buildDependencyGraph(wf);
        auto path = getCriticalPath(graph);
        assert(path.size() == 5);
        assert(path[0] == "t1");
        assert(path[4] == "t5");
        std::cout << "PASS: test 4 — dependency graph critical path\n";
        passed++;
    }

    // Test 5: Event stream captures workflow transitions
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "getName", "template", "local"));

        RoutingEngine routing;
        WorkerRegistry workers = WorkerRegistry::getDefaultRegistry();
        ContextAssembler assembler;
        ReviewGate gate;
        EventStream stream;

        WorkflowOrchestrator orchestrator(wf, routing, workers, assembler, gate);
        auto events = orchestrator.advance();

        // Emit events to stream
        for (const auto& ev : events) {
            stream.emit(ev);
        }

        assert(stream.getVersion() > 0);
        auto recent = stream.getRecent(10);
        assert(!recent.empty());

        // Check that we see routing and execution events
        bool hasRouted = false, hasExecuted = false;
        for (const auto& se : recent) {
            if (se.event.type == "routed") hasRouted = true;
            if (se.event.type == "executed") hasExecuted = true;
        }
        assert(hasRouted);
        assert(hasExecuted);
        std::cout << "PASS: test 5 — event stream captures transitions\n";
        passed++;
    }

    // Test 6: Full MCP client simulation through protocol phases
    {
        WorkflowSession session;
        session.sessionId = "test-session";
        session.projectName = "integration-test";

        // Phase transitions
        assert(validateTransition("init", "model"));
        assert(validateTransition("model", "annotate"));
        assert(validateTransition("annotate", "plan"));
        assert(validateTransition("plan", "route"));
        assert(validateTransition("route", "execute"));
        assert(validateTransition("execute", "assist"));
        assert(validateTransition("assist", "review"));
        assert(validateTransition("review", "complete"));
        assert(!validateTransition("complete", "init")); // can't go backward

        // getNextAction suggests correct phase
        auto action = getNextAction(session);
        assert(action.phase == "model" || action.phase == "init");
        std::cout << "PASS: test 6 — protocol phase validation\n";
        passed++;
    }

    // Test 7: Optimization: apply suggestion to narrow context → lower cost
    {
        WorkflowState wf("test");
        // Getter at project width (wasteful)
        wf.queue.enqueue(makeItem("t1", "getName", "", "project"));

        RoutingEngine routing;
        CostEstimator estimator;

        auto est1 = estimator.estimate(wf, routing);
        int originalCost = est1.totalContextTokens;

        // Apply optimization: narrow to local
        auto item = wf.queue.getItem("t1");
        WorkItem updated = item.value();
        updated.contextWidth = "local";
        wf.queue.updateItem("t1", updated);

        auto est2 = estimator.estimate(wf, routing);
        int optimizedCost = est2.totalContextTokens;

        assert(optimizedCost < originalCost);
        std::cout << "PASS: test 7 — optimization reduces cost\n";
        passed++;
    }

    // Test 8: Sprint 15 totals verification
    {
        // Verify all new components exist and work together
        // RoutingRules
        auto defaults = RulesEngine::getDefaultRules();
        assert(defaults.size() >= 5);

        // CostEstimator
        CostEstimator estimator;
        WorkflowState emptyWf("empty");
        RoutingEngine routing;
        auto est = estimator.estimate(emptyWf, routing);
        assert(est.totalContextTokens == 0);

        // WorkflowTemplates
        auto templates = WorkflowTemplates::getTemplates();
        assert(templates.size() == 5);

        // DependencyGraph
        auto graph = buildDependencyGraph(emptyWf);
        assert(graph.nodes.empty());

        // EventStream
        EventStream stream;
        assert(stream.getVersion() == 0);

        // WorkflowProtocol
        auto phases = workflowProtocolPhases();
        assert(phases.size() == 9);

        std::cout << "PASS: test 8 — Sprint 15 totals verification\n";
        passed++;
    }

    std::cout << "\nStep 393 result: " << passed << "/8 tests passed\n";
    return (passed == 8) ? 0 : 1;
}
