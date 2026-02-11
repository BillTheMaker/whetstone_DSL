// Step 251 TDD Test: Quick-Fix Actions as RPC Mutations
//
// Tests getQuickFixes and applyQuickFix RPC methods: fix discovery,
// mutation application, diagnostic clearing, and MCP tool registration.
#include "HeadlessEditorState.h"
#include "MCPBridge.h"

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

// Helper: set up state with a function that has a bad annotation
static void setupAnnotatedState(HeadlessEditorState& state,
                                 const std::string& bufName) {
    state.defaultLanguage = "python";
    state.openBuffer(bufName,
        "def cleanup(ptr):\n    x = ptr\n", "python");
    state.setAgentRole("s1", AgentRole::Refactor);

    Module* ast = state.activeAST();
    for (auto* c : ast->allChildren()) {
        if (c->conceptType == "Function") {
            auto* anno = new DeallocateAnnotation();
            c->addChild("annotations", anno);
            break;
        }
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    // ---------------------------------------------------------------
    //  Test 1: getQuickFixes returns fixes for annotation diagnostic
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t1.py");

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getQuickFixes"},
                        {"params", json::object()}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasFixes = resp.contains("result") &&
            resp["result"].contains("fixes") &&
            resp["result"]["fixes"].is_array();
        int count = 0;
        if (resp.contains("result"))
            count = resp["result"].value("count", 0);
        expect(hasFixes && count > 0,
               "getQuickFixes returns fixes (count=" +
               std::to_string(count) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: QuickFix has required fields
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t2.py");

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getQuickFixes"},
                        {"params", json::object()}};
        json resp = state.processAgentRequest(request, "s1");

        bool valid = false;
        if (resp.contains("result") &&
            !resp["result"]["fixes"].empty()) {
            const auto& f = resp["result"]["fixes"][0];
            valid = f.contains("id") && f.contains("diagCode") &&
                    f.contains("nodeId") && f.contains("description") &&
                    f.contains("category") && f.contains("mutation");
        }
        expect(valid,
               "QuickFix has id, diagCode, nodeId, description, category, mutation",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: QuickFix mutation is a concrete mutation object
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t3.py");

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getQuickFixes"},
                        {"params", json::object()}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasMutType = false;
        if (resp.contains("result") &&
            !resp["result"]["fixes"].empty()) {
            const auto& mut = resp["result"]["fixes"][0]["mutation"];
            hasMutType = mut.contains("type");
        }
        expect(hasMutType,
               "QuickFix mutation contains 'type' field",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: getQuickFixes with nodeId filter
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t4.py");

        // Find the function nodeId
        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") { funcId = c->id; break; }
        }

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getQuickFixes"},
                        {"params", {{"nodeId", funcId}}}};
        json resp = state.processAgentRequest(request, "s1");

        bool allMatch = true;
        int count = 0;
        if (resp.contains("result")) {
            count = resp["result"].value("count", 0);
            for (const auto& f : resp["result"]["fixes"]) {
                if (f.value("nodeId", "") != funcId)
                    allMatch = false;
            }
        }
        expect(count > 0 && allMatch,
               "getQuickFixes with nodeId returns fixes for that node only",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: applyQuickFix applies the mutation
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t5.py");

        // Find the function nodeId and its annotation
        std::string funcId;
        std::string annoId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                funcId = c->id;
                for (auto* a : c->getChildren("annotations")) {
                    if (a->conceptType == "DeallocateAnnotation") {
                        annoId = a->id;
                        break;
                    }
                }
                break;
            }
        }

        // The fix for E0200 is deleteNode on the function node
        // (where the annotation is attached)
        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "applyQuickFix"},
                        {"params", {{"diagCode", "E0200"},
                                    {"nodeId", funcId}}}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasResult = resp.contains("result");
        bool hasFixId = hasResult &&
            resp["result"].contains("fixApplied");
        expect(hasResult && hasFixId,
               "applyQuickFix returns result with fixApplied",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: applyQuickFix reports diagnostic status
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t6.py");

        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") { funcId = c->id; break; }
        }

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "applyQuickFix"},
                        {"params", {{"diagCode", "E0200"},
                                    {"nodeId", funcId}}}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasCleared = resp.contains("result") &&
            resp["result"].contains("diagnosticCleared");
        bool hasRemaining = resp.contains("result") &&
            resp["result"].contains("remainingDiagnostics");
        expect(hasCleared && hasRemaining,
               "applyQuickFix reports diagnosticCleared and remainingDiagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: applyQuickFix with invalid diagCode returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t7.py");

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "applyQuickFix"},
                        {"params", {{"diagCode", "E9999"},
                                    {"nodeId", "nonexistent"}}}};
        json resp = state.processAgentRequest(request, "s1");

        bool isErr = resp.contains("error");
        expect(isErr,
               "applyQuickFix with invalid diagCode returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: applyQuickFix requires both diagCode and nodeId
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t8.py");

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "applyQuickFix"},
                        {"params", {{"diagCode", "E0200"}}}};
        json resp = state.processAgentRequest(request, "s1");

        bool isErr = resp.contains("error") &&
            resp["error"].value("code", 0) == -32602;
        expect(isErr,
               "applyQuickFix without nodeId returns -32602 error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Linter role can getQuickFixes but not applyQuickFix
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t9.py");
        state.setAgentRole("linter", AgentRole::Linter);

        // getQuickFixes should work for Linter
        json getReq = {{"jsonrpc", "2.0"}, {"id", 1},
                       {"method", "getQuickFixes"},
                       {"params", json::object()}};
        json getResp = state.processAgentRequest(getReq, "linter");
        bool canGet = getResp.contains("result");

        // applyQuickFix should be denied for Linter
        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") { funcId = c->id; break; }
        }
        json applyReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "applyQuickFix"},
                         {"params", {{"diagCode", "E0200"},
                                     {"nodeId", funcId}}}};
        json applyResp = state.processAgentRequest(applyReq, "linter");
        bool denied = applyResp.contains("error") &&
            applyResp["error"].value("code", 0) == -32031;

        expect(canGet && denied,
               "Linter can getQuickFixes but not applyQuickFix",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: MCP tools/list includes quick fix tools (20 total)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("mcp.py", "x = 1\n", "python");
        state.setAgentRole("mcp-session", AgentRole::Refactor);

        MCPBridge bridge;
        bridge.setRequestHandler([&state](const json& req) -> json {
            return state.processAgentRequest(req, "mcp-session");
        });

        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "tools/list"}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);

        bool hasGetFixes = false, hasApplyFix = false;
        int toolCount = 0;
        if (r.contains("result") && r["result"].contains("tools")) {
            toolCount = (int)r["result"]["tools"].size();
            for (const auto& t : r["result"]["tools"]) {
                std::string name = t.value("name", "");
                if (name == "whetstone_get_quick_fixes") hasGetFixes = true;
                if (name == "whetstone_apply_quick_fix") hasApplyFix = true;
            }
        }
        expect(hasGetFixes && hasApplyFix && toolCount >= 20,
               "MCP tools/list includes quick fix tools (total " +
               std::to_string(toolCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: Function without return gets missing-return fix
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("noret.py",
            "def compute(x):\n    y = x * 2\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Find the function nodeId
        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") { funcId = c->id; break; }
        }

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getQuickFixes"},
                        {"params", {{"nodeId", funcId}}}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasMissingReturn = false;
        if (resp.contains("result")) {
            for (const auto& f : resp["result"]["fixes"]) {
                if (f.value("category", "") == "missing-return")
                    hasMissingReturn = true;
            }
        }
        expect(hasMissingReturn,
               "Function without return gets missing-return fix suggestion",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: QuickFix category matches diagnostic type
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        setupAnnotatedState(state, "t12.py");

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getQuickFixes"},
                        {"params", json::object()}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasCategory = false;
        if (resp.contains("result") &&
            !resp["result"]["fixes"].empty()) {
            for (const auto& f : resp["result"]["fixes"]) {
                std::string cat = f.value("category", "");
                if (cat == "remove-annotation" ||
                    cat == "change-strategy" ||
                    cat == "resolve-conflict" ||
                    cat == "remove-use-after-free" ||
                    cat == "add-deallocation" ||
                    cat == "remove-unused" ||
                    cat == "missing-return" ||
                    cat == "general") {
                    hasCategory = true;
                }
            }
        }
        expect(hasCategory,
               "QuickFix has recognized category",
               passed, failed);
    }

    std::cout << "\n=== Step 251 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
