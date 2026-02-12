// Step 267 TDD Test: Sidecar AST Persistence
//
// Tests saving/loading annotated ASTs as .whetstone/ sidecar files.
// Annotations live alongside the codebase without polluting source.
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

int main() {
    int passed = 0;
    int failed = 0;

    auto tmpDir = std::filesystem::temp_directory_path() /
                  "whetstone_step267_test";
    std::filesystem::remove_all(tmpDir);
    std::filesystem::create_directories(tmpDir);

    std::string src =
        "def greet(name):\n"
        "    return \"hello \" + name\n";

    // ---------------------------------------------------------------
    //  Test 1: saveAnnotatedAST creates sidecar file
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});

        // Add an annotation
        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "greets by name"},
                                       {"category", "io"}}}}}});

        json resp = rpc(state, "s1", "saveAnnotatedAST",
                        {{"path", "greet.py"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);

        auto sidecarPath = tmpDir / ".whetstone" / "greet.py.ast.json";
        bool exists = std::filesystem::exists(sidecarPath);

        expect(success && exists,
               "saveAnnotatedAST creates .whetstone/greet.py.ast.json",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: loadAnnotatedAST restores annotations
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        // Open same file fresh (no annotations)
        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});

        // Load sidecar
        json resp = rpc(state, "s1", "loadAnnotatedAST",
                        {{"path", "greet.py"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);

        // Check annotation restored
        bool found = false;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                for (auto* a : c->getChildren("annotations")) {
                    if (a->conceptType == "IntentAnnotation") {
                        auto* ia = static_cast<IntentAnnotation*>(a);
                        if (ia->summary == "greets by name") found = true;
                    }
                }
            }
        }

        expect(success && found,
               "loadAnnotatedAST restores IntentAnnotation from sidecar",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Sidecar merge matches by node ID
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});

        // Load sidecar and check that the annotation is on the right node
        rpc(state, "s1", "loadAnnotatedAST", {{"path", "greet.py"}});

        int annotatedFunctions = 0;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                auto annos = c->getChildren("annotations");
                for (auto* a : annos) {
                    if (a->conceptType == "IntentAnnotation")
                        ++annotatedFunctions;
                }
            }
        }

        expect(annotatedFunctions == 1,
               "Sidecar annotations merged onto correct function node",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: listAnnotatedFiles returns saved sidecars
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "listAnnotatedFiles");
        bool hasResult = resp.contains("result");
        int count = 0;
        if (hasResult && resp["result"].contains("files")) {
            count = (int)resp["result"]["files"].size();
        }

        expect(hasResult && count >= 1,
               "listAnnotatedFiles returns " + std::to_string(count) + " file(s)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Multiple annotations survive roundtrip
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "multi.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Add 3 annotations
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "greets"}, {"category", "io"}}}}}});
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "RiskAnnotation"},
                       {"properties", {{"level", "low"}, {"reason", "simple"},
                                       {"dependentCount", 1}}}}}});
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ComplexityAnnotation"},
                       {"properties", {{"timeComplexity", "O(1)"},
                                       {"cognitiveComplexity", 1},
                                       {"linesOfLogic", 2}}}}}});

        // Save
        rpc(state, "s1", "saveAnnotatedAST", {{"path", "multi.py"}});

        // Reload fresh
        HeadlessEditorState state2;
        state2.defaultLanguage = "python";
        state2.workspaceRoot = tmpDir.string();
        state2.setAgentRole("s1", AgentRole::Refactor);
        rpc(state2, "s1", "openFile",
            {{"path", "multi.py"}, {"content", src}});
        rpc(state2, "s1", "loadAnnotatedAST", {{"path", "multi.py"}});

        int count = 0;
        for (auto* c : state2.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                for (auto* a : c->getChildren("annotations")) {
                    if (a->conceptType == "IntentAnnotation" ||
                        a->conceptType == "RiskAnnotation" ||
                        a->conceptType == "ComplexityAnnotation")
                        ++count;
                }
            }
        }

        expect(count == 3,
               "3 annotations survive save/load roundtrip (" +
               std::to_string(count) + "/3)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Sidecar doesn't affect source code
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});
        rpc(state, "s1", "loadAnnotatedAST", {{"path", "greet.py"}});

        // editBuf should still be the original source
        bool codeClean = state.activeBuffer->editBuf.find("IntentAnnotation") ==
                         std::string::npos;
        bool hasGreet = state.activeBuffer->editBuf.find("greet") !=
                        std::string::npos;

        expect(codeClean && hasGreet,
               "Loading sidecar doesn't pollute source code in editBuf",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: saveAnnotatedAST on unknown buffer returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "saveAnnotatedAST",
                        {{"path", "nonexistent.py"}});
        bool hasError = resp.contains("error");

        expect(hasError,
               "saveAnnotatedAST on unknown buffer returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: loadAnnotatedAST with no sidecar returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "nosidecar.py"}, {"content", src}});

        json resp = rpc(state, "s1", "loadAnnotatedAST",
                        {{"path", "nosidecar.py"}});
        bool hasError = resp.contains("error");

        expect(hasError,
               "loadAnnotatedAST with no sidecar file returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Overwrite existing sidecar
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});

        // Add a different annotation
        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ContractAnnotation"},
                       {"properties", {{"preconditions", "name != empty"},
                                       {"postconditions", "returns greeting"},
                                       {"returnShape", "str"},
                                       {"sideEffects", "none"}}}}}});

        // Overwrite sidecar
        rpc(state, "s1", "saveAnnotatedAST", {{"path", "greet.py"}});

        // Reload and check ContractAnnotation exists
        HeadlessEditorState state2;
        state2.defaultLanguage = "python";
        state2.workspaceRoot = tmpDir.string();
        state2.setAgentRole("s1", AgentRole::Refactor);
        rpc(state2, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});
        rpc(state2, "s1", "loadAnnotatedAST", {{"path", "greet.py"}});

        bool hasContract = false;
        for (auto* c : state2.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                for (auto* a : c->getChildren("annotations")) {
                    if (a->conceptType == "ContractAnnotation")
                        hasContract = true;
                }
            }
        }

        expect(hasContract,
               "Overwritten sidecar has new ContractAnnotation",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Linter cannot save annotations
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("linter", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "greet.py"}, {"content", src}});

        json resp = rpc(state, "linter", "saveAnnotatedAST",
                        {{"path", "greet.py"}});
        bool denied = resp.contains("error");

        // Linter CAN load (read-only)
        json loadResp = rpc(state, "linter", "loadAnnotatedAST",
                            {{"path", "greet.py"}});
        bool loadOk = loadResp.contains("result");

        expect(denied && loadOk,
               "Linter denied save, allowed load",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tools registered
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool foundSave = false, foundLoad = false, foundList = false;
        for (const auto& t : mcp.getTools()) {
            if (t.name == "whetstone_save_annotated_ast") foundSave = true;
            if (t.name == "whetstone_load_annotated_ast") foundLoad = true;
            if (t.name == "whetstone_list_annotated_files") foundList = true;
        }
        expect(foundSave && foundLoad && foundList,
               "3 sidecar MCP tools registered",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Response includes annotation count and sidecar path
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "greet.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }
        rpc(state, "s1", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "test"},
                                       {"category", "io"}}}}}});

        json resp = rpc(state, "s1", "saveAnnotatedAST",
                        {{"path", "greet.py"}});
        bool hasPath = resp["result"].contains("sidecarPath");
        bool hasCount = resp["result"].contains("annotationCount");
        int count = resp["result"].value("annotationCount", 0);

        expect(hasPath && hasCount && count >= 1,
               "Response has sidecarPath and annotationCount=" +
               std::to_string(count),
               passed, failed);
    }

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "\n=== Step 267 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
