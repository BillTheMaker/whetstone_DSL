// Step 245 TDD Test: HeadlessEditorState — agent API without ImGui/SDL
#include "HeadlessEditorState.h"
#include <iostream>
#include <cassert>

static void expect(bool cond, const std::string& name,
                   int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1)
                  << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1)
                  << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    // 1. Construct headless state without crash
    HeadlessEditorState state;
    state.workspaceRoot = "/tmp/test";
    state.defaultLanguage = "python";
    expect(state.active() == nullptr, "no active buffer initially",
           passed, failed);

    // 2. Open a buffer with Python source
    std::string src = "def hello(name):\n    return 'hi ' + name\n";
    auto* buf = state.openBuffer("test.py", src, "python");
    expect(buf != nullptr, "buffer opened", passed, failed);
    expect(state.active() == buf, "buffer is active", passed, failed);
    expect(state.isStructured(), "buffer is structured", passed, failed);
    expect(state.activeAST() != nullptr, "AST parsed", passed, failed);

    // 3. Parse source via RPC
    json parseReq = {
        {"jsonrpc", "2.0"}, {"id", 1}, {"method", "parseSource"},
        {"params", {{"source", "x = 42"}, {"language", "python"}}}
    };
    // Set role to Refactor so parseSource is permitted
    state.setAgentRole("test-agent", AgentRole::Refactor);
    json parseRes = state.processAgentRequest(parseReq, "test-agent");
    expect(parseRes.contains("result"), "parseSource returns result",
           passed, failed);
    expect(parseRes["result"].contains("ast"), "parseSource has AST",
           passed, failed);

    // 4. Get AST via RPC
    json getReq = {
        {"jsonrpc", "2.0"}, {"id", 2}, {"method", "getAST"}
    };
    json getRes = state.processAgentRequest(getReq, "test-agent");
    expect(getRes.contains("result"), "getAST returns result",
           passed, failed);
    expect(getRes["result"].contains("ast"), "getAST has ast field",
           passed, failed);

    // 5. Run pipeline via RPC
    json pipeReq = {
        {"jsonrpc", "2.0"}, {"id", 3}, {"method", "runPipeline"},
        {"params", {{"source", "def foo(): pass"},
                    {"sourceLanguage", "python"},
                    {"targetLanguage", "cpp"}}}
    };
    json pipeRes = state.processAgentRequest(pipeReq, "test-agent");
    expect(pipeRes.contains("result"), "runPipeline returns result",
           passed, failed);
    expect(pipeRes["result"].contains("generatedCode"),
           "pipeline generates code", passed, failed);

    // 6. Mutation via RPC — rename the module
    Module* ast = state.activeAST();
    std::string rootId = ast ? ast->id : "";
    expect(!rootId.empty(), "root node has id", passed, failed);

    json mutReq = {
        {"jsonrpc", "2.0"}, {"id", 4}, {"method", "applyMutation"},
        {"params", {{"type", "setProperty"}, {"nodeId", rootId},
                    {"property", "name"}, {"value", "renamed"}}}
    };
    json mutRes = state.processAgentRequest(mutReq, "test-agent");
    expect(mutRes.contains("result"), "mutation returns result",
           passed, failed);
    expect(mutRes["result"].value("success", false),
           "mutation succeeded", passed, failed);

    // 7. Permission check — Linter cannot mutate
    state.setAgentRole("linter-agent", AgentRole::Linter);
    json mutReq2 = {
        {"jsonrpc", "2.0"}, {"id", 5}, {"method", "applyMutation"},
        {"params", {{"type", "setProperty"}, {"nodeId", rootId},
                    {"property", "name"}, {"value", "blocked"}}}
    };
    json mutRes2 = state.processAgentRequest(mutReq2, "linter-agent");
    expect(mutRes2.contains("error"), "linter mutation blocked",
           passed, failed);

    // 8. Ping
    json pingReq = {{"jsonrpc", "2.0"}, {"id", 6}, {"method", "ping"}};
    json pingRes = state.processAgentRequest(pingReq, "test-agent");
    expect(pingRes.contains("result"), "ping returns result",
           passed, failed);

    // 9. getSessionInfo
    json infoReq = {
        {"jsonrpc", "2.0"}, {"id", 7}, {"method", "getSessionInfo"}
    };
    json infoRes = state.processAgentRequest(infoReq, "test-agent");
    expect(infoRes.contains("result"), "getSessionInfo returns result",
           passed, failed);
    expect(infoRes["result"].value("mode", "") == "headless",
           "mode is headless", passed, failed);

    // 10. Buffer management — close and reopen
    state.closeBuffer("test.py");
    expect(state.active() == nullptr, "buffer closed", passed, failed);
    expect(!state.isStructured(), "no structured after close",
           passed, failed);

    std::cout << "\n=== Step 245 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
