// Step 265 TDD Test: Phase 9e Persistence & Undo/Redo Integration Tests
//
// End-to-end integration tests exercising save + undo/redo workflows:
// save after mutation, undo→save (reverted code on disk), redo→save,
// saveAllBuffers with undo state, MCP tool count, and full workflow.
//
// Follows the Phase closer pattern from Steps 249, 253, 257, 262.
#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

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

static json rpc(HeadlessEditorState& state, const std::string& session,
                 const std::string& method,
                 json params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", method}, {"params", params}};
    return state.processAgentRequest(request, session);
}

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    return std::string(std::istreambuf_iterator<char>(in),
                       std::istreambuf_iterator<char>());
}

int main() {
    int passed = 0;
    int failed = 0;

    // Temp workspace for file I/O
    auto tmpDir = std::filesystem::temp_directory_path() /
                  "whetstone_step265_test";
    std::filesystem::create_directories(tmpDir);

    std::string src =
        "def greet(name):\n"
        "    return \"hello \" + name\n";

    // ---------------------------------------------------------------
    //  Test 1: mutate → save writes mutated code to disk
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "test1.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});

        rpc(state, "s1", "saveBuffer",
            {{"path", "test1.py"}});

        std::string saved = readFile((tmpDir / "test1.py").string());
        bool hasSalute = saved.find("salute") != std::string::npos;
        bool noGreet = saved.find("greet") == std::string::npos;

        expect(hasSalute && noGreet,
               "mutate→save writes mutated code to disk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: mutate → undo → save writes original code to disk
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "test2.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});
        rpc(state, "s1", "undo");
        rpc(state, "s1", "saveBuffer",
            {{"path", "test2.py"}});

        std::string saved = readFile((tmpDir / "test2.py").string());
        bool hasGreet = saved.find("greet") != std::string::npos;
        bool noSalute = saved.find("salute") == std::string::npos;

        expect(hasGreet && noSalute,
               "mutate→undo→save writes original code to disk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: undo → redo → save writes mutated code back
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "test3.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});
        rpc(state, "s1", "undo");
        rpc(state, "s1", "redo");
        rpc(state, "s1", "saveBuffer",
            {{"path", "test3.py"}});

        std::string saved = readFile((tmpDir / "test3.py").string());
        bool hasSalute = saved.find("salute") != std::string::npos;

        expect(hasSalute,
               "undo→redo→save writes mutated code back to disk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: saveAllBuffers after mixed undo states
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "a.py"}, {"content", "def alpha():\n    pass\n"}});
        rpc(state, "s1", "openFile",
            {{"path", "b.py"}, {"content", "def beta():\n    pass\n"}});

        // Mutate both
        rpc(state, "s1", "setActiveBuffer", {{"path", "a.py"}});
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "alpha"}, {"newName", "alpha2"}});

        rpc(state, "s1", "setActiveBuffer", {{"path", "b.py"}});
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "beta"}, {"newName", "beta2"}});

        // Undo on b.py only
        rpc(state, "s1", "undo");

        // Save all
        json resp = rpc(state, "s1", "saveAllBuffers");
        int savedCount = resp["result"].value("savedCount", 0);

        std::string savedA = readFile((tmpDir / "a.py").string());
        std::string savedB = readFile((tmpDir / "b.py").string());

        bool aHasAlpha2 = savedA.find("alpha2") != std::string::npos;
        bool bHasBeta = savedB.find("beta") != std::string::npos &&
                        savedB.find("beta2") == std::string::npos;

        expect(savedCount == 2 && aHasAlpha2 && bHasBeta,
               "saveAllBuffers: a.py mutated, b.py undone",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: multiple undos + save + redo checks consistency
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "test5.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "a"}});
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "a"}, {"newName", "b"}});

        // Undo twice back to original
        rpc(state, "s1", "undo");
        rpc(state, "s1", "undo");

        rpc(state, "s1", "saveBuffer", {{"path", "test5.py"}});
        std::string saved1 = readFile((tmpDir / "test5.py").string());
        bool hasGreet = saved1.find("greet") != std::string::npos;

        // Redo once → "a"
        rpc(state, "s1", "redo");
        rpc(state, "s1", "saveBuffer", {{"path", "test5.py"}});
        std::string saved2 = readFile((tmpDir / "test5.py").string());
        bool hasA = saved2.find("def a(") != std::string::npos;

        // Redo again → "b"
        rpc(state, "s1", "redo");
        rpc(state, "s1", "saveBuffer", {{"path", "test5.py"}});
        std::string saved3 = readFile((tmpDir / "test5.py").string());
        bool hasB = saved3.find("def b(") != std::string::npos;

        expect(hasGreet && hasA && hasB,
               "undo/redo/save cycle: greet→a→b all consistent",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: MCP tools count includes save and undo/redo
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        auto tools = mcp.getTools();
        int count = (int)tools.size();

        bool hasSave = false, hasSaveAll = false;
        bool hasUndo = false, hasRedo = false;
        for (const auto& t : tools) {
            if (t.name == "whetstone_save_buffer") hasSave = true;
            if (t.name == "whetstone_save_all_buffers") hasSaveAll = true;
            if (t.name == "whetstone_undo") hasUndo = true;
            if (t.name == "whetstone_redo") hasRedo = true;
        }

        expect(count >= 34 && hasSave && hasSaveAll && hasUndo && hasRedo,
               "MCP has " + std::to_string(count) + " tools (>=34) with save+undo+redo",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: undo after save preserves on-disk version
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "test7.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});
        rpc(state, "s1", "saveBuffer", {{"path", "test7.py"}});

        // Disk has "salute" now
        std::string onDisk = readFile((tmpDir / "test7.py").string());
        bool diskHasSalute = onDisk.find("salute") != std::string::npos;

        // Undo in memory
        rpc(state, "s1", "undo");

        // Buffer has "greet" but disk still has "salute"
        bool bufHasGreet =
            state.activeBuffer->editBuf.find("greet") != std::string::npos;
        std::string stillOnDisk = readFile((tmpDir / "test7.py").string());
        bool diskStillSalute = stillOnDisk.find("salute") != std::string::npos;

        expect(diskHasSalute && bufHasGreet && diskStillSalute,
               "undo in memory doesn't affect saved file on disk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: full workflow: open → mutate → diagnose → undo → redo → save
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        // 1. Open
        rpc(state, "s1", "openFile",
            {{"path", "workflow.py"}, {"content", src}});
        bool opened = state.activeBuffer != nullptr;

        // 2. Get AST
        json astResp = rpc(state, "s1", "getAST");
        bool gotAST = astResp.contains("result") &&
                      astResp["result"].contains("ast");

        // 3. Mutate (rename)
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "wave"}});
        bool hasWave = state.activeBuffer->editBuf.find("wave") !=
                       std::string::npos;

        // 4. Get diagnostics
        json diagResp = rpc(state, "s1", "getDiagnostics");
        bool gotDiags = diagResp.contains("result");

        // 5. Undo
        json undoResp = rpc(state, "s1", "undo");
        bool undoOk = undoResp.contains("result") &&
                      undoResp["result"].value("success", false);
        bool backToGreet = state.activeBuffer->editBuf.find("greet") !=
                           std::string::npos;

        // 6. Redo
        json redoResp = rpc(state, "s1", "redo");
        bool redoOk = redoResp.contains("result") &&
                      redoResp["result"].value("success", false);
        bool backToWave = state.activeBuffer->editBuf.find("wave") !=
                          std::string::npos;

        // 7. Save
        json saveResp = rpc(state, "s1", "saveBuffer",
                            {{"path", "workflow.py"}});
        bool saveOk = saveResp.contains("result") &&
                      saveResp["result"].value("success", false);

        std::string onDisk = readFile((tmpDir / "workflow.py").string());
        bool diskHasWave = onDisk.find("wave") != std::string::npos;

        expect(opened && gotAST && hasWave && gotDiags &&
               undoOk && backToGreet && redoOk && backToWave &&
               saveOk && diskHasWave,
               "Full workflow: open→mutate→diagnose→undo→redo→save",
               passed, failed);
    }

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "\n=== Step 265 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
