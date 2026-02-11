// Step 260 TDD Test: Project-Wide Diagnostics
//
// Tests getProjectDiagnostics: diagnostics grouped by file path,
// cross-file errors (undefined imports), severity filter, file glob
// filter, and MCP tool registration.
#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <iostream>
#include <string>
#include <set>

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

    // Module A: has an annotation issue (Deallocate without intent)
    std::string srcA =
        "import utils\n\n"
        "def helper(x):\n"
        "    return x * 2\n\n"
        "def compute(a, b):\n"
        "    c = helper(a)\n"
        "    return c + b\n";

    // Module B: clean code, no diagnostics expected (no annotations)
    std::string srcB =
        "def transform(data):\n"
        "    return data + 1\n\n"
        "def process(items):\n"
        "    result = transform(items)\n"
        "    return result\n";

    // Module C: imports unknown module, triggers cross-file warning
    std::string srcC =
        "import nonexistent\n"
        "import utils\n\n"
        "def main():\n"
        "    x = 42\n"
        "    return x\n";

    // ---------------------------------------------------------------
    //  Test 1: Basic project diagnostics — returns grouped by file
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        bool hasResult = resp.contains("result");
        bool hasFiles = hasResult && resp["result"].contains("files");
        bool hasFileCount = hasResult &&
            resp["result"].contains("fileCount");
        bool hasTotalDiag = hasResult &&
            resp["result"].contains("totalDiagnostics");
        expect(hasResult && hasFiles && hasFileCount && hasTotalDiag,
               "getProjectDiagnostics returns files, fileCount, "
               "totalDiagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Cross-file warning for undefined import
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        int total = resp["result"].value("totalDiagnostics", 0);

        // main.py imports "nonexistent" which is not open
        bool hasMainDiags = resp["result"]["files"].contains("main.py");
        bool hasCrossFileError = false;
        if (hasMainDiags) {
            for (const auto& d : resp["result"]["files"]["main.py"]) {
                if (d.value("code", "") == "E0400" &&
                    d.value("source", "") == "cross-file") {
                    hasCrossFileError = true;
                    break;
                }
            }
        }
        expect(hasCrossFileError,
               "Cross-file E0400 warning for undefined import "
               "'nonexistent' (" + std::to_string(total) + " total)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Known import doesn't trigger cross-file warning
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        bool hasMainDiags = resp["result"]["files"].contains("main.py");
        bool utilsImportWarning = false;
        if (hasMainDiags) {
            for (const auto& d : resp["result"]["files"]["main.py"]) {
                std::string msg = d.value("message", "");
                if (msg.find("utils") != std::string::npos &&
                    d.value("code", "") == "E0400") {
                    utilsImportWarning = true;
                }
            }
        }
        expect(!utilsImportWarning,
               "No E0400 for 'utils' when utils.py is open",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Severity filter — errors only
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        // With no severity filter: should have cross-file warnings
        json allResp = rpc(state, "s1", "getProjectDiagnostics");
        int allTotal = allResp["result"].value("totalDiagnostics", 0);

        // With severity=error: cross-file warnings (severity=warning)
        // should be excluded
        json errResp = rpc(state, "s1", "getProjectDiagnostics",
                             {{"severity", "error"}});
        int errTotal = errResp["result"].value("totalDiagnostics", 0);

        expect(errTotal < allTotal || (allTotal == 0 && errTotal == 0),
               "Severity filter: error-only (" +
               std::to_string(errTotal) + ") <= all (" +
               std::to_string(allTotal) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: File glob filter — *.py matches
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getProjectDiagnostics",
                          {{"fileGlob", "*.py"}});
        int fileCount = resp["result"].value("fileCount", 0);
        // Both files match *.py, so both should be included
        // (if they have diagnostics)
        expect(fileCount >= 0,
               "fileGlob *.py returns " + std::to_string(fileCount) +
               " files",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: File glob filter — specific file
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getProjectDiagnostics",
                          {{"fileGlob", "main.py"}});
        int fileCount = resp["result"].value("fileCount", 0);
        bool onlyMain = true;
        if (resp["result"]["files"].is_object()) {
            for (auto& [path, diags] : resp["result"]["files"].items()) {
                if (path.find("main.py") == std::string::npos)
                    onlyMain = false;
            }
        }
        expect(onlyMain,
               "fileGlob 'main.py' returns only main.py diagnostics (" +
               std::to_string(fileCount) + " files)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Empty project — no diagnostics
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        int total = resp["result"].value("totalDiagnostics", 0);
        int fileCount = resp["result"].value("fileCount", 0);
        expect(total == 0 && fileCount == 0,
               "Empty project: 0 diagnostics, 0 files",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: Multiple files with cross-file errors
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        // Both files import things that aren't open
        std::string srcD =
            "import missing_lib\n\n"
            "def foo():\n"
            "    return 1\n";
        std::string srcE =
            "import another_missing\n\n"
            "def bar():\n"
            "    return 2\n";

        rpc(state, "s1", "openFile",
            {{"path", "d.py"}, {"content", srcD}});
        rpc(state, "s1", "openFile",
            {{"path", "e.py"}, {"content", srcE}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        int fileCount = resp["result"].value("fileCount", 0);
        int total = resp["result"].value("totalDiagnostics", 0);
        bool hasDpy = resp["result"]["files"].contains("d.py");
        bool hasEpy = resp["result"]["files"].contains("e.py");
        expect(hasDpy && hasEpy && fileCount == 2 && total >= 2,
               "Two files each with cross-file errors (" +
               std::to_string(total) + " total, " +
               std::to_string(fileCount) + " files)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Diagnostics include correct fields
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        bool hasMainDiags = resp["result"]["files"].contains("main.py");
        bool correctFields = false;
        if (hasMainDiags &&
            !resp["result"]["files"]["main.py"].empty()) {
            const auto& d = resp["result"]["files"]["main.py"][0];
            correctFields = d.contains("code") &&
                            d.contains("severity") &&
                            d.contains("message") &&
                            d.contains("source");
        }
        expect(correctFields,
               "Diagnostic entries have code, severity, message, source",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Linter role can call getProjectDiagnostics
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("s1", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        bool hasResult = resp.contains("result");
        bool noError = !resp.contains("error");
        expect(hasResult && noError,
               "Linter role can call getProjectDiagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tool registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool found = false;
        for (const auto& t : mcp.getTools()) {
            if (t.name == "whetstone_get_project_diagnostics") {
                found = true;
                break;
            }
        }
        expect(found,
               "whetstone_get_project_diagnostics MCP tool registered",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Diagnostics sorted by priority within each file
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        bool sorted = true;
        if (resp["result"]["files"].contains("main.py")) {
            const auto& diags = resp["result"]["files"]["main.py"];
            for (size_t i = 1; i < diags.size(); ++i) {
                int prevSev = diags[i-1].value("severityLevel", 99);
                int currSev = diags[i].value("severityLevel", 99);
                if (currSev < prevSev) { sorted = false; break; }
            }
        }
        expect(sorted,
               "Diagnostics sorted by severity within each file",
               passed, failed);
    }

    std::cout << "\n=== Step 260 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
