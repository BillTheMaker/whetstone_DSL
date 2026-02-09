// Step 120 TDD Test: Orchestrator snapshot undo/redo
#include "Orchestrator.h"
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

    Orchestrator orch;
    Module a("m1", "A", "python");
    Module b("m2", "B", "python");

    orch.recordSnapshot("one", &a);
    orch.recordSnapshot("two", &b);

    std::string text;
    std::unique_ptr<Module> ast;
    bool undoOk = orch.undoSnapshot(text, ast);
    expect(undoOk && text == "two" && ast && ast->name == "B", "undo snapshot", passed, failed);

    bool undoOk2 = orch.undoSnapshot(text, ast);
    expect(undoOk2 && text == "one" && ast && ast->name == "A", "undo snapshot (second)", passed, failed);

    bool redoOk = orch.redoSnapshot(text, ast);
    expect(redoOk && text == "one" && ast && ast->name == "A", "redo snapshot (first)", passed, failed);

    bool redoOk2 = orch.redoSnapshot(text, ast);
    expect(redoOk2 && text == "two" && ast && ast->name == "B", "redo snapshot (second)", passed, failed);

    std::cout << "\n=== Step 120 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
