// Step 264 TDD Test: Undo/Redo for Headless Mode
//
// Tests undo and redo RPC methods that use the Orchestrator's snapshot
// journal to restore previous/next AST + editBuf states after mutations.
// Snapshots are recorded automatically before each mutation.
#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <iostream>
#include <string>

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

int main() {
    int passed = 0;
    int failed = 0;

    std::string src =
        "def greet(name):\n"
        "    return \"hello \" + name\n";

    // ---------------------------------------------------------------
    //  Test 1: undo with no history returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        json resp = rpc(state, "s1", "undo");
        bool hasError = resp.contains("error");
        expect(hasError,
               "undo with no mutation history returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: mutate → undo restores previous AST
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        // Rename greet → salute
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});

        // Verify salute exists
        bool hasSalute = false;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "salute")
                hasSalute = true;
        }

        // Undo — should restore greet
        json resp = rpc(state, "s1", "undo");
        bool undoOk = resp.contains("result") &&
                      resp["result"].value("success", false);

        bool hasGreet = false;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "greet")
                hasGreet = true;
        }

        expect(hasSalute && undoOk && hasGreet,
               "rename→undo restores original function name",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: undo restores editBuf (regenerated code)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string beforeCode = state.activeBuffer->editBuf;

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});

        std::string afterCode = state.activeBuffer->editBuf;

        rpc(state, "s1", "undo");

        std::string undoneCode = state.activeBuffer->editBuf;

        bool codeChanged = afterCode != beforeCode;
        bool codeRestored = undoneCode.find("greet") != std::string::npos;
        expect(codeChanged && codeRestored,
               "undo restores editBuf with original code",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: undo → redo restores the mutation
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});

        rpc(state, "s1", "undo");

        // Now redo
        json resp = rpc(state, "s1", "redo");
        bool redoOk = resp.contains("result") &&
                      resp["result"].value("success", false);

        bool hasSalute = false;
        if (state.activeAST()) {
            for (auto* c : state.activeAST()->allChildren()) {
                if (c->conceptType == "Function" &&
                    getNodeName(c) == "salute")
                    hasSalute = true;
            }
        }

        expect(redoOk && hasSalute,
               "undo→redo restores the renamed function",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: redo with nothing to redo returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        json resp = rpc(state, "s1", "redo");
        bool hasError = resp.contains("error");
        expect(hasError,
               "redo with nothing to redo returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: undo/redo report depths
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        // Two mutations
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "salute"}, {"newName", "welcome"}});

        // Undo once
        json resp1 = rpc(state, "s1", "undo");
        int undoDepth1 = resp1["result"].value("undoDepth", -1);
        int redoDepth1 = resp1["result"].value("redoDepth", -1);

        // Undo again
        json resp2 = rpc(state, "s1", "undo");
        int undoDepth2 = resp2["result"].value("undoDepth", -1);
        int redoDepth2 = resp2["result"].value("redoDepth", -1);

        expect(undoDepth1 == 1 && redoDepth1 == 1 &&
               undoDepth2 == 0 && redoDepth2 == 2,
               "Depths: after 1st undo=" + std::to_string(undoDepth1) +
               "/" + std::to_string(redoDepth1) + ", after 2nd undo=" +
               std::to_string(undoDepth2) + "/" +
               std::to_string(redoDepth2),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Linter cannot undo/redo
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("linter", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "t.py"}, {"content", src}});

        rpc(state, "admin", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});

        json undoResp = rpc(state, "linter", "undo");
        json redoResp = rpc(state, "linter", "redo");
        bool undoDenied = undoResp.contains("error");
        bool redoDenied = redoResp.contains("error");

        expect(undoDenied && redoDenied,
               "Linter cannot undo or redo",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: multiple mutations → multiple undos
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        // Three renames
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "a"}});
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "a"}, {"newName", "b"}});
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "b"}, {"newName", "c"}});

        // Function should be "c" now
        bool isC = false;
        for (auto* ch : state.activeAST()->allChildren()) {
            if (ch->conceptType == "Function" && getNodeName(ch) == "c")
                isC = true;
        }

        // Undo 3 times — back to greet
        rpc(state, "s1", "undo");
        rpc(state, "s1", "undo");
        rpc(state, "s1", "undo");

        bool isGreet = false;
        for (auto* ch : state.activeAST()->allChildren()) {
            if (ch->conceptType == "Function" &&
                getNodeName(ch) == "greet")
                isGreet = true;
        }

        expect(isC && isGreet,
               "3 mutations → 3 undos restores original",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: new mutation after undo clears redo stack
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "a"}});
        rpc(state, "s1", "undo");

        // Redo should be available
        int redoBefore = state.activeBuffer->undoStack.redoDepth();

        // New mutation should clear redo
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "z"}});
        int redoAfter = state.activeBuffer->undoStack.redoDepth();

        // Redo should now be empty
        json redoResp = rpc(state, "s1", "redo");
        bool redoFails = redoResp.contains("error");

        expect(redoBefore == 1 && redoAfter == 0 && redoFails,
               "New mutation after undo clears redo stack",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: undo sets modified flag
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});
        state.activeBuffer->modified = false;

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});
        state.activeBuffer->modified = false; // simulate save

        rpc(state, "s1", "undo");
        bool modAfterUndo = state.activeBuffer->modified;

        expect(modAfterUndo,
               "undo sets modified flag on buffer",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tools registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool foundUndo = false;
        bool foundRedo = false;
        for (const auto& t : mcp.getTools()) {
            if (t.name == "whetstone_undo") foundUndo = true;
            if (t.name == "whetstone_redo") foundRedo = true;
        }
        expect(foundUndo && foundRedo,
               "whetstone_undo and whetstone_redo MCP tools registered",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: undo/redo with applyMutation (not just rename)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        // Get function node ID
        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Mutate via setProperty
        rpc(state, "s1", "applyMutation",
            {{"type", "setProperty"}, {"nodeId", fnId},
             {"property", "name"}, {"value", "howdy"}});

        bool isHowdy = false;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "howdy")
                isHowdy = true;
        }

        // Undo
        rpc(state, "s1", "undo");

        bool isGreetAgain = false;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "greet")
                isGreetAgain = true;
        }

        expect(isHowdy && isGreetAgain,
               "applyMutation→undo restores original name",
               passed, failed);
    }

    std::cout << "\n=== Step 264 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
