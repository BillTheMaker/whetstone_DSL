// Step 125 Integration Test: EditorState agent mutation flow
#include "EditorUtils.h"
#include "imgui.h"
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    EditorState state;
    auto buf = std::make_unique<BufferState>();
    buf->path = "(untitled)";
    buf->language = "python";
    buf->bufferMode = BufferManager::BufferMode::Structured;
    auto mod = std::make_unique<Module>("root", "OldName", "python");
    buf->sync.setAST(std::move(mod));
    buf->orchestrator.setAST(cloneModule(buf->sync.getAST()));
    buf->orchestratorDirty = false;
    state.bufferStates[buf->path] = std::move(buf);
    state.activeBuffer = state.bufferStates["(untitled)"].get();

    state.agentMutationPermissions["agent_1"] = false;
    json mutReq = {
        {"jsonrpc","2.0"},
        {"id",1},
        {"method","applyMutation"},
        {"params", {{"type","setProperty"}, {"nodeId","root"}, {"property","name"}, {"value","NewName"}}}
    };
    json mutRes = state.processAgentRequest(mutReq, "agent_1");
    expect(mutRes.contains("error"), "mutation blocked", passed, failed);

    state.agentMutationPermissions["agent_1"] = true;
    json mutRes2 = state.processAgentRequest(mutReq, "agent_1");
    expect(mutRes2.contains("result"), "mutation allowed", passed, failed);
    Module* astAfter = state.activeAST();
    expect(astAfter && astAfter->name == "NewName", "mutation applied", passed, failed);

    std::cout << "\n=== Step 125 Integration Results: " << passed << " passed, " << failed << " failed ===\n";
    ImGui::DestroyContext();
    return failed == 0 ? 0 : 1;
}
