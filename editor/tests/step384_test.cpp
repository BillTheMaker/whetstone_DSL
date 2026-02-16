// Step 384: Workflow session protocol (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "WorkflowProtocol.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& workerType,
                         const std::string& status) {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = id;
    item.nodeType = "Function";
    item.bufferId = "buf";
    item.workerType = workerType;
    item.contextWidth = "local";
    item.priority = "medium";
    item.status = status;
    item.createdAt = workItemTimestamp();
    return item;
}

int main() {
    int passed = 0;

    // Test 1: protocol phase ordering transitions are valid
    {
        auto phases = workflowProtocolPhases();
        for (size_t i = 1; i < phases.size(); ++i) {
            assert(validateTransition(phases[i - 1], phases[i]));
        }
        std::cout << "Test 1 PASSED: phase ordering transition validity\n";
        passed++;
    }

    // Test 2: invalid transition is rejected
    {
        assert(!validateTransition("init", "route"));
        assert(!validateTransition("execute", "annotate"));
        std::cout << "Test 2 PASSED: invalid transitions rejected\n";
        passed++;
    }

    // Test 3: next action suggestions map to phases
    {
        WorkflowSession s;
        s.currentPhase = "plan";
        auto a = getNextAction(s);
        assert(a.command == "whetstone_create_workflow");
        s.currentPhase = "execute";
        auto b = getNextAction(s);
        assert(b.command == "whetstone_orchestrate_run_deterministic");
        s.currentPhase = "complete";
        auto c = getNextAction(s);
        assert(c.command == "whetstone_save_workflow");
        std::cout << "Test 3 PASSED: next-action hints per phase\n";
        passed++;
    }

    // Test 4: session tracks command history
    {
        WorkflowSession s;
        recordWorkflowCommand(s, "whetstone_create_skeleton");
        recordWorkflowCommand(s, "whetstone_add_skeleton_node");
        recordWorkflowCommand(s, "whetstone_create_workflow");
        assert(s.commands.size() == 3);
        assert(s.commands[0] == "whetstone_create_skeleton");
        std::cout << "Test 4 PASSED: command history tracked\n";
        passed++;
    }

    // Test 5: summary includes core session fields
    {
        WorkflowSession s;
        s.sessionId = "sess-1";
        s.projectName = "proj";
        s.currentPhase = "model";
        recordWorkflowCommand(s, "whetstone_create_skeleton");
        auto summary = getSessionSummary(s);
        assert(summary["sessionId"] == "sess-1");
        assert(summary["projectName"] == "proj");
        assert(summary["commandCount"].get<int>() == 1);
        assert(summary.contains("nextAction"));
        std::cout << "Test 5 PASSED: summary fields correct\n";
        passed++;
    }

    // Test 6: phase auto-detection from complete workflow
    {
        WorkflowState wf("p");
        WorkItem c = makeItem("c1", "template", WI_COMPLETE);
        wf.queue.enqueue(c);
        auto phase = detectProtocolPhaseFromWorkflow(wf);
        assert(phase == "complete");
        std::cout << "Test 6 PASSED: complete workflow auto-detected\n";
        passed++;
    }

    // Test 7: phase auto-detection for assist stage
    {
        WorkflowState wf("p");
        wf.queue.enqueue(makeItem("a1", "llm", WI_IN_PROGRESS));
        auto phase = detectProtocolPhaseFromWorkflow(wf);
        assert(phase == "assist");
        std::cout << "Test 7 PASSED: assist phase auto-detected\n";
        passed++;
    }

    // Test 8: session JSON persistence roundtrip
    {
        WorkflowSession s;
        s.sessionId = "sess-2";
        s.projectName = "alpha";
        s.currentPhase = "route";
        s.commands = {"whetstone_create_workflow", "whetstone_orchestrate_advance"};
        auto j = s.toJson();
        auto restored = WorkflowSession::fromJson(j);
        assert(restored.sessionId == s.sessionId);
        assert(restored.projectName == s.projectName);
        assert(restored.currentPhase == s.currentPhase);
        assert(restored.commands.size() == 2);
        std::cout << "Test 8 PASSED: session persistence roundtrip\n";
        passed++;
    }

    // Test 9: empty project protocol remains in planning path
    {
        WorkflowSession s;
        s.currentPhase = "plan";
        WorkflowState wf("empty");
        auto next = getNextAction(s, &wf);
        assert(next.command == "whetstone_create_workflow");
        std::cout << "Test 9 PASSED: empty project plan behavior\n";
        passed++;
    }

    // Test 10: unknown phases are rejected as protocol phases
    {
        assert(!isWorkflowProtocolPhase("unknown-phase"));
        assert(!validateTransition("unknown-phase", "init"));
        std::cout << "Test 10 PASSED: unknown phase guards\n";
        passed++;
    }

    // Test 11: recorded commands advance session phase monotonically
    {
        WorkflowSession s;
        s.currentPhase = "init";
        recordWorkflowCommand(s, "whetstone_create_skeleton");
        assert(s.currentPhase == "model");
        recordWorkflowCommand(s, "whetstone_orchestrate_advance");
        assert(s.currentPhase == "route");
        recordWorkflowCommand(s, "whetstone_create_workflow");
        assert(s.currentPhase == "route"); // no backward transition
        std::cout << "Test 11 PASSED: monotonic phase advancement by command\n";
        passed++;
    }

    // Test 12: workflow-aware next action can override stale session phase
    {
        WorkflowSession s;
        s.currentPhase = "plan";
        WorkflowState wf("assist");
        wf.queue.enqueue(makeItem("x1", "human", WI_IN_PROGRESS));
        auto next = getNextAction(s, &wf);
        assert(next.phase == "assist");
        assert(next.command == "whetstone_get_blockers");
        std::cout << "Test 12 PASSED: workflow-aware next action override\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
