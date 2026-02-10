// Step 224: Evaluation harness tests.

#include <cassert>
#include <string>
#include "EvalHarness.h"

int main() {
    EvalTask task;
    task.id = "t1";
    task.expectedOutcome = {{"toolCalls", {"whetstone_get_ast"}}};

    Trace good;
    good.id = "t1";
    good.steps.push_back({"tool_call", "", "whetstone_get_ast", {}, {}});
    good.toolCallCount = 1;

    EvalResult r1 = EvalRunner::evaluateTraceForTask(task, good);
    assert(r1.score == 1.0);
    assert(r1.passed);

    Trace bad;
    bad.id = "t1";
    EvalResult r2 = EvalRunner::evaluateTraceForTask(task, bad);
    assert(r2.score == 0.0);
    assert(!r2.passed);

    EvalTask partial;
    partial.id = "t2";
    partial.allowPartial = true;
    partial.expectedOutcome = {{"toolCalls", {"a", "b"}}};
    Trace partialTrace;
    partialTrace.id = "t2";
    partialTrace.steps.push_back({"tool_call", "", "a", {}, {}});
    EvalResult r3 = EvalRunner::evaluateTraceForTask(partial, partialTrace);
    assert(r3.score > 0.0 && r3.score < 1.0);
    assert(r3.passed);

    printf("step224_test: all assertions passed\n");
    return 0;
}
