// Step 247 TDD Test: File Operation Tools for MCP
//
// 12 test cases covering path resolution, file CRUD, diff, listing,
// and permission enforcement via HeadlessEditorState + RPC dispatch.
#include "HeadlessEditorState.h"
#include "MCPBridge.h"

#include <iostream>
#include <cassert>
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

static std::string tmpDir;

static void setupTmpDir() {
    tmpDir = "/tmp/whetstone_test_247_" +
        std::to_string(std::chrono::steady_clock::now()
            .time_since_epoch().count());
    fs::create_directories(tmpDir);
}

static void cleanupTmpDir() {
    std::error_code ec;
    fs::remove_all(tmpDir, ec);
}

int main() {
    int passed = 0;
    int failed = 0;

    setupTmpDir();

    // Set up HeadlessEditorState with temp workspace
    HeadlessEditorState state;
    state.workspaceRoot = tmpDir;
    state.defaultLanguage = "python";
    state.verbose = false;

    // Create a test file on disk for reading
    std::string testContent = "line one\nline two\nline three\nline four\n";
    {
        std::ofstream f(tmpDir + "/test.txt");
        f << testContent;
    }

    // Open a buffer with Python source for diff tests
    std::string src = "def greet(name):\n    return 'hello ' + name\n";
    state.openBuffer("greet.py", src, "python");
    state.setAgentRole("test-session", AgentRole::Refactor);

    auto rpc = [&](const std::string& method, const json& params = {}) {
        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", method}, {"params", params}};
        return state.processAgentRequest(request, "test-session");
    };

    // ---------------------------------------------------------------
    //  Test 1: Resolve path within workspace
    // ---------------------------------------------------------------
    {
        auto [ok, resolved] = fileOpsResolvePath(tmpDir, "test.txt");
        expect(ok && resolved == tmpDir + "/test.txt",
               "Resolve path within workspace",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Reject path escaping workspace
    // ---------------------------------------------------------------
    {
        auto [ok, resolved] =
            fileOpsResolvePath(tmpDir, "../../../etc/passwd");
        expect(!ok && resolved.find("escapes") != std::string::npos,
               "Reject path escaping workspace (../../../etc/passwd)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Create file via RPC, verify it exists on disk
    // ---------------------------------------------------------------
    {
        json resp = rpc("fileCreate", {{"path", "new_file.txt"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);
        expect(success && fs::exists(tmpDir + "/new_file.txt"),
               "Create file via RPC, verify on disk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Read file via RPC, verify content matches
    // ---------------------------------------------------------------
    {
        json resp = rpc("fileRead", {{"path", "test.txt"}});
        std::string content = "";
        if (resp.contains("result"))
            content = resp["result"].value("content", "");
        expect(content.find("line one") != std::string::npos &&
               content.find("line four") != std::string::npos,
               "Read file via RPC, content matches",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Read file with line range (startLine/endLine)
    // ---------------------------------------------------------------
    {
        json resp = rpc("fileRead",
            {{"path", "test.txt"}, {"startLine", 2}, {"endLine", 3}});
        std::string content = "";
        if (resp.contains("result"))
            content = resp["result"].value("content", "");
        expect(content.find("line two") != std::string::npos &&
               content.find("line three") != std::string::npos &&
               content.find("line one") == std::string::npos,
               "Read file with line range (startLine/endLine)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Write file via RPC, verify content on disk
    // ---------------------------------------------------------------
    {
        std::string writeContent = "hello from agent\nsecond line\n";
        json resp = rpc("fileWrite",
            {{"path", "written.txt"}, {"content", writeContent}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);
        // Read back from disk
        std::string diskContent;
        if (success) {
            std::ifstream f(tmpDir + "/written.txt");
            std::ostringstream ss;
            ss << f.rdbuf();
            diskContent = ss.str();
        }
        expect(success && diskContent == writeContent,
               "Write file via RPC, verify content on disk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Diff active buffer vs disk after modification
    // ---------------------------------------------------------------
    {
        // Write the original greet.py content to disk
        {
            auto [ok, resolved] =
                fileOpsResolvePath(tmpDir, "greet.py");
            if (ok) {
                std::ofstream f(resolved);
                f << "def greet(name):\n    return 'hi ' + name\n";
            }
        }
        // Buffer has 'hello', disk has 'hi' — should produce a diff
        json resp = rpc("fileDiff", {{"path", "greet.py"}});
        std::string diff = "";
        if (resp.contains("result"))
            diff = resp["result"].value("diff", "");
        expect(!diff.empty() && diff.find("---") != std::string::npos,
               "Diff active buffer vs disk after modification",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: List workspace files, verify count > 0
    // ---------------------------------------------------------------
    {
        json resp = rpc("workspaceList", json::object());
        int count = 0;
        if (resp.contains("result") && resp["result"].contains("files"))
            count = (int)resp["result"]["files"].size();
        expect(count > 0,
               "List workspace files, count > 0 (got " +
               std::to_string(count) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: List workspace with glob filter
    // ---------------------------------------------------------------
    {
        json resp = rpc("workspaceList", {{"glob", "*.txt"}});
        int count = 0;
        bool allTxt = true;
        if (resp.contains("result") && resp["result"].contains("files")) {
            count = (int)resp["result"]["files"].size();
            for (const auto& f : resp["result"]["files"]) {
                std::string p = f.value("path", "");
                if (p.find(".txt") == std::string::npos) allTxt = false;
            }
        }
        expect(count > 0 && allTxt,
               "List workspace with glob *.txt (got " +
               std::to_string(count) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: File create with language template
    // ---------------------------------------------------------------
    {
        json resp = rpc("fileCreate",
            {{"path", "app.py"}, {"language", "python"},
             {"template", "module"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);
        std::string content;
        if (success) {
            std::ifstream f(tmpDir + "/app.py");
            std::ostringstream ss;
            ss << f.rdbuf();
            content = ss.str();
        }
        expect(success && content.find("__main__") != std::string::npos,
               "File create with language template (python module)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: Write creates parent directories
    // ---------------------------------------------------------------
    {
        json resp = rpc("fileWrite",
            {{"path", "sub/dir/deep.txt"}, {"content", "nested"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);
        expect(success && fs::exists(tmpDir + "/sub/dir/deep.txt"),
               "Write creates parent directories",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Permission check: Linter cannot write files
    // ---------------------------------------------------------------
    {
        state.setAgentRole("linter-session", AgentRole::Linter);
        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "fileWrite"},
                        {"params", {{"path", "blocked.txt"},
                                    {"content", "nope"}}}};
        json resp = state.processAgentRequest(request, "linter-session");
        bool denied = resp.contains("error") &&
            resp["error"].value("code", 0) == -32031;
        expect(denied && !fs::exists(tmpDir + "/blocked.txt"),
               "Permission check: Linter cannot write files",
               passed, failed);
    }

    cleanupTmpDir();

    std::cout << "\n=== Step 247 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
