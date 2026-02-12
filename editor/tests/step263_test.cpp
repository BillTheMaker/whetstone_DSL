// Step 263 TDD Test: Save Buffer to Disk
//
// Tests saveBuffer (regenerate code from AST, write to disk, clear modified)
// and saveAllBuffers (batch save all modified buffers). Both respect the
// workspace root for path resolution.
#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

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

// Helper: read file content from disk
static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return "";
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    return content;
}

int main() {
    int passed = 0;
    int failed = 0;

    // Create a temp directory for save tests
    fs::path tmpDir = fs::temp_directory_path() / "whetstone_step263_test";
    fs::create_directories(tmpDir);

    std::string workspace = tmpDir.string();

    std::string srcA =
        "def greet(name):\n"
        "    return \"hello \" + name\n";

    std::string srcB =
        "def add(x, y):\n"
        "    return x + y\n";

    // ---------------------------------------------------------------
    //  Test 1: saveBuffer writes editBuf to disk
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "saveBuffer",
                          {{"path", "greet.py"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);
        int bytes = resp["result"].value("bytesWritten", 0);

        // Verify file on disk
        std::string onDisk = readFile((tmpDir / "greet.py").string());
        bool written = !onDisk.empty();

        expect(success && bytes > 0 && written,
               "saveBuffer writes file to disk (" +
               std::to_string(bytes) + " bytes)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: saveBuffer clears the modified flag
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "mod.py"}, {"content", srcA}});

        // Mark as modified (simulating a mutation)
        state.bufferStates["mod.py"]->modified = true;
        bool wasMod = state.bufferStates["mod.py"]->modified;

        rpc(state, "s1", "saveBuffer", {{"path", "mod.py"}});

        bool nowClean = !state.bufferStates["mod.py"]->modified;
        expect(wasMod && nowClean,
               "saveBuffer clears modified flag",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: saveBuffer returns correct response fields
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "fields.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "saveBuffer",
                          {{"path", "fields.py"}});
        bool hasPath = resp["result"].contains("path");
        bool hasBytes = resp["result"].contains("bytesWritten");
        bool hasSuccess = resp["result"].contains("success");
        expect(hasPath && hasBytes && hasSuccess,
               "Response has path, bytesWritten, success fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: saveAllBuffers saves all modified, skips unmodified
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "a.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "b.py"}, {"content", srcB}});
        rpc(state, "s1", "openFile",
            {{"path", "c.py"}, {"content", srcA}});

        // Mark only a.py and c.py as modified
        state.bufferStates["a.py"]->modified = true;
        state.bufferStates["c.py"]->modified = true;
        // b.py stays unmodified

        json resp = rpc(state, "s1", "saveAllBuffers");
        int savedCount = resp["result"].value("savedCount", 0);
        int skippedCount = resp["result"].value("skippedCount", 0);

        expect(savedCount == 2 && skippedCount == 1,
               "saveAllBuffers: saved=" + std::to_string(savedCount)
               + " skipped=" + std::to_string(skippedCount),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: saveBuffer after mutation — regenerated code on disk
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "rename_save.py"}, {"content", srcA}});

        // Rename the function
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "greet"}, {"newName", "salute"}});

        // Save to disk
        rpc(state, "s1", "saveBuffer",
            {{"path", "rename_save.py"}});

        // Read from disk and verify the rename
        std::string onDisk = readFile(
            (tmpDir / "rename_save.py").string());
        bool hasSalute = onDisk.find("salute") != std::string::npos;
        bool noGreet = onDisk.find("greet") == std::string::npos;

        expect(hasSalute && noGreet,
               "Saved file contains renamed function 'salute'",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: saveBuffer requires Refactor or Generator role
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("linter", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "perm.py"}, {"content", srcA}});

        // Linter cannot save
        json denied = rpc(state, "linter", "saveBuffer",
                            {{"path", "perm.py"}});
        bool linterDenied = denied.contains("error");

        // Refactor can save
        json allowed = rpc(state, "admin", "saveBuffer",
                             {{"path", "perm.py"}});
        bool refactorOk = allowed.contains("result");

        expect(linterDenied && refactorOk,
               "Linter denied, Refactor allowed for saveBuffer",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: saveBuffer on active buffer (no path param)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "active_save.py"}, {"content", srcA}});
        state.bufferStates["active_save.py"]->modified = true;

        // Save without specifying path — should save active buffer
        json resp = rpc(state, "s1", "saveBuffer");
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);

        // Verify file on disk
        std::string onDisk = readFile(
            (tmpDir / "active_save.py").string());
        bool written = !onDisk.empty();

        expect(success && written,
               "saveBuffer without path saves active buffer",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: saveBuffer error on unknown buffer
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "saveBuffer",
                          {{"path", "nonexistent.py"}});
        bool hasError = resp.contains("error");
        expect(hasError,
               "saveBuffer on unknown buffer returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: saveAllBuffers clears modified flags on all saved
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "x.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "y.py"}, {"content", srcB}});
        state.bufferStates["x.py"]->modified = true;
        state.bufferStates["y.py"]->modified = true;

        rpc(state, "s1", "saveAllBuffers");

        bool xClean = !state.bufferStates["x.py"]->modified;
        bool yClean = !state.bufferStates["y.py"]->modified;
        expect(xClean && yClean,
               "saveAllBuffers clears modified flags on all saved",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: saveAllBuffers returns saved file paths
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "p.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "q.py"}, {"content", srcB}});
        state.bufferStates["p.py"]->modified = true;
        state.bufferStates["q.py"]->modified = true;

        json resp = rpc(state, "s1", "saveAllBuffers");
        bool hasSaved = resp["result"].contains("saved") &&
                        resp["result"]["saved"].is_array();
        int savedSize = hasSaved ? (int)resp["result"]["saved"].size() : 0;
        expect(hasSaved && savedSize == 2,
               "saveAllBuffers returns saved file paths (" +
               std::to_string(savedSize) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tools registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool foundSave = false;
        bool foundSaveAll = false;
        for (const auto& t : mcp.getTools()) {
            if (t.name == "whetstone_save_buffer") foundSave = true;
            if (t.name == "whetstone_save_all_buffers")
                foundSaveAll = true;
        }
        expect(foundSave && foundSaveAll,
               "whetstone_save_buffer and whetstone_save_all_buffers "
               "MCP tools registered",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: saveBuffer with workspace path resolution
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = workspace;
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "sub/deep.py"}, {"content", srcA}});
        state.bufferStates["sub/deep.py"]->modified = true;

        json resp = rpc(state, "s1", "saveBuffer",
                          {{"path", "sub/deep.py"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);

        // Verify the subdirectory was created
        bool fileExists = fs::exists(tmpDir / "sub" / "deep.py");
        expect(success && fileExists,
               "saveBuffer creates parent dirs and writes file",
               passed, failed);
    }

    // Cleanup
    fs::remove_all(tmpDir);

    std::cout << "\n=== Step 263 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
