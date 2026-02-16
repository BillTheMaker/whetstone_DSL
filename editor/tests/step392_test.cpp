// Step 392: Dependency Graph Data (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "DependencyGraph.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name = "doWork",
                         const std::string& workerType = "slm",
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

    // Test 1: Graph has correct node count
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "funcA"));
        wf.queue.enqueue(makeItem("t2", "funcB"));
        wf.queue.enqueue(makeItem("t3", "funcC"));

        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes.size() == 3);
        std::cout << "PASS: test 1 — graph has correct node count\n";
        passed++;
    }

    // Test 2: Edges match dependencies
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "funcA"));
        wf.queue.enqueue(makeItem("t2", "funcB", "slm", "local", "medium", {"t1"}));
        wf.queue.enqueue(makeItem("t3", "funcC", "slm", "local", "medium", {"t2"}));

        auto graph = buildDependencyGraph(wf);
        assert(graph.edges.size() == 2);
        // t1 -> t2
        assert(graph.edges[0].from == "t1");
        assert(graph.edges[0].to == "t2");
        assert(graph.edges[0].type == "depends-on");
        // t2 -> t3
        assert(graph.edges[1].from == "t2");
        assert(graph.edges[1].to == "t3");
        std::cout << "PASS: test 2 — edges match dependencies\n";
        passed++;
    }

    // Test 3: Critical path is longest chain
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "funcA"));
        wf.queue.enqueue(makeItem("t2", "funcB", "slm", "local", "medium", {"t1"}));
        wf.queue.enqueue(makeItem("t3", "funcC", "slm", "local", "medium", {"t2"}));
        wf.queue.enqueue(makeItem("t4", "funcD")); // independent, shorter

        auto graph = buildDependencyGraph(wf);
        auto path = getCriticalPath(graph);
        assert(path.size() == 3); // t1 -> t2 -> t3
        assert(path[0] == "t1");
        assert(path[1] == "t2");
        assert(path[2] == "t3");
        std::cout << "PASS: test 3 — critical path is longest chain\n";
        passed++;
    }

    // Test 4: Node status matches workflow state
    {
        WorkflowState wf("test");
        auto item = makeItem("t1", "funcA");
        item.status = WI_COMPLETE;
        wf.queue.enqueue(item);

        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes.size() == 1);
        assert(graph.nodes[0].status == WI_COMPLETE);
        std::cout << "PASS: test 4 — node status matches workflow state\n";
        passed++;
    }

    // Test 5: Worker type populated from routing
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "funcA", "llm"));
        wf.queue.enqueue(makeItem("t2", "getName", "template"));

        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes[0].workerType == "llm");
        assert(graph.nodes[1].workerType == "template");
        std::cout << "PASS: test 5 — worker type populated\n";
        passed++;
    }

    // Test 6: JSON serialization
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "funcA"));
        wf.queue.enqueue(makeItem("t2", "funcB", "slm", "local", "medium", {"t1"}));

        auto graph = buildDependencyGraph(wf);
        json j = graphToJson(graph);

        assert(j.contains("nodes"));
        assert(j.contains("edges"));
        assert(j["nodes"].size() == 2);
        assert(j["edges"].size() == 1);
        assert(j["nodes"][0]["id"] == "t1");

        // Roundtrip
        auto restored = GraphData::fromJson(j);
        assert(restored.nodes.size() == 2);
        assert(restored.edges.size() == 1);
        std::cout << "PASS: test 6 — JSON serialization\n";
        passed++;
    }

    // Test 7: Empty workflow produces empty graph
    {
        WorkflowState wf("test");
        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes.empty());
        assert(graph.edges.empty());

        auto path = getCriticalPath(graph);
        assert(path.empty());
        std::cout << "PASS: test 7 — empty workflow produces empty graph\n";
        passed++;
    }

    // Test 8: Single-node graph
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "only"));

        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes.size() == 1);
        assert(graph.edges.empty());

        auto path = getCriticalPath(graph);
        assert(path.size() == 1);
        assert(path[0] == "t1");
        std::cout << "PASS: test 8 — single-node graph\n";
        passed++;
    }

    // Test 9: Complex graph (diamond dependencies)
    {
        //     t1
        //    /  \.
        //  t2    t3
        //    \  /
        //     t4
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "root"));
        wf.queue.enqueue(makeItem("t2", "left", "slm", "local", "medium", {"t1"}));
        wf.queue.enqueue(makeItem("t3", "right", "slm", "local", "medium", {"t1"}));
        wf.queue.enqueue(makeItem("t4", "merge", "slm", "local", "medium", {"t2", "t3"}));

        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes.size() == 4);
        assert(graph.edges.size() == 4); // t1->t2, t1->t3, t2->t4, t3->t4

        auto path = getCriticalPath(graph);
        // Longest path: t1 -> t2 -> t4 or t1 -> t3 -> t4 (both length 3)
        assert(path.size() == 3);
        assert(path[0] == "t1");
        assert(path[2] == "t4");
        std::cout << "PASS: test 9 — complex graph (diamond dependencies)\n";
        passed++;
    }

    // Test 10: Graph updates after task completion
    {
        WorkflowState wf("test");
        auto item1 = makeItem("t1", "funcA");
        auto item2 = makeItem("t2", "funcB", "slm", "local", "medium", {"t1"});
        wf.queue.enqueue(item1);
        wf.queue.enqueue(item2);

        auto graph1 = buildDependencyGraph(wf);
        assert(graph1.nodes[0].status == WI_READY);

        // Complete t1
        item1.status = WI_COMPLETE;
        wf.queue.updateItem("t1", item1);

        auto graph2 = buildDependencyGraph(wf);
        bool t1Complete = false;
        for (const auto& n : graph2.nodes) {
            if (n.id == "t1" && n.status == WI_COMPLETE) t1Complete = true;
        }
        assert(t1Complete);
        std::cout << "PASS: test 10 — graph updates after task completion\n";
        passed++;
    }

    // Test 11: Node label and type from work item
    {
        WorkflowState wf("test");
        auto item = makeItem("t1", "myFunction");
        item.nodeType = "Method";
        item.priority = "critical";
        wf.queue.enqueue(item);

        auto graph = buildDependencyGraph(wf);
        assert(graph.nodes[0].label == "myFunction");
        assert(graph.nodes[0].type == "Method");
        assert(graph.nodes[0].priority == "critical");
        std::cout << "PASS: test 11 — node label and type from work item\n";
        passed++;
    }

    // Test 12: 5-function chain critical path = 5
    {
        WorkflowState wf("test");
        wf.queue.enqueue(makeItem("t1", "step1"));
        wf.queue.enqueue(makeItem("t2", "step2", "slm", "local", "medium", {"t1"}));
        wf.queue.enqueue(makeItem("t3", "step3", "slm", "local", "medium", {"t2"}));
        wf.queue.enqueue(makeItem("t4", "step4", "slm", "local", "medium", {"t3"}));
        wf.queue.enqueue(makeItem("t5", "step5", "slm", "local", "medium", {"t4"}));

        auto graph = buildDependencyGraph(wf);
        auto path = getCriticalPath(graph);
        assert(path.size() == 5);
        assert(path[0] == "t1");
        assert(path[4] == "t5");
        std::cout << "PASS: test 12 — 5-function chain critical path\n";
        passed++;
    }

    std::cout << "\nStep 392 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
