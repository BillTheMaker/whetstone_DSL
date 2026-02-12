// Step 262 TDD Test: Phase 9d Multi-File Project Integration Tests
//
// End-to-end integration tests exercising the full multi-file project
// workflow: open files, switch buffers, cross-file symbols, import graph,
// project-wide diagnostics, project-wide search, cross-file rename,
// and batch queries across multiple buffers.
//
// Follows the Phase closer pattern from Steps 249, 253, 257.
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

    // Shared test sources — a 3-file Python project
    std::string srcUtils =
        "def clamp(value, lo, hi):\n"
        "    if value < lo:\n"
        "        return lo\n"
        "    if value > hi:\n"
        "        return hi\n"
        "    return value\n\n"
        "def double(x):\n"
        "    return x * 2\n";

    std::string srcMath =
        "import utils\n\n"
        "def transform(data):\n"
        "    return clamp(double(data), 0, 100)\n";

    std::string srcMain =
        "import utils\n"
        "import math_ops\n\n"
        "def run(input):\n"
        "    return transform(clamp(input, 1, 50))\n";

    // ---------------------------------------------------------------
    //  Test 1: Open 3 files, listBuffers shows all with correct state
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcMain}});

        json resp = rpc(state, "s1", "listBuffers");
        int count = resp["result"].value("count", 0);
        bool allPresent = false;
        if (count == 3) {
            std::set<std::string> paths;
            for (const auto& b : resp["result"]["buffers"])
                paths.insert(b.value("path", ""));
            allPresent = paths.count("utils.py") &&
                         paths.count("math_ops.py") &&
                         paths.count("main.py");
        }
        expect(count == 3 && allPresent,
               "3 files open, all present in listBuffers",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: setActiveBuffer switches context, getAST returns
    //          correct module for each buffer
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});

        rpc(state, "s1", "setActiveBuffer", {{"path", "utils.py"}});
        json astU = rpc(state, "s1", "getAST", {{"compact", true}});
        // utils.py has clamp and double
        bool hasClamp = false;
        for (const auto& n : astU["result"]["nodes"]) {
            if (n.value("name", "") == "clamp") hasClamp = true;
        }

        rpc(state, "s1", "setActiveBuffer", {{"path", "math_ops.py"}});
        json astM = rpc(state, "s1", "getAST", {{"compact", true}});
        bool hasTransform = false;
        for (const auto& n : astM["result"]["nodes"]) {
            if (n.value("name", "") == "transform") hasTransform = true;
        }

        expect(hasClamp && hasTransform,
               "Switching buffers returns correct AST per file",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Cross-file symbol resolution — crossFile=true includes
    //          exports from other open buffers
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});
        rpc(state, "s1", "setActiveBuffer", {{"path", "math_ops.py"}});

        // Get the module node ID for math_ops
        std::string moduleId;
        Module* ast = state.activeAST();
        if (ast) moduleId = ast->id;

        // Local scope only
        json localResp = rpc(state, "s1", "getInScopeSymbols",
                               {{"nodeId", moduleId}});
        int localCount = localResp["result"].value("count", 0);

        // Cross-file scope
        json crossResp = rpc(state, "s1", "getInScopeSymbols",
                               {{"nodeId", moduleId},
                                {"crossFile", true}});
        int crossCount = crossResp["result"].value("count", 0);
        bool crossHasFile = false;
        for (const auto& s : crossResp["result"]["symbols"]) {
            if (s.contains("file") &&
                s.value("file", "") == "utils.py") {
                crossHasFile = true;
                break;
            }
        }

        expect(crossCount > localCount && crossHasFile,
               "Cross-file scope includes utils.py exports ("
               + std::to_string(crossCount) + " > "
               + std::to_string(localCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Import graph tracks which files import which
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcMain}});

        // Check full import graph
        json graphResp = rpc(state, "s1", "getImportGraph");
        int fileCount = graphResp["result"].value("fileCount", 0);

        // Check per-file imports for main.py
        json mainGraph = rpc(state, "s1", "getImportGraph",
                               {{"file", "main.py"}});
        bool importsUtils = false;
        bool importsMath = false;
        for (const auto& m : mainGraph["result"]["imports"]) {
            if (m == "utils") importsUtils = true;
            if (m == "math_ops") importsMath = true;
        }

        expect(fileCount >= 2 && importsUtils && importsMath,
               "Import graph: main.py imports utils + math_ops, "
               + std::to_string(fileCount) + " files tracked",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Project-wide diagnostics — cross-file E0400 for
    //          undefined import, no E0400 for known imports
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        // Open utils and math_ops but NOT main
        // So math_ops imports "utils" which IS open — no error
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});

        json resp = rpc(state, "s1", "getProjectDiagnostics");
        int totalDiags = resp["result"].value("totalDiagnostics", 0);

        // Now open a file that imports a module NOT open
        std::string srcBad =
            "import nonexistent_module\n\n"
            "def broken():\n"
            "    return 1\n";
        rpc(state, "s1", "openFile",
            {{"path", "bad.py"}, {"content", srcBad}});

        json resp2 = rpc(state, "s1", "getProjectDiagnostics");
        int totalDiags2 = resp2["result"].value("totalDiagnostics", 0);

        // bad.py should have at least one cross-file diagnostic
        bool badHasDiag = resp2["result"]["files"].contains("bad.py");

        expect(totalDiags2 > totalDiags && badHasDiag,
               "Undefined import in bad.py raises cross-file diagnostic ("
               + std::to_string(totalDiags2) + " > "
               + std::to_string(totalDiags) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: searchProject finds symbol across all 3 files
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcMain}});

        json resp = rpc(state, "s1", "searchProject",
                          {{"name", "clamp"}});
        int count = resp["result"].value("count", 0);
        int fileCount = resp["result"].value("fileCount", 0);

        // clamp: defined in utils.py, called in math_ops.py and main.py
        std::set<std::string> files;
        bool hasDef = false;
        bool hasCall = false;
        for (const auto& ref : resp["result"]["references"]) {
            files.insert(ref.value("file", ""));
            if (ref.value("kind", "") == "definition") hasDef = true;
            if (ref.value("kind", "") == "call") hasCall = true;
        }

        expect(count >= 3 && fileCount >= 2 && hasDef && hasCall,
               "searchProject 'clamp': " + std::to_string(count)
               + " refs across " + std::to_string(fileCount)
               + " files, has definition + call",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: renameSymbol across files — preview then apply
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});
        rpc(state, "s1", "openFile",
            {{"path", "main.py"}, {"content", srcMain}});

        // Preview first
        json preview = rpc(state, "s1", "renameSymbol",
                             {{"oldName", "clamp"},
                              {"newName", "constrain"},
                              {"preview", true}});
        int previewCount = preview["result"].value("changeCount", 0);
        bool isPreview = preview["result"].value("preview", false);

        // Apply the rename
        json apply = rpc(state, "s1", "renameSymbol",
                           {{"oldName", "clamp"},
                            {"newName", "constrain"}});
        int applied = apply["result"].value("applied", 0);
        int fileCount = apply["result"].value("fileCount", 0);

        // Verify: search for new name finds references
        json search = rpc(state, "s1", "searchProject",
                            {{"name", "constrain"}});
        int newCount = search["result"].value("count", 0);

        // Verify: search for old name finds nothing
        json oldSearch = rpc(state, "s1", "searchProject",
                               {{"name", "clamp"}});
        int oldCount = oldSearch["result"].value("count", 0);

        expect(isPreview && previewCount >= 3 &&
               applied >= 3 && fileCount >= 2 &&
               newCount >= 3 && oldCount == 0,
               "Rename clamp→constrain: preview=" +
               std::to_string(previewCount) + ", applied=" +
               std::to_string(applied) + " across " +
               std::to_string(fileCount) + " files, verified",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: Full agent workflow — open, diagnose, search, rename,
    //          re-diagnose, verify via batch query
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        // Step 1: Open project files
        rpc(state, "s1", "openFile",
            {{"path", "utils.py"}, {"content", srcUtils}});
        rpc(state, "s1", "openFile",
            {{"path", "math_ops.py"}, {"content", srcMath}});

        // Step 2: Batch query — diagnostics + search + buffers in one call
        json batchResp = rpc(state, "s1", "batchQuery",
            {{"queries", json::array({
                {{"method", "getProjectDiagnostics"}},
                {{"method", "searchProject"},
                 {"params", {{"name", "double"}}}},
                {{"method", "listBuffers"}}
            })}});

        int batchCount = batchResp["result"].value("count", 0);
        bool allSucceeded = true;
        for (const auto& r : batchResp["result"]["results"]) {
            if (r.contains("error")) allSucceeded = false;
        }

        // Step 3: Rename "double" across project
        rpc(state, "s1", "renameSymbol",
            {{"oldName", "double"}, {"newName", "multiply_by_two"}});

        // Step 4: Verify via batch — new name found, old name gone
        json verifyBatch = rpc(state, "s1", "batchQuery",
            {{"queries", json::array({
                {{"method", "searchProject"},
                 {"params", {{"name", "multiply_by_two"}}}},
                {{"method", "searchProject"},
                 {"params", {{"name", "double"}}}},
                {{"method", "getProjectDiagnostics"}}
            })}});

        int newRefs = 0;
        int oldRefs = 0;
        if (verifyBatch["result"]["results"].size() >= 2) {
            auto& r0 = verifyBatch["result"]["results"][0];
            auto& r1 = verifyBatch["result"]["results"][1];
            if (r0.contains("result"))
                newRefs = r0["result"].value("count", 0);
            if (r1.contains("result"))
                oldRefs = r1["result"].value("count", 0);
        }

        expect(batchCount == 3 && allSucceeded &&
               newRefs >= 2 && oldRefs == 0,
               "Full workflow: batch(3)→rename→verify: new=" +
               std::to_string(newRefs) + " old=" +
               std::to_string(oldRefs),
               passed, failed);
    }

    std::cout << "\n=== Step 262 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
