// Step 157 TDD Test: Agent roles + transform provenance
#include "AgentPermissionPolicy.h"
#include "IncrementalOptimizer.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    expect(!AgentPermissionPolicy::canInvoke(AgentRole::Linter, "applyMutation"),
           "linter cannot mutate", passed, failed);
    expect(AgentPermissionPolicy::canInvoke(AgentRole::Refactor, "applyMutation"),
           "refactor can mutate", passed, failed);
    expect(AgentPermissionPolicy::canInvoke(AgentRole::Generator, "generateCode"),
           "generator can generate code", passed, failed);

    Module mod;
    mod.id = "mod1";
    mod.name = "Test";
    mod.targetLanguage = "cpp";
    auto* fn = new Function();
    fn->id = "fn1";
    fn->name = "work";
    mod.addChild("functions", fn);

    IncrementalOptimizer inc;
    inc.setRoot(&mod);
    std::string tid = inc.recordExternalTransform(
        "agent-mutation:setProperty",
        {fn->id},
        "agent:test");

    auto history = inc.getTransformHistory();
    bool found = false;
    for (const auto& h : history) {
        if (h.transformId == tid) {
            found = true;
            expect(h.actor == "agent:test", "actor recorded", passed, failed);
            expect(!h.reversible, "external transform not reversible", passed, failed);
        }
    }
    expect(found, "external transform in history", passed, failed);
    expect(inc.getProvenance(fn->id) == tid, "provenance recorded", passed, failed);
    expect(!inc.undoTransform(tid), "external transform cannot undo", passed, failed);

    std::cout << "\n=== Step 157 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
