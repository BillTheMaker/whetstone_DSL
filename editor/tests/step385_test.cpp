// Step 385: Context bundle format (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "ContextBundle.h"

static WorkItem makeItem(const std::string& id = "w1") {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = "buildThing";
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.contextWidth = "project";
    item.workerType = "llm";
    item.reviewRequired = true;
    item.priority = "high";
    item.status = WI_READY;
    item.createdAt = workItemTimestamp();
    return item;
}

static WorkerContext makeContext() {
    WorkerContext ctx;
    ctx.nodeAst = json{{"id", "w1_node"}, {"type", "Function"}, {"name", "buildThing"}};
    ctx.bufferContent = "def helper():\n    return 1\n";
    ctx.projectSummary = json{{"files", 4}, {"keyModules", json::array({"core", "api"})}};
    ctx.annotations = {
        json{{"type", "Contract"}, {"name", "precondition"}, {"value", "input non-empty"}},
        json{{"type", "Risk"}, {"name", "level"}, {"value", "medium"}}
    };
    ctx.skeletonIntent = "Build normalized result map";
    return ctx;
}

static RoutingDecision makeDecision() {
    RoutingDecision d;
    d.workerType = "llm";
    d.contextWidth = "project";
    d.contextBudgetTokens = 8000;
    d.reviewRequired = true;
    d.confidence = 0.7f;
    d.reasoning = "Project context";
    d.agentRole = "generator";
    return d;
}

int main() {
    int passed = 0;

    // Test 1: bundle contains core fields
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        assert(!b.taskDescription.empty());
        assert(!b.skeletonCode.empty());
        assert(!b.contextCode.empty());
        assert(b.tokenBudget == 8000);
        std::cout << "Test 1 PASSED: bundle contains required fields\n";
        passed++;
    }

    // Test 2: bundleToPrompt produces structured readable output
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        std::string prompt = bundleToPrompt(b);
        assert(prompt.find("Task:") != std::string::npos);
        assert(prompt.find("Constraints:") != std::string::npos);
        assert(prompt.find("Output Format:") != std::string::npos);
        std::cout << "Test 2 PASSED: prompt rendering structured\n";
        passed++;
    }

    // Test 3: rejection history included in previous attempts
    {
        WorkItem item = makeItem();
        RejectionAttempt a;
        a.workerType = "slm";
        a.result.generatedCode = "return tmp";
        a.feedback = "missed edge case";
        item.rejectionHistory.push_back(a);
        auto b = buildBundle(item, makeContext(), makeDecision());
        assert(b.previousAttempts.size() == 1);
        assert(b.previousAttempts[0].feedback == "missed edge case");
        std::cout << "Test 3 PASSED: rejection history carried into bundle\n";
        passed++;
    }

    // Test 4: token estimate is positive and size-correlated
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        int estimate = estimateBundleTokens(b);
        assert(estimate > 0);
        assert(std::abs(estimate - (int)(b.toJson().dump().size() / 4)) <= 1);
        std::cout << "Test 4 PASSED: token estimate reasonable\n";
        passed++;
    }

    // Test 5: budget value preserved from routing decision
    {
        RoutingDecision d = makeDecision();
        d.contextBudgetTokens = 1234;
        auto b = buildBundle(makeItem(), makeContext(), d);
        assert(b.tokenBudget == 1234);
        std::cout << "Test 5 PASSED: budget respected from routing decision\n";
        passed++;
    }

    // Test 6: annotation constraints included
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        assert(!b.constraints.empty());
        bool hasContract = false;
        for (const auto& c : b.constraints) {
            if (c.find("Contract") != std::string::npos) hasContract = true;
        }
        assert(hasContract);
        std::cout << "Test 6 PASSED: constraints include annotations\n";
        passed++;
    }

    // Test 7: intent comes from worker context
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        assert(b.intent == "Build normalized result map");
        std::cout << "Test 7 PASSED: intent carried from context\n";
        passed++;
    }

    // Test 8: empty context fields handled gracefully
    {
        WorkItem item = makeItem();
        WorkerContext ctx;
        RoutingDecision d = makeDecision();
        auto b = buildBundle(item, ctx, d);
        std::string prompt = bundleToPrompt(b);
        assert(!prompt.empty());
        assert(b.constraints.size() == 1); // default "none"
        std::cout << "Test 8 PASSED: empty fields handled gracefully\n";
        passed++;
    }

    // Test 9: bundle JSON roundtrip
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        auto round = ContextBundle::fromJson(bundleToJson(b));
        assert(round.taskDescription == b.taskDescription);
        assert(round.tokenBudget == b.tokenBudget);
        assert(round.outputFormat == b.outputFormat);
        std::cout << "Test 9 PASSED: JSON roundtrip works\n";
        passed++;
    }

    // Test 10: prompt format remains model-agnostic
    {
        auto b = buildBundle(makeItem(), makeContext(), makeDecision());
        std::string prompt = bundleToPrompt(b);
        assert(prompt.find("Claude") == std::string::npos);
        assert(prompt.find("OpenAI") == std::string::npos);
        std::cout << "Test 10 PASSED: prompt format model-agnostic\n";
        passed++;
    }

    // Test 11: deterministic/template defaults to code-only output format
    {
        RoutingDecision d = makeDecision();
        d.workerType = "template";
        auto b = buildBundle(makeItem(), makeContext(), d);
        assert(b.outputFormat == "code-only");
        std::cout << "Test 11 PASSED: code-only format for deterministic/template\n";
        passed++;
    }

    // Test 12: previous attempts preserve insertion order
    {
        WorkItem item = makeItem();
        RejectionAttempt a1; a1.workerType = "slm"; a1.feedback = "first";
        RejectionAttempt a2; a2.workerType = "llm"; a2.feedback = "second";
        item.rejectionHistory.push_back(a1);
        item.rejectionHistory.push_back(a2);
        auto b = buildBundle(item, makeContext(), makeDecision());
        assert(b.previousAttempts.size() == 2);
        assert(b.previousAttempts[0].feedback == "first");
        assert(b.previousAttempts[1].feedback == "second");
        std::cout << "Test 12 PASSED: previous-attempt order preserved\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
