// Step 252 TDD Test: Diagnostic Delta Streaming
//
// Tests getDiagnosticsDelta RPC method: version tracking, added/removed
// diagnostics after mutations, efficient delta computation, and MCP tool.
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

int main() {
    int passed = 0;
    int failed = 0;

    // ---------------------------------------------------------------
    //  Test 1: getDiagnostics returns version number
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("v.py",
            "def add(a, b):\n    return a + b\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "getDiagnostics"},
                    {"params", json::object()}};
        json resp = state.processAgentRequest(req, "s1");

        bool hasVersion = resp.contains("result") &&
            resp["result"].contains("version");
        int ver = 0;
        if (hasVersion) ver = resp["result"].value("version", -1);
        expect(hasVersion && ver >= 1,
               "getDiagnostics returns version >= 1 (got " +
               std::to_string(ver) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: getDiagnosticsDelta with same version → empty delta
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("d.py",
            "def add(a, b):\n    return a + b\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Get initial diagnostics to set version
        json diagReq = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getDiagnostics"},
                        {"params", json::object()}};
        json diagResp = state.processAgentRequest(diagReq, "s1");
        int ver = diagResp["result"].value("version", 0);

        // Get delta with same version → no changes
        json deltaReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "getDiagnosticsDelta"},
                         {"params", {{"sinceVersion", ver}}}};
        json deltaResp = state.processAgentRequest(deltaReq, "s1");

        bool hasResult = deltaResp.contains("result");
        int addedCount = 0, removedCount = 0;
        if (hasResult) {
            addedCount = deltaResp["result"].value("addedCount", -1);
            removedCount = deltaResp["result"].value("removedCount", -1);
        }
        expect(hasResult && addedCount == 0 && removedCount == 0,
               "Delta with current version returns empty (added=0, removed=0)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Introduce annotation error → delta shows addition
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("intro.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Get initial diagnostics (clean state)
        json req1 = {{"jsonrpc", "2.0"}, {"id", 1},
                     {"method", "getDiagnostics"},
                     {"params", json::object()}};
        json resp1 = state.processAgentRequest(req1, "s1");
        int ver1 = resp1["result"].value("version", 0);
        int count1 = resp1["result"].value("count", 0);

        // Add annotation to introduce an error
        Module* ast = state.activeAST();
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") {
                auto* anno = new DeallocateAnnotation();
                c->addChild("annotations", anno);
                break;
            }
        }

        // Get delta → should show additions
        json deltaReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "getDiagnosticsDelta"},
                         {"params", {{"sinceVersion", ver1}}}};
        json deltaResp = state.processAgentRequest(deltaReq, "s1");

        int addedCount = 0;
        if (deltaResp.contains("result"))
            addedCount = deltaResp["result"].value("addedCount", 0);
        expect(addedCount > 0,
               "Delta after introducing error shows additions (added=" +
               std::to_string(addedCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Fix annotation error → delta shows removal
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("fix.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Add annotation to create an error
        Module* ast = state.activeAST();
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        // Get diagnostics with the error (establishes baseline)
        json req1 = {{"jsonrpc", "2.0"}, {"id", 1},
                     {"method", "getDiagnostics"},
                     {"params", json::object()}};
        json resp1 = state.processAgentRequest(req1, "s1");
        int ver1 = resp1["result"].value("version", 0);
        int errCount = resp1["result"].value("count", 0);

        // Remove the annotation to fix the error
        if (fn) {
            auto annos = fn->getChildren("annotations");
            for (auto* a : annos) {
                if (a->conceptType == "DeallocateAnnotation") {
                    fn->removeChild(a);
                    delete a;
                    break;
                }
            }
        }

        // Get delta → should show removals
        json deltaReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "getDiagnosticsDelta"},
                         {"params", {{"sinceVersion", ver1}}}};
        json deltaResp = state.processAgentRequest(deltaReq, "s1");

        int removedCount = 0;
        if (deltaResp.contains("result"))
            removedCount = deltaResp["result"].value("removedCount", 0);
        expect(errCount > 0 && removedCount > 0,
               "Delta after fix shows removals (had=" +
               std::to_string(errCount) + " removed=" +
               std::to_string(removedCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Delta response has correct structure
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("struct.py",
            "def foo():\n    pass\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "getDiagnostics"},
                    {"params", json::object()}};
        state.processAgentRequest(req, "s1");

        json deltaReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "getDiagnosticsDelta"},
                         {"params", {{"sinceVersion", 0}}}};
        json resp = state.processAgentRequest(deltaReq, "s1");

        bool hasAdded = resp.contains("result") &&
            resp["result"].contains("added") &&
            resp["result"]["added"].is_array();
        bool hasRemoved = resp.contains("result") &&
            resp["result"].contains("removed") &&
            resp["result"]["removed"].is_array();
        bool hasVersion = resp.contains("result") &&
            resp["result"].contains("version");
        bool hasCounts = resp.contains("result") &&
            resp["result"].contains("addedCount") &&
            resp["result"].contains("removedCount");

        expect(hasAdded && hasRemoved && hasVersion && hasCounts,
               "Delta has added, removed, version, addedCount, removedCount",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Version increments on each getDiagnostics call
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("ver.py",
            "def foo():\n    pass\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "getDiagnostics"},
                    {"params", json::object()}};
        json r1 = state.processAgentRequest(req, "s1");
        int v1 = r1["result"].value("version", 0);

        json r2 = state.processAgentRequest(req, "s1");
        int v2 = r2["result"].value("version", 0);

        expect(v2 == v1 + 1,
               "Version increments: " + std::to_string(v1) +
               " → " + std::to_string(v2),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: DiagnosticKey comparison works correctly
    // ---------------------------------------------------------------
    {
        DiagnosticKey a{"E0200", "func_1", 10};
        DiagnosticKey b{"E0200", "func_1", 10};
        DiagnosticKey c{"E0201", "func_1", 10};
        DiagnosticKey d{"E0200", "func_2", 10};

        expect(a == b && !(a == c) && !(a == d) && a < c && a < d,
               "DiagnosticKey equality and ordering",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: DiagnosticVersionTracker reset clears state
    // ---------------------------------------------------------------
    {
        DiagnosticVersionTracker tracker;
        StructuredDiagnostic d;
        d.code = "E0100"; d.source = "parser"; d.line = 1;
        tracker.recordSnapshot({d});
        expect(tracker.version == 1, "version=1 before reset",
               passed, failed);
        // (extra check after reset will be in test 9)
    }

    // ---------------------------------------------------------------
    //  Test 9: Tracker reset returns to version 0
    // ---------------------------------------------------------------
    {
        DiagnosticVersionTracker tracker;
        StructuredDiagnostic d;
        d.code = "E0100"; d.source = "parser"; d.line = 1;
        tracker.recordSnapshot({d});
        tracker.reset();
        expect(tracker.version == 0,
               "Tracker reset returns to version 0",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: MCP tools/list includes diagnostics delta tool
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

        bool found = false;
        int toolCount = 0;
        if (r.contains("result") && r["result"].contains("tools")) {
            toolCount = (int)r["result"]["tools"].size();
            for (const auto& t : r["result"]["tools"]) {
                if (t.value("name", "") ==
                    "whetstone_get_diagnostics_delta")
                    found = true;
            }
        }
        expect(found && toolCount >= 21,
               "MCP tools/list has diagnostics_delta (total " +
               std::to_string(toolCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: Delta added diagnostics have structured format
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("fmt.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Baseline (no errors)
        json req1 = {{"jsonrpc", "2.0"}, {"id", 1},
                     {"method", "getDiagnostics"},
                     {"params", json::object()}};
        state.processAgentRequest(req1, "s1");

        // Add annotation error
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        json deltaReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "getDiagnosticsDelta"},
                         {"params", {{"sinceVersion", 0}}}};
        json resp = state.processAgentRequest(deltaReq, "s1");

        bool validFormat = false;
        if (resp.contains("result") &&
            !resp["result"]["added"].empty()) {
            const auto& d = resp["result"]["added"][0];
            validFormat = d.contains("code") &&
                          d.contains("severity") &&
                          d.contains("message") &&
                          d.contains("source");
        }
        expect(validFormat,
               "Delta added diagnostics have structured format",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Linter role can access getDiagnosticsDelta
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("perm.py",
            "def foo():\n    pass\n", "python");
        state.setAgentRole("linter", AgentRole::Linter);

        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "getDiagnosticsDelta"},
                    {"params", {{"sinceVersion", 0}}}};
        json resp = state.processAgentRequest(req, "linter");

        bool hasResult = resp.contains("result");
        expect(hasResult,
               "Linter role can access getDiagnosticsDelta",
               passed, failed);
    }

    std::cout << "\n=== Step 252 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
