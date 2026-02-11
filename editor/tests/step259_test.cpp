// Step 259 TDD Test: Cross-File Symbol Resolution
//
// Tests crossFile parameter on getInScopeSymbols, import graph tracking,
// exported symbol collection across open buffers, and getImportGraph RPC.
#include "HeadlessEditorState.h"
#include "CompactAST.h"

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

    // Module A: defines helper and compute, imports utils
    std::string srcA =
        "import utils\n\n"
        "def helper(x):\n"
        "    return x * 2\n\n"
        "def compute(a, b):\n"
        "    c = helper(a)\n"
        "    return c + b\n";

    // Module B: defines transform and process
    std::string srcB =
        "def transform(data):\n"
        "    return data + 1\n\n"
        "def process(items):\n"
        "    result = transform(items)\n"
        "    return result\n";

    // Module C: imports both A and B
    std::string srcC =
        "import app\n"
        "import utils\n\n"
        "def main():\n"
        "    x = 42\n"
        "    return x\n";

    // ---------------------------------------------------------------
    //  Test 1: getInScopeSymbols without crossFile (baseline)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "getInScopeSymbols",
                          {{"nodeId", computeId}});
        int count = resp["result"].value("count", 0);
        bool noCrossFile = !resp["result"].contains("crossFile");
        // Should only have symbols from app.py
        bool noFileField = true;
        for (const auto& s : resp["result"]["symbols"]) {
            if (s.contains("file")) noFileField = false;
        }
        expect(count > 0 && noCrossFile && noFileField,
               "Baseline: " + std::to_string(count) +
               " local symbols, no crossFile flag",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: getInScopeSymbols with crossFile includes other buffers
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "getInScopeSymbols",
                          {{"nodeId", computeId}, {"crossFile", true}});
        int count = resp["result"].value("count", 0);
        bool hasCrossFile = resp["result"].value("crossFile", false);

        // Should include symbols from utils.py (transform, process)
        std::set<std::string> names;
        for (const auto& s : resp["result"]["symbols"])
            names.insert(s.value("name", ""));

        bool hasTransform = names.count("transform") > 0;
        bool hasProcess = names.count("process") > 0;
        expect(hasCrossFile && hasTransform && hasProcess,
               "crossFile=true includes transform, process from utils.py (" +
               std::to_string(count) + " total)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Cross-file symbols include file path
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "getInScopeSymbols",
                          {{"nodeId", computeId}, {"crossFile", true}});
        bool hasFileField = false;
        std::string fileVal;
        for (const auto& s : resp["result"]["symbols"]) {
            if (s.contains("file")) {
                hasFileField = true;
                fileVal = s.value("file", "");
                break;
            }
        }
        expect(hasFileField && fileVal == "utils.py",
               "Cross-file symbols have file='utils.py'",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Import graph populated on openFile
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});

        json resp = rpc(state, "s1", "getImportGraph",
                          {{"file", "app.py"}});
        bool hasImports = resp["result"].contains("imports") &&
            resp["result"]["imports"].is_array();
        int importCount = hasImports
            ? (int)resp["result"]["imports"].size() : 0;
        expect(hasImports && importCount == 1,
               "app.py imports 1 module (utils)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Full import graph across multiple files
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getImportGraph");
        int fileCount = resp["result"].value("fileCount", 0);
        bool hasEdges = resp["result"].contains("edges");
        // app.py imports utils; main.py imports app and utils
        // utils.py has no imports
        expect(hasEdges && fileCount == 2,
               "Import graph has 2 files with imports",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: importedBy query — who imports "utils"?
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        json resp = rpc(state, "s1", "getImportGraph",
                          {{"file", "utils.py"}});
        bool hasImportedBy = resp["result"].contains("importedBy") &&
            resp["result"]["importedBy"].is_array();
        int importerCount = hasImportedBy
            ? (int)resp["result"]["importedBy"].size() : 0;
        // Both app.py and main.py import "utils"
        expect(hasImportedBy && importerCount == 2,
               "utils imported by 2 files (app.py, main.py)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Cross-file detailed mode includes node JSON
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "getInScopeSymbols",
                          {{"nodeId", computeId},
                           {"crossFile", true},
                           {"detailed", true}});
        bool hasNodeOnCrossFile = false;
        for (const auto& s : resp["result"]["symbols"]) {
            if (s.contains("file") && s.contains("node"))
                hasNodeOnCrossFile = true;
        }
        expect(hasNodeOnCrossFile,
               "Cross-file detailed mode includes node JSON",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: Cross-file count is larger than local-only
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "app.py"}});

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json local = rpc(state, "s1", "getInScopeSymbols",
                           {{"nodeId", computeId}});
        json cross = rpc(state, "s1", "getInScopeSymbols",
                           {{"nodeId", computeId},
                            {"crossFile", true}});
        int localCount = local["result"].value("count", 0);
        int crossCount = cross["result"].value("count", 0);
        expect(crossCount > localCount,
               "Cross-file count (" + std::to_string(crossCount) +
               ") > local count (" + std::to_string(localCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: closeFile removes from import graph
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcC}});

        // Before close: 2 files in import graph
        json before = rpc(state, "s1", "getImportGraph");
        int beforeCount = before["result"].value("fileCount", 0);

        rpc(state, "s1", "closeFile", {{"path", "main.py"}});

        // After close: only 1 file should remain
        json after = rpc(state, "s1", "getImportGraph");
        int afterCount = after["result"].value("fileCount", 0);
        expect(beforeCount == 2 && afterCount == 1,
               "closeFile removes from import graph (2→1)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Cross-file with no other buffers returns local only
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "solo.py"}, {"content", srcA}});

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json local = rpc(state, "s1", "getInScopeSymbols",
                           {{"nodeId", computeId}});
        json cross = rpc(state, "s1", "getInScopeSymbols",
                           {{"nodeId", computeId},
                            {"crossFile", true}});
        int localCount = local["result"].value("count", 0);
        int crossCount = cross["result"].value("count", 0);
        // With only one buffer, crossFile adds nothing
        expect(localCount == crossCount,
               "Single buffer: crossFile=local (" +
               std::to_string(crossCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: collectExportedSymbols extracts module-level symbols
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});

        auto it = state.bufferStates.find("utils.py");
        Module* ast = it->second->sync.getAST();
        auto exports = collectExportedSymbols(ast, "utils.py");
        std::set<std::string> names;
        for (const auto& ex : exports)
            names.insert(ex.name);
        bool hasTransform = names.count("transform") > 0;
        bool hasProcess = names.count("process") > 0;
        expect(hasTransform && hasProcess && exports.size() == 2,
               "collectExportedSymbols: transform, process (2 total)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Linter role can read import graph and cross-file scope
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("s1", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "app.py"}, {"content", srcA}});
        rpc(state, "admin", "openFile",
            {{"path", "utils.py"}, {"content", srcB}});

        // Linter can read import graph
        json graphResp = rpc(state, "s1", "getImportGraph");
        bool graphOk = graphResp.contains("result");

        // Linter can query cross-file scope
        state.setActiveBuffer("app.py");
        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }
        json scopeResp = rpc(state, "s1", "getInScopeSymbols",
                               {{"nodeId", computeId},
                                {"crossFile", true}});
        bool scopeOk = scopeResp.contains("result");
        expect(graphOk && scopeOk,
               "Linter role can read import graph and cross-file scope",
               passed, failed);
    }

    std::cout << "\n=== Step 259 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
