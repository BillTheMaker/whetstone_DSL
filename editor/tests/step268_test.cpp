// Step 268 TDD Test: Phase 10a Integration Tests
//
// End-to-end: create annotations via applyMutation, verify they appear
// in compact AST, save sidecar, re-open file, verify auto-loaded.
#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <iostream>
#include <string>
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
                  "whetstone_step268_test";
    std::filesystem::remove_all(tmpDir);
    std::filesystem::create_directories(tmpDir);

    std::string src =
        "def greet(name):\n"
        "    return \"hello \" + name\n"
        "\n"
        "def farewell(name):\n"
        "    return \"goodbye \" + name\n";

    // ---------------------------------------------------------------
    //  Test 1: Full annotation workflow — create, verify compact, save
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});

        // Find both functions
        std::string greetId, farewellId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                std::string n = getNodeName(c);
                if (n == "greet") greetId = c->id;
                if (n == "farewell") farewellId = c->id;
            }
        }

        // Annotate greet with intent + risk
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", greetId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "greets user by name"},
                                       {"category", "io"}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", greetId},
             {"role", "annotations"},
             {"node", {{"concept", "RiskAnnotation"},
                       {"properties", {{"level", "low"},
                                       {"reason", "simple string concat"},
                                       {"dependentCount", 3}}}}}});

        // Annotate farewell with intent + contract
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", farewellId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "says goodbye"},
                                       {"category", "io"}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", farewellId},
             {"role", "annotations"},
             {"node", {{"concept", "ContractAnnotation"},
                       {"properties", {{"preconditions", "name is string"},
                                       {"postconditions", "returns greeting"},
                                       {"returnShape", "str"},
                                       {"sideEffects", "none"}}}}}});

        // Verify compact AST has semantic fields
        json resp = rpc(state, "agent", "getAST", {{"compact", true}});
        json nodes = resp["result"]["nodes"];

        bool greetSemantic = false, farewellSemantic = false;
        for (const auto& n : nodes) {
            if (n.value("name", "") == "greet" && n.contains("semantic")) {
                auto sem = n["semantic"];
                greetSemantic = sem.contains("intent") && sem.contains("risk");
            }
            if (n.value("name", "") == "farewell" && n.contains("semantic")) {
                auto sem = n["semantic"];
                farewellSemantic = sem.contains("intent") &&
                                   sem.contains("contract");
            }
        }

        // Save sidecar
        json saveResp = rpc(state, "agent", "saveAnnotatedAST",
                            {{"path", "funcs.py"}});
        bool saved = saveResp.contains("result") &&
                     saveResp["result"].value("success", false);
        int annoCount = saveResp["result"].value("annotationCount", 0);

        expect(greetSemantic && farewellSemantic && saved && annoCount >= 4,
               "Annotate 2 functions, compact AST has semantic, sidecar saved (" +
               std::to_string(annoCount) + " annotations)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Re-open file and load sidecar restores all annotations
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});
        rpc(state, "agent", "loadAnnotatedAST", {{"path", "funcs.py"}});

        // Count semantic annotations on both functions
        int greetAnnos = 0, farewellAnnos = 0;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType != "Function") continue;
            std::string name = getNodeName(c);
            for (auto* a : c->getChildren("annotations")) {
                if (isSemanticAnnotation(a->conceptType)) {
                    if (name == "greet") ++greetAnnos;
                    if (name == "farewell") ++farewellAnnos;
                }
            }
        }

        expect(greetAnnos == 2 && farewellAnnos == 2,
               "Sidecar load restores greet=" + std::to_string(greetAnnos) +
               " farewell=" + std::to_string(farewellAnnos) + " annotations",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Compact AST after sidecar load has semantic fields
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});
        rpc(state, "agent", "loadAnnotatedAST", {{"path", "funcs.py"}});

        json resp = rpc(state, "agent", "getAST", {{"compact", true}});
        json nodes = resp["result"]["nodes"];

        int nodesWithSemantic = 0;
        for (const auto& n : nodes) {
            if (n.contains("semantic")) ++nodesWithSemantic;
        }

        expect(nodesWithSemantic >= 2,
               "Compact AST after sidecar load has " +
               std::to_string(nodesWithSemantic) + " nodes with semantic fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Annotations don't appear in generated source code
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});
        rpc(state, "agent", "loadAnnotatedAST", {{"path", "funcs.py"}});

        std::string code = state.activeBuffer->editBuf;
        bool noAnnotationClutter =
            code.find("IntentAnnotation") == std::string::npos &&
            code.find("RiskAnnotation") == std::string::npos &&
            code.find("ContractAnnotation") == std::string::npos;
        bool hasOriginalCode =
            code.find("def greet") != std::string::npos &&
            code.find("def farewell") != std::string::npos;

        expect(noAnnotationClutter && hasOriginalCode,
               "Loaded annotations don't pollute generated source code",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Multiple save/load cycles are idempotent
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});
        rpc(state, "agent", "loadAnnotatedAST", {{"path", "funcs.py"}});

        // Save again (should overwrite with same content)
        rpc(state, "agent", "saveAnnotatedAST", {{"path", "funcs.py"}});

        // Load into fresh state
        HeadlessEditorState state2;
        state2.defaultLanguage = "python";
        state2.workspaceRoot = tmpDir.string();
        state2.setAgentRole("agent", AgentRole::Refactor);
        rpc(state2, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});
        rpc(state2, "agent", "loadAnnotatedAST", {{"path", "funcs.py"}});

        int totalAnnos = 0;
        for (auto* c : state2.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                for (auto* a : c->getChildren("annotations")) {
                    if (isSemanticAnnotation(a->conceptType))
                        ++totalAnnos;
                }
            }
        }

        expect(totalAnnos == 4,
               "Save/load/save/load cycle preserves exactly 4 annotations (" +
               std::to_string(totalAnnos) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Multi-file sidecar workflow
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        std::string src2 = "def helper():\n    pass\n";

        rpc(state, "agent", "openFile",
            {{"path", "funcs.py"}, {"content", src}});
        rpc(state, "agent", "openFile",
            {{"path", "helper.py"}, {"content", src2}});

        // Annotate helper
        state.setActiveBuffer("helper.py");
        std::string helperId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "helper") {
                helperId = c->id;
                break;
            }
        }
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", helperId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "placeholder helper"},
                                       {"category", "initialization"}}}}}});

        rpc(state, "agent", "saveAnnotatedAST", {{"path", "helper.py"}});

        // List files — should show both funcs.py and helper.py
        json listResp = rpc(state, "agent", "listAnnotatedFiles");
        int fileCount = listResp["result"].value("count", 0);

        expect(fileCount >= 2,
               "listAnnotatedFiles returns " + std::to_string(fileCount) +
               " sidecar files for multi-file project",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: All 5 annotation types in compact AST semantic field
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        std::string src3 = "def compute(x):\n    return x * 2\n";
        rpc(state, "agent", "openFile",
            {{"path", "compute.py"}, {"content", src3}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "compute") {
                fnId = c->id;
                break;
            }
        }

        // Add all 5 annotation types
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "doubles input"},
                                       {"category", "computation"}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ComplexityAnnotation"},
                       {"properties", {{"timeComplexity", "O(1)"},
                                       {"cognitiveComplexity", 1},
                                       {"linesOfLogic", 1}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "RiskAnnotation"},
                       {"properties", {{"level", "low"},
                                       {"reason", "pure function"},
                                       {"dependentCount", 0}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ContractAnnotation"},
                       {"properties", {{"preconditions", "x is numeric"},
                                       {"postconditions", "returns x*2"},
                                       {"returnShape", "number"},
                                       {"sideEffects", "none"}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "SemanticTagAnnotation"},
                       {"properties", {{"tags", json::array({"@pure", "@math"})}}}}}});

        json resp = rpc(state, "agent", "getAST", {{"compact", true}});
        json nodes = resp["result"]["nodes"];

        bool allFields = false;
        for (const auto& n : nodes) {
            if (n.value("name", "") == "compute" && n.contains("semantic")) {
                auto sem = n["semantic"];
                allFields = sem.contains("intent") &&
                            sem.contains("complexity") &&
                            sem.contains("risk") &&
                            sem.contains("contract") &&
                            sem.contains("tags");
            }
        }

        expect(allFields,
               "Compact AST has all 5 semantic annotation fields for compute()",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: Sidecar roundtrip preserves all 5 annotation types
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        std::string src3 = "def compute(x):\n    return x * 2\n";
        rpc(state, "agent", "openFile",
            {{"path", "compute.py"}, {"content", src3}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "compute") {
                fnId = c->id;
                break;
            }
        }

        // Add all 5 types
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "IntentAnnotation"},
                       {"properties", {{"summary", "doubles"},
                                       {"category", "math"}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ComplexityAnnotation"},
                       {"properties", {{"timeComplexity", "O(1)"},
                                       {"cognitiveComplexity", 1},
                                       {"linesOfLogic", 1}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "RiskAnnotation"},
                       {"properties", {{"level", "low"},
                                       {"reason", "pure"},
                                       {"dependentCount", 0}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "ContractAnnotation"},
                       {"properties", {{"preconditions", "x numeric"},
                                       {"postconditions", "returns x*2"},
                                       {"returnShape", "number"},
                                       {"sideEffects", "none"}}}}}});
        rpc(state, "agent", "applyMutation",
            {{"type", "insertNode"}, {"parentId", fnId},
             {"role", "annotations"},
             {"node", {{"concept", "SemanticTagAnnotation"},
                       {"properties", {{"tags", json::array({"@pure"})}}}}}});

        // Save sidecar
        rpc(state, "agent", "saveAnnotatedAST", {{"path", "compute.py"}});

        // Load into fresh state
        HeadlessEditorState state2;
        state2.defaultLanguage = "python";
        state2.workspaceRoot = tmpDir.string();
        state2.setAgentRole("agent", AgentRole::Refactor);
        rpc(state2, "agent", "openFile",
            {{"path", "compute.py"}, {"content", src3}});
        rpc(state2, "agent", "loadAnnotatedAST", {{"path", "compute.py"}});

        // Count annotation types
        int count = 0;
        bool hasIntent = false, hasComplexity = false, hasRisk = false,
             hasContract = false, hasTags = false;
        for (auto* c : state2.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "compute") {
                for (auto* a : c->getChildren("annotations")) {
                    if (a->conceptType == "IntentAnnotation") { hasIntent = true; ++count; }
                    if (a->conceptType == "ComplexityAnnotation") { hasComplexity = true; ++count; }
                    if (a->conceptType == "RiskAnnotation") { hasRisk = true; ++count; }
                    if (a->conceptType == "ContractAnnotation") { hasContract = true; ++count; }
                    if (a->conceptType == "SemanticTagAnnotation") { hasTags = true; ++count; }
                }
            }
        }

        expect(count == 5 && hasIntent && hasComplexity && hasRisk &&
               hasContract && hasTags,
               "Sidecar roundtrip preserves all 5 annotation types (" +
               std::to_string(count) + "/5)",
               passed, failed);
    }

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "\n=== Step 268 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
