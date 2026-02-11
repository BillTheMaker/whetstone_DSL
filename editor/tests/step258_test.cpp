// Step 258 TDD Test: Project Model and Workspace Indexing
//
// Tests openFile/closeFile/listBuffers/setActiveBuffer RPC methods,
// workspace indexing, language detection, and MCP tool registration.
#include "HeadlessEditorState.h"
#include "CompactAST.h"
#include "MCPBridge.h"

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

// Create a temp workspace with some files for indexing tests
struct TempWorkspace {
    std::string root;
    TempWorkspace() {
        root = (fs::temp_directory_path() / "whetstone_test_ws_258").string();
        fs::create_directories(root);
        fs::create_directories(root + "/src");
        fs::create_directories(root + "/tests");
        writeFile("src/main.py",
            "def main():\n    print('hello')\n");
        writeFile("src/utils.py",
            "def helper(x):\n    return x * 2\n");
        writeFile("src/app.cpp",
            "#include <iostream>\nint main() { return 0; }\n");
        writeFile("tests/test_main.py",
            "def test_main():\n    assert True\n");
        writeFile("README.md", "# Test Project\n");
    }
    ~TempWorkspace() {
        std::error_code ec;
        fs::remove_all(root, ec);
    }
    void writeFile(const std::string& rel, const std::string& content) {
        std::string full = root + "/" + rel;
        fs::create_directories(fs::path(full).parent_path());
        std::ofstream f(full);
        f << content;
    }
};

int main() {
    int passed = 0;
    int failed = 0;

    std::string pySrc =
        "def greet(name):\n    return 'hello ' + name\n";
    std::string pySrc2 =
        "def compute(a, b):\n    return a + b\n";
    std::string cppSrc =
        "#include <iostream>\nint main() { return 0; }\n";

    // ---------------------------------------------------------------
    //  Test 1: openFile creates a buffer and parses AST
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "openFile",
                          {{"path", "greet.py"},
                           {"content", pySrc},
                           {"language", "python"}});
        bool hasPath = resp.contains("result") &&
            resp["result"].value("path", "") == "greet.py";
        int bufCount = resp["result"].value("bufferCount", 0);
        expect(hasPath && bufCount == 1,
               "openFile creates buffer (bufferCount=1)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Open 3 files, listBuffers returns all 3
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "a.py"}, {"content", pySrc}});
        rpc(state, "s1", "openFile",
            {{"path", "b.py"}, {"content", pySrc2}});
        rpc(state, "s1", "openFile",
            {{"path", "c.cpp"}, {"content", cppSrc},
             {"language", "cpp"}});

        json resp = rpc(state, "s1", "listBuffers");
        int count = resp["result"].value("count", 0);
        bool hasBuffers = resp["result"].contains("buffers") &&
            resp["result"]["buffers"].is_array();
        expect(count == 3 && hasBuffers,
               "listBuffers returns 3 buffers after opening 3 files",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: setActiveBuffer switches which buffer getAST reads
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "a.py"}, {"content", pySrc}});
        rpc(state, "s1", "openFile",
            {{"path", "b.py"}, {"content", pySrc2}});

        // Active should be "a.py" (first opened)
        json ast1 = rpc(state, "s1", "getAST", {{"compact", true}});

        // Switch to b.py
        json switchResp = rpc(state, "s1", "setActiveBuffer",
                                {{"path", "b.py"}});
        std::string activePath =
            switchResp["result"].value("activeBuffer", "");

        json ast2 = rpc(state, "s1", "getAST", {{"compact", true}});

        // The two ASTs should differ (different source code)
        std::string ast1Dump = ast1["result"].dump();
        std::string ast2Dump = ast2["result"].dump();
        expect(activePath == "b.py" && ast1Dump != ast2Dump,
               "setActiveBuffer switches AST context",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: closeFile removes buffer from list
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "a.py"}, {"content", pySrc}});
        rpc(state, "s1", "openFile",
            {{"path", "b.py"}, {"content", pySrc2}});

        json closeResp = rpc(state, "s1", "closeFile",
                               {{"path", "a.py"}});
        int remaining = closeResp["result"].value("bufferCount", 0);

        json listResp = rpc(state, "s1", "listBuffers");
        int count = listResp["result"].value("count", 0);
        expect(remaining == 1 && count == 1,
               "closeFile removes buffer (1 remaining)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Language auto-detection from file extension
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "openFile",
                          {{"path", "app.cpp"}, {"content", cppSrc}});
        std::string lang = resp["result"].value("language", "");
        expect(lang == "cpp",
               "Language auto-detected as 'cpp' for .cpp file",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: listBuffers shows active flag correctly
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "a.py"}, {"content", pySrc}});
        rpc(state, "s1", "openFile",
            {{"path", "b.py"}, {"content", pySrc2}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "b.py"}});

        json resp = rpc(state, "s1", "listBuffers");
        bool aActive = false, bActive = false;
        for (const auto& buf : resp["result"]["buffers"]) {
            if (buf.value("path", "") == "a.py")
                aActive = buf.value("active", false);
            if (buf.value("path", "") == "b.py")
                bActive = buf.value("active", false);
        }
        expect(!aActive && bActive,
               "listBuffers active flag: a=false, b=true",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Workspace indexing scans files
    // ---------------------------------------------------------------
    {
        TempWorkspace ws;
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "indexWorkspace",
                          {{"root", ws.root}});
        int fileCount = resp["result"].value("fileCount", 0);
        int dirCount = resp["result"].value("dirCount", 0);
        // 5 files: main.py, utils.py, app.cpp, test_main.py, README.md
        // 2 dirs: src, tests
        expect(fileCount == 5 && dirCount == 2,
               "indexWorkspace finds 5 files, 2 dirs",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: openFile reads from disk when no content provided
    // ---------------------------------------------------------------
    {
        TempWorkspace ws;
        HeadlessEditorState state;
        state.workspaceRoot = ws.root;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "openFile",
                          {{"path", "src/main.py"}});
        bool success = resp.contains("result") &&
            !resp.contains("error");

        // Verify AST was parsed from disk content
        json ast = rpc(state, "s1", "getAST", {{"compact", true}});
        bool hasNodes = ast.contains("result") &&
            (ast["result"].contains("nodes") ||
             ast["result"].contains("ast"));
        expect(success && hasNodes,
               "openFile reads from disk and parses AST",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: closeFile on nonexistent buffer returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "closeFile",
                          {{"path", "nonexistent.py"}});
        bool hasError = resp.contains("error");
        expect(hasError,
               "closeFile on nonexistent buffer returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Linter role cannot openFile (mutation)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Linter);

        json resp = rpc(state, "s1", "openFile",
                          {{"path", "test.py"}, {"content", pySrc}});
        bool denied = resp.contains("error");
        expect(denied,
               "Linter role denied openFile (requires Refactor)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tools registered for project operations
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("test.py", pySrc, "python");
        state.setAgentRole("mcp-session", AgentRole::Refactor);

        MCPBridge bridge;
        bridge.setRequestHandler([&state](const json& request) -> json {
            return state.processAgentRequest(request, "mcp-session");
        });

        json initReq = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "initialize"},
                        {"params", {{"protocolVersion", "2024-11-05"},
                                    {"clientInfo", {{"name", "test"}}}}}};
        bridge.processMessage(initReq.dump());

        json toolsReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "tools/list"}};
        std::string toolsResp = bridge.processMessage(toolsReq.dump());
        json toolsJson = json::parse(toolsResp);

        std::set<std::string> expected = {
            "whetstone_open_file", "whetstone_close_file",
            "whetstone_list_buffers", "whetstone_set_active_buffer",
            "whetstone_index_workspace"
        };
        std::set<std::string> found;
        for (const auto& t : toolsJson["result"]["tools"]) {
            std::string name = t.value("name", "");
            if (expected.count(name)) found.insert(name);
        }
        int totalTools = (int)toolsJson["result"]["tools"].size();
        expect(found.size() == 5 && totalTools == 27,
               "5 project tools registered (" +
               std::to_string(totalTools) + " total tools)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Full workflow — open, switch, query, close
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        // Open two files
        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", pySrc}});
        rpc(state, "s1", "openFile",
            {{"path", "math.py"}, {"content", pySrc2}});

        // Get AST from first (active)
        json ast1 = rpc(state, "s1", "getAST", {{"compact", true}});
        int nodes1 = ast1["result"].value("nodeCount", 0);

        // Switch and get AST from second
        rpc(state, "s1", "setActiveBuffer", {{"path", "math.py"}});
        json ast2 = rpc(state, "s1", "getAST", {{"compact", true}});
        int nodes2 = ast2["result"].value("nodeCount", 0);

        // Both should have nodes
        bool bothHaveNodes = nodes1 > 0 && nodes2 > 0;

        // Close first, verify active stays on second
        rpc(state, "s1", "closeFile", {{"path", "greet.py"}});
        json list = rpc(state, "s1", "listBuffers");
        int remaining = list["result"].value("count", 0);
        std::string active = list["result"].value("activeBuffer", "");

        expect(bothHaveNodes && remaining == 1 && active == "math.py",
               "Full workflow: open→switch→query→close " +
               std::to_string(nodes1) + "/" + std::to_string(nodes2) +
               " nodes, 1 remaining",
               passed, failed);
    }

    std::cout << "\n=== Step 258 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
