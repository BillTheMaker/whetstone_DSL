// Step 159 TDD Test: Agent registry
#include "AgentRegistry.h"
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

    AgentRegistry reg;
    AgentDescriptor agent;
    agent.name = "TestAgent";
    agent.description = "Example agent";
    agent.endpoint = "ws://localhost:9001";
    agent.capabilities = {"lint", "refactor"};
    agent.requiredPermissions = {"getAST", "applyMutation"};
    agent.installed = true;
    reg.upsert(agent);

    expect(reg.find("TestAgent") != nullptr, "agent inserted", passed, failed);
    expect(reg.list().size() == 1, "registry size", passed, failed);

    json j = reg.toJson();
    AgentRegistry reg2;
    std::string error;
    expect(reg2.loadFromJson(j, error), "load from json", passed, failed);
    expect(reg2.list().size() == 1, "loaded size", passed, failed);
    expect(reg2.list()[0].installed, "installed flag preserved", passed, failed);

    AgentRegistry reg3;
    expect(!reg3.loadFromJsonString("not json", error), "invalid json rejected", passed, failed);

    std::cout << "\n=== Step 159 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
