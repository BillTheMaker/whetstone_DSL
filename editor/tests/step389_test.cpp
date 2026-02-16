// Step 389: Routing Rules Engine (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "RoutingRules.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& name = "doWork",
                         const std::string& workerType = "",
                         const std::string& contextWidth = "local",
                         const std::string& priority = "medium") {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = name;
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.workerType = workerType;
    item.contextWidth = contextWidth;
    item.priority = priority;
    item.status = WI_PENDING;
    item.createdAt = workItemTimestamp();
    return item;
}

int main() {
    int passed = 0;

    // Test 1: First matching rule wins
    {
        RulesEngine engine;
        RoutingRule r1;
        r1.name = "rule-A";
        r1.priority = 10;
        r1.conditions.push_back({"contextWidth", "", "==", "local"});
        r1.action.workerType = "template";

        RoutingRule r2;
        r2.name = "rule-B";
        r2.priority = 20;
        r2.conditions.push_back({"contextWidth", "", "==", "local"});
        r2.action.workerType = "llm";

        engine.addRule(r2);
        engine.addRule(r1); // added second but lower priority number = higher priority

        auto item = makeItem("t1");
        auto decision = engine.evaluate(item);
        assert(decision.workerType == "template");
        assert(decision.reasoning.find("rule-A") != std::string::npos);
        std::cout << "PASS: test 1 — first matching rule wins\n";
        passed++;
    }

    // Test 2: Conditions AND together
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "local-function";
        r.priority = 10;
        r.conditions.push_back({"contextWidth", "", "==", "local"});
        r.conditions.push_back({"nodeType", "", "==", "Function"});
        r.action.workerType = "template";
        engine.addRule(r);

        // Matches both conditions
        auto item1 = makeItem("t1");
        assert(engine.evaluate(item1).workerType == "template");

        // Fails nodeType condition
        auto item2 = makeItem("t2");
        item2.nodeType = "Class";
        auto dec2 = engine.evaluate(item2);
        assert(dec2.workerType == "llm"); // falls through to default
        std::cout << "PASS: test 2 — conditions AND together\n";
        passed++;
    }

    // Test 3: Annotation condition matches
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "explicit-human";
        r.priority = 10;
        r.conditions.push_back({"annotation", "workerType", "==", "human"});
        r.action.workerType = "human";
        r.action.reviewRequired = true;
        engine.addRule(r);

        auto item = makeItem("t1", "doWork", "human");
        auto dec = engine.evaluate(item);
        assert(dec.workerType == "human");
        assert(dec.reviewRequired == true);
        std::cout << "PASS: test 3 — annotation condition matches\n";
        passed++;
    }

    // Test 4: Complexity threshold (via priority proxy)
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "high-priority-llm";
        r.priority = 10;
        // priority critical=0, high=1 → complexity < 2 means critical or high
        r.conditions.push_back({"complexity", "", "<", "2"});
        r.action.workerType = "llm";
        r.action.budgetMultiplier = 2.0f;
        engine.addRule(r);

        auto highItem = makeItem("t1", "doWork", "", "local", "high");
        auto dec1 = engine.evaluate(highItem);
        assert(dec1.workerType == "llm");

        auto lowItem = makeItem("t2", "doWork", "", "local", "low");
        auto dec2 = engine.evaluate(lowItem);
        assert(dec2.workerType == "llm"); // falls through to default (also llm)
        assert(dec2.reasoning.find("Default") != std::string::npos); // but via default
        std::cout << "PASS: test 4 — complexity threshold\n";
        passed++;
    }

    // Test 5: Pattern matching (getter-setter)
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "getter-template";
        r.priority = 10;
        r.conditions.push_back({"pattern", "", "==", "getter-setter"});
        r.action.workerType = "template";
        engine.addRule(r);

        auto getter = makeItem("t1", "getName");
        assert(engine.evaluate(getter).workerType == "template");

        auto setter = makeItem("t2", "setValue");
        assert(engine.evaluate(setter).workerType == "template");

        auto normal = makeItem("t3", "processData");
        assert(engine.evaluate(normal).workerType != "template");
        std::cout << "PASS: test 5 — pattern matching\n";
        passed++;
    }

    // Test 6: Default rules sensible
    {
        auto defaults = RulesEngine::getDefaultRules();
        assert(defaults.size() >= 5);

        // Verify known rules exist
        bool hasGetterRule = false, hasProjectRule = false;
        for (const auto& r : defaults) {
            if (r.name == "getter-setter-template") hasGetterRule = true;
            if (r.name == "project-context-llm") hasProjectRule = true;
        }
        assert(hasGetterRule);
        assert(hasProjectRule);
        std::cout << "PASS: test 6 — default rules sensible\n";
        passed++;
    }

    // Test 7: Custom rules override defaults
    {
        RulesEngine engine;
        // Load defaults first
        auto defaults = RulesEngine::getDefaultRules();
        for (const auto& r : defaults) engine.addRule(r);

        // Add custom rule with highest priority
        RoutingRule custom;
        custom.name = "force-deterministic";
        custom.priority = 1; // highest
        custom.conditions.push_back({"contextWidth", "", "==", "local"});
        custom.action.workerType = "deterministic";
        engine.addRule(custom);

        auto item = makeItem("t1", "doWork", "", "local");
        auto dec = engine.evaluate(item);
        assert(dec.workerType == "deterministic");
        assert(dec.reasoning.find("force-deterministic") != std::string::npos);
        std::cout << "PASS: test 7 — custom rules override defaults\n";
        passed++;
    }

    // Test 8: No match falls through to default
    {
        RulesEngine engine;
        // Add a rule that won't match
        RoutingRule r;
        r.name = "specific";
        r.priority = 10;
        r.conditions.push_back({"contextWidth", "", "==", "cross-project"});
        r.action.workerType = "human";
        engine.addRule(r);

        auto item = makeItem("t1", "doWork", "", "local");
        auto dec = engine.evaluate(item);
        assert(dec.workerType == "llm"); // default
        assert(dec.reasoning.find("Default") != std::string::npos);
        std::cout << "PASS: test 8 — no match falls through to default\n";
        passed++;
    }

    // Test 9: Rule priority ordering
    {
        RulesEngine engine;
        RoutingRule low;
        low.name = "low-pri";
        low.priority = 100;
        low.action.workerType = "slm";
        engine.addRule(low);

        RoutingRule high;
        high.name = "high-pri";
        high.priority = 1;
        high.action.workerType = "template";
        engine.addRule(high);

        // Both have empty conditions (match anything), high-pri should win
        auto item = makeItem("t1");
        auto dec = engine.evaluate(item);
        assert(dec.workerType == "template");
        assert(engine.getRules()[0].name == "high-pri");
        std::cout << "PASS: test 9 — rule priority ordering\n";
        passed++;
    }

    // Test 10: Rules persist to JSON
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "test-rule";
        r.priority = 42;
        r.conditions.push_back({"pattern", "", "==", "getter-setter"});
        r.action.workerType = "template";
        r.action.budgetMultiplier = 1.5f;
        engine.addRule(r);

        json saved = engine.saveRules();
        assert(saved.is_array());
        assert(saved.size() == 1);
        assert(saved[0]["name"] == "test-rule");
        assert(saved[0]["priority"] == 42);

        RulesEngine loaded;
        loaded.loadRules(saved);
        assert(loaded.ruleCount() == 1);
        assert(loaded.getRules()[0].name == "test-rule");
        assert(loaded.getRules()[0].action.budgetMultiplier == 1.5f);
        std::cout << "PASS: test 10 — rules persist to JSON\n";
        passed++;
    }

    // Test 11: Budget multiplier applied
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "big-budget";
        r.priority = 10;
        r.conditions.push_back({"contextWidth", "", "==", "local"});
        r.action.workerType = "llm";
        r.action.budgetMultiplier = 3.0f;
        engine.addRule(r);

        auto item = makeItem("t1");
        auto dec = engine.evaluate(item);
        assert(dec.contextBudgetTokens == 500 * 3); // local=500 * 3.0
        std::cout << "PASS: test 11 — budget multiplier applied\n";
        passed++;
    }

    // Test 12: Rejection count escalation
    {
        RulesEngine engine;
        RoutingRule r;
        r.name = "escalate-on-reject";
        r.priority = 10;
        r.conditions.push_back({"rejectionCount", "", ">=", "2"});
        r.action.workerType = "human";
        r.action.reviewRequired = true;
        engine.addRule(r);

        auto item = makeItem("t1");
        // No rejections — rule doesn't match
        auto dec1 = engine.evaluate(item);
        assert(dec1.workerType != "human");

        // Add 2 rejections
        RejectionAttempt a1; a1.workerType = "slm"; a1.feedback = "bad";
        RejectionAttempt a2; a2.workerType = "llm"; a2.feedback = "still bad";
        item.rejectionHistory.push_back(a1);
        item.rejectionHistory.push_back(a2);

        auto dec2 = engine.evaluate(item);
        assert(dec2.workerType == "human");
        assert(dec2.reviewRequired == true);
        std::cout << "PASS: test 12 — rejection count escalation\n";
        passed++;
    }

    std::cout << "\nStep 389 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
