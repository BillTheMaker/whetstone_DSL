// Step 269 TDD Test: Annotation RPC Methods
//
// High-level RPC methods for agents to set/get/remove semantic annotations
// without needing low-level applyMutation(insertNode) calls.
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
        "    return \"hello \" + name\n"
        "\n"
        "def farewell(name):\n"
        "    return \"goodbye \" + name\n";

    // ---------------------------------------------------------------
    //  Test 1: setSemanticAnnotation adds intent annotation
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets user"},
                         {"category", "io"}}}});

        bool success = resp.contains("result") &&
                       resp["result"].value("success", false);

        // Verify annotation exists
        bool found = false;
        ASTNode* fn = findNodeById(state.activeAST(), fnId);
        if (fn) {
            for (auto* a : fn->getChildren("annotations")) {
                if (a->conceptType == "IntentAnnotation") {
                    auto* ia = static_cast<IntentAnnotation*>(a);
                    if (ia->summary == "greets user") found = true;
                }
            }
        }

        expect(success && found,
               "setSemanticAnnotation adds IntentAnnotation",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: setSemanticAnnotation updates existing (no duplicates)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Set intent twice with different values
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets v1"}, {"category", "io"}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets v2"}, {"category", "io"}}}});

        // Should have exactly 1 IntentAnnotation with "greets v2"
        int count = 0;
        std::string lastSummary;
        ASTNode* fn = findNodeById(state.activeAST(), fnId);
        if (fn) {
            for (auto* a : fn->getChildren("annotations")) {
                if (a->conceptType == "IntentAnnotation") {
                    ++count;
                    lastSummary = static_cast<IntentAnnotation*>(a)->summary;
                }
            }
        }

        expect(count == 1 && lastSummary == "greets v2",
               "setSemanticAnnotation updates existing, no duplicates",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: setSemanticAnnotation for all 5 types
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets"}, {"category", "io"}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "complexity"},
             {"fields", {{"timeComplexity", "O(1)"},
                         {"cognitiveComplexity", 1},
                         {"linesOfLogic", 2}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "risk"},
             {"fields", {{"level", "low"}, {"reason", "simple"},
                         {"dependentCount", 2}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "contract"},
             {"fields", {{"preconditions", "name is str"},
                         {"postconditions", "returns greeting"},
                         {"returnShape", "str"},
                         {"sideEffects", "none"}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "tags"},
             {"fields", {{"tags", json::array({"@io", "@greeting"})}}}});

        int count = 0;
        ASTNode* fn = findNodeById(state.activeAST(), fnId);
        if (fn) {
            for (auto* a : fn->getChildren("annotations")) {
                if (isSemanticAnnotation(a->conceptType))
                    ++count;
            }
        }

        expect(count == 5,
               "setSemanticAnnotation works for all 5 types (" +
               std::to_string(count) + "/5)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: getSemanticAnnotations on a specific node
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets user"}, {"category", "io"}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "risk"},
             {"fields", {{"level", "low"}, {"reason", "trivial"},
                         {"dependentCount", 0}}}});

        json resp = rpc(state, "s1", "getSemanticAnnotations",
                        {{"nodeId", fnId}});
        bool hasResult = resp.contains("result");
        int annoCount = 0;
        bool hasIntent = false, hasRisk = false;
        if (hasResult && resp["result"].contains("annotations")) {
            auto annos = resp["result"]["annotations"];
            annoCount = (int)annos.size();
            for (const auto& a : annos) {
                if (a.value("type", "") == "intent") hasIntent = true;
                if (a.value("type", "") == "risk") hasRisk = true;
            }
        }

        expect(hasResult && annoCount == 2 && hasIntent && hasRisk,
               "getSemanticAnnotations returns 2 annotations on specific node",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: getSemanticAnnotations with no nodeId returns all
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string greetId, farewellId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                if (getNodeName(c) == "greet") greetId = c->id;
                if (getNodeName(c) == "farewell") farewellId = c->id;
            }
        }

        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", greetId}, {"type", "intent"},
             {"fields", {{"summary", "greets"}, {"category", "io"}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", farewellId}, {"type", "intent"},
             {"fields", {{"summary", "says goodbye"}, {"category", "io"}}}});

        json resp = rpc(state, "s1", "getSemanticAnnotations");
        bool hasResult = resp.contains("result");
        int nodeCount = 0;
        if (hasResult && resp["result"].contains("nodes"))
            nodeCount = (int)resp["result"]["nodes"].size();

        expect(hasResult && nodeCount >= 2,
               "getSemanticAnnotations (no nodeId) returns " +
               std::to_string(nodeCount) + " annotated nodes",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: removeSemanticAnnotation removes specific type
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Add two annotations
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets"}, {"category", "io"}}}});
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "risk"},
             {"fields", {{"level", "low"}, {"reason", "simple"},
                         {"dependentCount", 0}}}});

        // Remove intent
        json resp = rpc(state, "s1", "removeSemanticAnnotation",
                        {{"nodeId", fnId}, {"type", "intent"}});
        bool success = resp.contains("result") &&
                       resp["result"].value("removed", false);

        // Check only risk remains
        int intentCount = 0, riskCount = 0;
        ASTNode* fn = findNodeById(state.activeAST(), fnId);
        if (fn) {
            for (auto* a : fn->getChildren("annotations")) {
                if (a->conceptType == "IntentAnnotation") ++intentCount;
                if (a->conceptType == "RiskAnnotation") ++riskCount;
            }
        }

        expect(success && intentCount == 0 && riskCount == 1,
               "removeSemanticAnnotation removes intent, keeps risk",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: getUnannotatedNodes returns functions without annotations
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        // Before annotations: both should be unannotated
        json resp = rpc(state, "s1", "getUnannotatedNodes");
        int countBefore = 0;
        if (resp.contains("result") && resp["result"].contains("nodes"))
            countBefore = (int)resp["result"]["nodes"].size();

        // Annotate greet
        std::string greetId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                greetId = c->id;
                break;
            }
        }
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", greetId}, {"type", "intent"},
             {"fields", {{"summary", "greets"}, {"category", "io"}}}});

        // After: only farewell should be unannotated
        json resp2 = rpc(state, "s1", "getUnannotatedNodes");
        int countAfter = 0;
        if (resp2.contains("result") && resp2["result"].contains("nodes"))
            countAfter = (int)resp2["result"]["nodes"].size();

        expect(countBefore >= 2 && countAfter == countBefore - 1,
               "getUnannotatedNodes: " + std::to_string(countBefore) +
               " before, " + std::to_string(countAfter) + " after annotating greet",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: getUnannotatedNodes with type filter
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string greetId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                greetId = c->id;
                break;
            }
        }

        // Add only intent to greet
        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", greetId}, {"type", "intent"},
             {"fields", {{"summary", "greets"}, {"category", "io"}}}});

        // getUnannotatedNodes for "risk" should include greet
        json resp = rpc(state, "s1", "getUnannotatedNodes",
                        {{"type", "risk"}});
        int count = 0;
        bool greetIncluded = false;
        if (resp.contains("result") && resp["result"].contains("nodes")) {
            count = (int)resp["result"]["nodes"].size();
            for (const auto& n : resp["result"]["nodes"]) {
                if (n.value("name", "") == "greet") greetIncluded = true;
            }
        }

        expect(count >= 2 && greetIncluded,
               "getUnannotatedNodes(type=risk) includes greet (has intent but not risk)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Linter cannot set/remove annotations
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("linter", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        // Linter cannot set
        json resp1 = rpc(state, "linter", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "test"}, {"category", "io"}}}});
        bool setDenied = resp1.contains("error");

        // Linter CAN get
        json resp2 = rpc(state, "linter", "getSemanticAnnotations",
                         {{"nodeId", fnId}});
        bool getOk = resp2.contains("result");

        // Linter CAN get unannotated
        json resp3 = rpc(state, "linter", "getUnannotatedNodes");
        bool getUnOk = resp3.contains("result");

        expect(setDenied && getOk && getUnOk,
               "Linter denied set, allowed get/getUnannotated",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: 4 MCP tools registered for annotation operations
    // ---------------------------------------------------------------
    {
        MCPServer mcp;
        bool foundSet = false, foundGet = false,
             foundRemove = false, foundUnannotated = false;
        for (const auto& t : mcp.getTools()) {
            if (t.name == "whetstone_set_semantic_annotation") foundSet = true;
            if (t.name == "whetstone_get_semantic_annotations") foundGet = true;
            if (t.name == "whetstone_remove_semantic_annotation") foundRemove = true;
            if (t.name == "whetstone_get_unannotated_nodes") foundUnannotated = true;
        }
        expect(foundSet && foundGet && foundRemove && foundUnannotated,
               "4 annotation MCP tools registered",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: setSemanticAnnotation on invalid node returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        json resp = rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", "nonexistent_node"}, {"type", "intent"},
             {"fields", {{"summary", "test"}, {"category", "io"}}}});
        bool hasError = resp.contains("error");

        expect(hasError,
               "setSemanticAnnotation on invalid nodeId returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: getSemanticAnnotations response format
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("s1", AgentRole::Refactor);

        rpc(state, "s1", "openFile",
            {{"path", "t.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "greet") {
                fnId = c->id;
                break;
            }
        }

        rpc(state, "s1", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "greets user"}, {"category", "io"}}}});

        json resp = rpc(state, "s1", "getSemanticAnnotations",
                        {{"nodeId", fnId}});
        bool hasAnnotations = resp.contains("result") &&
                              resp["result"].contains("annotations");
        bool hasCorrectFields = false;
        if (hasAnnotations) {
            for (const auto& a : resp["result"]["annotations"]) {
                if (a.value("type", "") == "intent") {
                    hasCorrectFields =
                        a.contains("summary") &&
                        a.value("summary", "") == "greets user" &&
                        a.contains("category") &&
                        a.value("category", "") == "io";
                }
            }
        }

        expect(hasAnnotations && hasCorrectFields,
               "getSemanticAnnotations returns structured annotation fields",
               passed, failed);
    }

    std::cout << "\n=== Step 269 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
