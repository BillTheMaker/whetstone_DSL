// Step 271 TDD Test: Phase 10b Integration Tests
//
// Full agent annotation session: discover unannotated → set annotations →
// query → remove → re-query → save sidecar. Tests multi-function workflow,
// prompt template content, permission enforcement, and hint accuracy.
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
                  "whetstone_step271_test";
    std::filesystem::remove_all(tmpDir);
    std::filesystem::create_directories(tmpDir);

    std::string src =
        "def validate(data):\n"
        "    if not data:\n"
        "        return False\n"
        "    return True\n"
        "\n"
        "def transform(items):\n"
        "    result = []\n"
        "    for item in items:\n"
        "        result.append(item * 2)\n"
        "    return result\n"
        "\n"
        "def notify(user):\n"
        "    print(\"hello \" + user)\n";

    // ---------------------------------------------------------------
    //  Test 1: Full annotation workflow — discover → annotate → query
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "app.py"}, {"content", src}});

        // Step 1: Discover unannotated functions
        json unannotated = rpc(state, "agent", "getUnannotatedNodes",
                                {{"includeHints", true}});
        int initialCount = unannotated["result"].value("count", 0);

        // Step 2: Get function IDs
        std::string validateId, transformId, notifyId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                std::string n = getNodeName(c);
                if (n == "validate") validateId = c->id;
                if (n == "transform") transformId = c->id;
                if (n == "notify") notifyId = c->id;
            }
        }

        // Step 3: Annotate all 3 functions with intent
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "intent"},
             {"fields", {{"summary", "validates input data"},
                         {"category", "validation"}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", transformId}, {"type", "intent"},
             {"fields", {{"summary", "doubles each item"},
                         {"category", "transformation"}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", notifyId}, {"type", "intent"},
             {"fields", {{"summary", "prints greeting"},
                         {"category", "io"}}}});

        // Step 4: Query — all should be annotated now
        json afterAnnotate = rpc(state, "agent", "getUnannotatedNodes");
        int afterCount = afterAnnotate["result"].value("count", 0);

        // Step 5: Query all annotations project-wide
        json allAnnotations = rpc(state, "agent", "getSemanticAnnotations");
        int nodeCount = (int)allAnnotations["result"]["nodes"].size();

        expect(initialCount == 3 && afterCount == 0 && nodeCount == 3,
               "Full workflow: discover " + std::to_string(initialCount) +
               " → annotate → " + std::to_string(afterCount) +
               " unannotated, " + std::to_string(nodeCount) + " annotated nodes",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Remove and re-query workflow
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "app.py"}, {"content", src}});

        std::string validateId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" && getNodeName(c) == "validate") {
                validateId = c->id;
                break;
            }
        }

        // Add intent + risk
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "intent"},
             {"fields", {{"summary", "validates"}, {"category", "validation"}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "risk"},
             {"fields", {{"level", "medium"}, {"reason", "multiple callers"},
                         {"dependentCount", 5}}}});

        // Verify 2 annotations
        json before = rpc(state, "agent", "getSemanticAnnotations",
                          {{"nodeId", validateId}});
        int beforeCount = (int)before["result"]["annotations"].size();

        // Remove risk
        rpc(state, "agent", "removeSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "risk"}});

        // Re-query: should have 1
        json after = rpc(state, "agent", "getSemanticAnnotations",
                         {{"nodeId", validateId}});
        int afterCount = (int)after["result"]["annotations"].size();

        // getUnannotatedNodes for risk should include validate now
        json unanRisk = rpc(state, "agent", "getUnannotatedNodes",
                            {{"type", "risk"}});
        bool validateMissingRisk = false;
        for (const auto& n : unanRisk["result"]["nodes"]) {
            if (n.value("name", "") == "validate")
                validateMissingRisk = true;
        }

        expect(beforeCount == 2 && afterCount == 1 && validateMissingRisk,
               "Remove risk → re-query shows 2→1, validate needs risk again",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Annotation session with sidecar save/load roundtrip
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "roundtrip.py"}, {"content", src}});

        // Annotate via RPC
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType != "Function") continue;
            std::string name = getNodeName(c);
            rpc(state, "agent", "setSemanticAnnotation",
                {{"nodeId", c->id}, {"type", "intent"},
                 {"fields", {{"summary", name + " function"},
                             {"category", "general"}}}});
            rpc(state, "agent", "setSemanticAnnotation",
                {{"nodeId", c->id}, {"type", "complexity"},
                 {"fields", {{"timeComplexity", "O(n)"},
                             {"cognitiveComplexity", 3},
                             {"linesOfLogic", 4}}}});
        }

        // Save sidecar
        json saveResp = rpc(state, "agent", "saveAnnotatedAST",
                            {{"path", "roundtrip.py"}});
        int savedCount = saveResp["result"].value("annotationCount", 0);

        // Load into fresh state
        HeadlessEditorState state2;
        state2.defaultLanguage = "python";
        state2.workspaceRoot = tmpDir.string();
        state2.setAgentRole("agent", AgentRole::Refactor);
        rpc(state2, "agent", "openFile",
            {{"path", "roundtrip.py"}, {"content", src}});
        rpc(state2, "agent", "loadAnnotatedAST", {{"path", "roundtrip.py"}});

        // Query via RPC in new state
        json allAnnotations = rpc(state2, "agent", "getSemanticAnnotations");
        int restoredNodes = (int)allAnnotations["result"]["nodes"].size();

        expect(savedCount >= 6 && restoredNodes == 3,
               "Sidecar roundtrip: saved " + std::to_string(savedCount) +
               " annotations, restored " + std::to_string(restoredNodes) +
               " annotated nodes",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Permission enforcement across the pipeline
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("admin", AgentRole::Refactor);
        state.setAgentRole("reader", AgentRole::Linter);

        rpc(state, "admin", "openFile",
            {{"path", "perms.py"}, {"content", src}});

        std::string fnId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                fnId = c->id;
                break;
            }
        }

        // Admin can set annotation
        json setResp = rpc(state, "admin", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"},
             {"fields", {{"summary", "test"}, {"category", "io"}}}});
        bool adminSetOk = setResp.contains("result");

        // Reader cannot set
        json readerSet = rpc(state, "reader", "setSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "risk"},
             {"fields", {{"level", "low"}, {"reason", "test"},
                         {"dependentCount", 0}}}});
        bool readerSetDenied = readerSet.contains("error");

        // Reader cannot remove
        json readerRemove = rpc(state, "reader", "removeSemanticAnnotation",
            {{"nodeId", fnId}, {"type", "intent"}});
        bool readerRemoveDenied = readerRemove.contains("error");

        // Reader CAN query
        json readerGet = rpc(state, "reader", "getSemanticAnnotations",
                             {{"nodeId", fnId}});
        bool readerGetOk = readerGet.contains("result");

        // Reader CAN get unannotated
        json readerUnan = rpc(state, "reader", "getUnannotatedNodes");
        bool readerUnanOk = readerUnan.contains("result");

        expect(adminSetOk && readerSetDenied && readerRemoveDenied &&
               readerGetOk && readerUnanOk,
               "Linter: denied set/remove, allowed get/getUnannotated",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Hint accuracy — sideEffectHint for io function
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "hints.py"}, {"content", src}});

        json resp = rpc(state, "agent", "getUnannotatedNodes",
                        {{"includeHints", true}});

        // notify() calls print(), so sideEffectHint should be "possible"
        std::string notifyHint;
        // validate() is pure, so sideEffectHint should be "none"
        std::string validateHint;
        for (const auto& n : resp["result"]["nodes"]) {
            if (n.value("name", "") == "notify" && n.contains("hints"))
                notifyHint = n["hints"].value("sideEffectHint", "");
            if (n.value("name", "") == "validate" && n.contains("hints"))
                validateHint = n["hints"].value("sideEffectHint", "");
        }

        expect(notifyHint == "possible" && validateHint == "none",
               "Hint accuracy: notify='" + notifyHint +
               "' validate='" + validateHint + "'",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Prompt template content validates workflow steps
    // ---------------------------------------------------------------
    {
        MCPServer mcp;

        // Each annotation prompt should have non-empty messages
        std::vector<std::string> promptNames = {
            "annotate_intent", "annotate_complexity",
            "annotate_risk", "annotate_contracts", "annotate_full"
        };

        int validPrompts = 0;
        for (const auto& pname : promptNames) {
            json req = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "prompts/get"},
                        {"params", {{"name", pname},
                                    {"arguments", json::object()}}}};
            json resp = mcp.handleRequest(req);
            if (resp.contains("result") &&
                resp["result"].contains("messages") &&
                !resp["result"]["messages"].empty()) {
                std::string text = resp["result"]["messages"][0]
                                   ["content"].value("text", "");
                if (text.size() > 50) ++validPrompts;
            }
        }

        expect(validPrompts == 5,
               "All 5 prompt templates return substantive instructions (" +
               std::to_string(validPrompts) + "/5)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Multi-type annotation then compact AST verification
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "compact.py"}, {"content", src}});

        std::string transformId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "transform") {
                transformId = c->id;
                break;
            }
        }

        // Set 3 annotation types via RPC
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", transformId}, {"type", "intent"},
             {"fields", {{"summary", "doubles items"},
                         {"category", "transformation"}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", transformId}, {"type", "complexity"},
             {"fields", {{"timeComplexity", "O(n)"},
                         {"cognitiveComplexity", 2},
                         {"linesOfLogic", 4}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", transformId}, {"type", "tags"},
             {"fields", {{"tags", json::array({"@collection", "@pure"})}}}});

        // Verify compact AST shows all 3 in semantic field
        json resp = rpc(state, "agent", "getAST", {{"compact", true}});
        json nodes = resp["result"]["nodes"];

        bool allPresent = false;
        for (const auto& n : nodes) {
            if (n.value("name", "") == "transform" &&
                n.contains("semantic")) {
                auto sem = n["semantic"];
                allPresent = sem.contains("intent") &&
                             sem.contains("complexity") &&
                             sem.contains("tags");
            }
        }

        expect(allPresent,
               "Compact AST reflects 3 annotation types set via RPC",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: End-to-end: RPC annotate → save → load → RPC query
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.workspaceRoot = tmpDir.string();
        state.setAgentRole("agent", AgentRole::Refactor);

        rpc(state, "agent", "openFile",
            {{"path", "e2e.py"}, {"content", src}});

        // Annotate validate with all 5 types via RPC
        std::string validateId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "validate") {
                validateId = c->id;
                break;
            }
        }

        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "intent"},
             {"fields", {{"summary", "validates input"},
                         {"category", "validation"}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "complexity"},
             {"fields", {{"timeComplexity", "O(1)"},
                         {"cognitiveComplexity", 2},
                         {"linesOfLogic", 3}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "risk"},
             {"fields", {{"level", "high"}, {"reason", "12 callers"},
                         {"dependentCount", 12}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "contract"},
             {"fields", {{"preconditions", "data not null"},
                         {"postconditions", "returns bool"},
                         {"returnShape", "bool"},
                         {"sideEffects", "none"}}}});
        rpc(state, "agent", "setSemanticAnnotation",
            {{"nodeId", validateId}, {"type", "tags"},
             {"fields", {{"tags", json::array({"@validation", "@guard"})}}}});

        // Save sidecar
        rpc(state, "agent", "saveAnnotatedAST", {{"path", "e2e.py"}});

        // Load into fresh state and query via RPC
        HeadlessEditorState state2;
        state2.defaultLanguage = "python";
        state2.workspaceRoot = tmpDir.string();
        state2.setAgentRole("agent", AgentRole::Refactor);
        rpc(state2, "agent", "openFile",
            {{"path", "e2e.py"}, {"content", src}});
        rpc(state2, "agent", "loadAnnotatedAST", {{"path", "e2e.py"}});

        // Find validate in new state
        std::string newValidateId;
        for (auto* c : state2.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "validate") {
                newValidateId = c->id;
                break;
            }
        }

        json resp = rpc(state2, "agent", "getSemanticAnnotations",
                        {{"nodeId", newValidateId}});
        int annoCount = (int)resp["result"]["annotations"].size();

        // Check all 5 types present
        bool hasI = false, hasCx = false, hasR = false,
             hasCt = false, hasT = false;
        for (const auto& a : resp["result"]["annotations"]) {
            std::string t = a.value("type", "");
            if (t == "intent") hasI = true;
            if (t == "complexity") hasCx = true;
            if (t == "risk") hasR = true;
            if (t == "contract") hasCt = true;
            if (t == "tags") hasT = true;
        }

        expect(annoCount == 5 && hasI && hasCx && hasR && hasCt && hasT,
               "E2E: RPC annotate 5 types → save → load → RPC query (" +
               std::to_string(annoCount) + "/5)",
               passed, failed);
    }

    // Cleanup
    std::filesystem::remove_all(tmpDir);

    std::cout << "\n=== Step 271 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
