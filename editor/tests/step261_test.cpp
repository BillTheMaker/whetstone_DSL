// Step 261 TDD Test: Project-Wide Search and Refactor
//
// Tests searchProject (find symbol references across files by name/nodeId),
// renameSymbol (rename across all files with preview and apply modes),
// and MCP tool registration.
//
// Note: Python parser creates FunctionCall nodes for calls in return
// statements and expressions, but not inside assignments. Test sources
// use return-based calls to ensure proper AST node creation.
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

    // Module A: defines helper and compute; compute calls helper in return
    std::string srcA =
        "def helper(x):\n"
        "    return x * 2\n\n"
        "def compute(a, b):\n"
        "    return helper(a) + b\n";

    // Module B: defines process, calls helper in return (cross-file ref)
    std::string srcB =
        "def process(data):\n"
        "    return helper(data)\n";

    // ---------------------------------------------------------------
    //  Test 1: searchProject by name — finds references across files
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "lib.py"}, {"content", srcB}});

        json resp = rpc(state, "s1", "searchProject",
                          {{"name", "helper"}});
        bool hasResult = resp.contains("result");
        int count = hasResult ? resp["result"].value("count", 0) : 0;
        int fileCount = hasResult
            ? resp["result"].value("fileCount", 0) : 0;
        // app.py: function def + call in compute; lib.py: call in process
        expect(hasResult && count >= 3 && fileCount == 2,
               "searchProject 'helper': " + std::to_string(count) +
               " refs across " + std::to_string(fileCount) + " files",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: searchProject returns correct fields
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "searchProject",
                          {{"name", "helper"}});
        bool correctFields = false;
        if (resp.contains("result") &&
            !resp["result"]["references"].empty()) {
            const auto& ref = resp["result"]["references"][0];
            correctFields = ref.contains("file") &&
                            ref.contains("line") &&
                            ref.contains("col") &&
                            ref.contains("nodeId") &&
                            ref.contains("kind") &&
                            ref.contains("context");
        }
        expect(correctFields,
               "Search results have file, line, col, nodeId, kind, "
               "context",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: searchProject by nodeId
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "lib.py"}, {"content", srcB}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});

        // Find the helper function node ID
        std::string helperId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "helper") {
                helperId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "searchProject",
                          {{"nodeId", helperId}});
        int count = resp["result"].value("count", 0);
        std::string resolvedName = resp["result"].value("name", "");
        expect(count >= 3 && resolvedName == "helper",
               "searchProject by nodeId resolves to 'helper', " +
               std::to_string(count) + " refs",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: searchProject distinguishes definition vs call
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "searchProject",
                          {{"name", "helper"}});
        bool hasDef = false;
        bool hasCall = false;
        for (const auto& ref : resp["result"]["references"]) {
            std::string kind = ref.value("kind", "");
            if (kind == "definition") hasDef = true;
            if (kind == "call") hasCall = true;
        }
        expect(hasDef && hasCall,
               "Search distinguishes definition and call kinds",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: renameSymbol preview mode
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "lib.py"}, {"content", srcB}});

        json resp = rpc(state, "s1", "renameSymbol",
                          {{"oldName", "helper"},
                           {"newName", "assist"},
                           {"preview", true}});
        bool isPreview = resp["result"].value("preview", false);
        int changeCount = resp["result"].value("changeCount", 0);
        bool hasChanges = resp["result"].contains("changes") &&
            resp["result"]["changes"].is_array();
        // Preview should NOT actually rename
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});
        bool notRenamed = false;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "helper") {
                notRenamed = true;
                break;
            }
        }
        expect(isPreview && hasChanges && changeCount >= 3 && notRenamed,
               "Preview: " + std::to_string(changeCount) +
               " changes, NOT applied",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: renameSymbol apply mode — function renamed across files
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "lib.py"}, {"content", srcB}});

        json resp = rpc(state, "s1", "renameSymbol",
                          {{"oldName", "helper"},
                           {"newName", "assist"}});
        int applied = resp["result"].value("applied", 0);
        int fileCount = resp["result"].value("fileCount", 0);

        // Verify rename took effect in app.py (walk full tree)
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});
        bool defRenamed = false;
        bool callRenamed = false;
        std::function<void(ASTNode*)> walkA = [&](ASTNode* n) {
            if (!n) return;
            if (n->conceptType == "Function" &&
                getNodeName(n) == "assist")
                defRenamed = true;
            if (n->conceptType == "FunctionCall" &&
                getNodeName(n) == "assist")
                callRenamed = true;
            for (auto* c : n->allChildren()) walkA(c);
        };
        walkA(state.activeAST());

        // Verify rename took effect in lib.py
        rpc(state, "s1", "setActiveBuffer", {{"path", "lib.py"}});
        bool libCallRenamed = false;
        std::function<void(ASTNode*)> walkB = [&](ASTNode* n) {
            if (!n) return;
            if (n->conceptType == "FunctionCall" &&
                getNodeName(n) == "assist")
                libCallRenamed = true;
            for (auto* c : n->allChildren()) walkB(c);
        };
        walkB(state.activeAST());

        expect(applied >= 3 && fileCount == 2 &&
               defRenamed && callRenamed && libCallRenamed,
               "Rename applied: " + std::to_string(applied) +
               " changes across " + std::to_string(fileCount) +
               " files, verified in both ASTs",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: renameSymbol — parameter and variable reference rename
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        // Use parameter (properly parsed) and VariableReference in return
        std::string src =
            "def foo(result):\n"
            "    return result + 1\n";
        rpc(state, "s1", "openFile",
            {{"path", "var.py"}, {"content", src}});

        json resp = rpc(state, "s1", "renameSymbol",
                          {{"oldName", "result"},
                           {"newName", "output"}});
        int applied = resp["result"].value("applied", 0);

        // Verify: parameter and reference both renamed (walk tree)
        rpc(state, "s1", "setActiveBuffer", {{"path", "var.py"}});
        bool paramRenamed = false;
        bool refRenamed = false;
        std::function<void(ASTNode*)> walk = [&](ASTNode* n) {
            if (!n) return;
            if (n->conceptType == "Parameter" &&
                getNodeName(n) == "output")
                paramRenamed = true;
            if (n->conceptType == "VariableReference" &&
                getNodeName(n) == "output")
                refRenamed = true;
            for (auto* c : n->allChildren()) walk(c);
        };
        walk(state.activeAST());
        expect(applied >= 2 && paramRenamed && refRenamed,
               "Param/ref rename: " + std::to_string(applied) +
               " changes, param+ref both updated",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: renameSymbol — no matches returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "renameSymbol",
                          {{"oldName", "nonexistent"},
                           {"newName", "something"}});
        bool hasError = resp.contains("error");
        expect(hasError,
               "Rename of nonexistent symbol returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Linter role can search but not rename
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("linter", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "app.py"}, {"content", srcA}});

        // Linter can search
        json searchResp = rpc(state, "linter", "searchProject",
                                {{"name", "helper"}});
        bool searchOk = searchResp.contains("result");

        // Linter cannot rename
        json renameResp = rpc(state, "linter", "renameSymbol",
                                {{"oldName", "helper"},
                                 {"newName", "assist"}});
        bool renameDenied = renameResp.contains("error");

        expect(searchOk && renameDenied,
               "Linter can search but cannot rename",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: searchProject — no results for unknown name
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "searchProject",
                          {{"name", "zzz_not_found"}});
        int count = resp["result"].value("count", 0);
        expect(count == 0,
               "Search for unknown name returns 0 references",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tools registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool foundSearch = false;
        bool foundRename = false;
        for (const auto& t : mcp.getTools()) {
            if (t.name == "whetstone_search_project")
                foundSearch = true;
            if (t.name == "whetstone_rename_symbol")
                foundRename = true;
        }
        expect(foundSearch && foundRename,
               "whetstone_search_project and whetstone_rename_symbol "
               "MCP tools registered",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: renameSymbol marks buffers as modified
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "lib.py"}, {"content", srcB}});

        // Reset modified flags
        for (auto& [path, buf] : state.bufferStates)
            buf->modified = false;

        rpc(state, "s1", "renameSymbol",
            {{"oldName", "helper"}, {"newName", "assist"}});

        bool appModified = false;
        bool libModified = false;
        for (const auto& [path, buf] : state.bufferStates) {
            if (path == "app.py" && buf->modified) appModified = true;
            if (path == "lib.py" && buf->modified) libModified = true;
        }
        expect(appModified && libModified,
               "Rename marks both app.py and lib.py as modified",
               passed, failed);
    }

    std::cout << "\n=== Step 261 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
