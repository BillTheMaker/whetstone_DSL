// Step 256 TDD Test: Batch Query Endpoint
//
// Tests batchQuery method: multiple queries in a single round-trip,
// independent error handling, and MCP tool registration.
#include "HeadlessEditorState.h"
#include "CompactAST.h"
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
        "def helper(x):\n"
        "    return x * 2\n\n"
        "def compute(a, b):\n"
        "    c = helper(a)\n"
        "    d = helper(b)\n"
        "    return c + d\n\n"
        "def main():\n"
        "    result = compute(1, 2)\n"
        "    return result\n";

    // ---------------------------------------------------------------
    //  Test 1: Basic batch with 3 queries returns 3 results
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"}, {"params", {{"compact", true}}}},
                {{"method", "getDiagnostics"}},
                {{"method", "ping"}}
            })}
        });
        bool hasResults = resp.contains("result") &&
            resp["result"].contains("results") &&
            resp["result"]["results"].is_array();
        int count = resp["result"].value("count", 0);
        expect(hasResults && count == 3,
               "Batch of 3 queries returns 3 results",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Each result has method field
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"}},
                {{"method", "ping"}}
            })}
        });
        bool allHaveMethod = true;
        for (const auto& r : resp["result"]["results"]) {
            if (!r.contains("method")) allHaveMethod = false;
        }
        expect(allHaveMethod,
               "Each batch result has method field",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Successful sub-queries have result field
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "ping"}},
                {{"method", "getAST"}, {"params", {{"compact", true}}}}
            })}
        });
        bool pingOk = resp["result"]["results"][0].contains("result");
        bool astOk = resp["result"]["results"][1].contains("result");
        expect(pingOk && astOk,
               "Successful sub-queries have result field",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Error in one sub-query doesn't fail others
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "ping"}},
                {{"method", "nonExistentMethod"}},
                {{"method", "getAST"}, {"params", {{"compact", true}}}}
            })}
        });
        bool pingOk = resp["result"]["results"][0].contains("result");
        bool errorOk = resp["result"]["results"][1].contains("error");
        bool astOk = resp["result"]["results"][2].contains("result");
        expect(pingOk && errorOk && astOk,
               "Error in sub-query #2 doesn't affect #1 or #3",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Batch with AST + diagnostics + scope
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"}, {"params", {{"compact", true}}}},
                {{"method", "getDiagnostics"}},
                {{"method", "getInScopeSymbols"},
                 {"params", {{"nodeId", computeId}}}}
            })}
        });
        auto& results = resp["result"]["results"];
        bool astHasNodes = results[0]["result"].contains("nodes") ||
                          results[0]["result"].contains("ast");
        bool diagHasDiags = results[1]["result"].contains("diagnostics");
        bool scopeHasSymbols = results[2]["result"].contains("symbols");
        expect(astHasNodes && diagHasDiags && scopeHasSymbols,
               "Batch [getAST, getDiagnostics, getInScopeSymbols] all succeed",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Empty batch returns empty results
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array()}
        });
        int count = resp["result"].value("count", -1);
        expect(count == 0,
               "Empty batch returns count=0",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Missing queries parameter returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", json::object());
        bool hasError = resp.contains("error");
        expect(hasError,
               "Missing queries parameter returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: Batch with budget params passes through
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"},
                 {"params", {{"compact", true}, {"budget", 200}}}}
            })}
        });
        auto& r = resp["result"]["results"][0]["result"];
        bool hasTruncated = r.contains("truncated");
        expect(hasTruncated,
               "Budget parameter passes through in batch sub-query",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Batch uses single round-trip for multiple queries
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Batch call returns all 3 results in a single response
        json batchResp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"}, {"params", {{"compact", true}}}},
                {{"method", "getDiagnostics"}},
                {{"method", "ping"}}
            })}
        });
        int batchCount = batchResp["result"].value("count", 0);
        bool allHaveResults = true;
        for (const auto& r : batchResp["result"]["results"]) {
            if (!r.contains("result") && !r.contains("error"))
                allHaveResults = false;
        }
        // One envelope, three results — that's the protocol win
        expect(batchCount == 3 && allHaveResults,
               "Single round-trip returns 3 results in one envelope",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Permission denied in sub-query returns error
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Linter);

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "ping"}},
                {{"method", "applyMutation"},
                 {"params", {{"type", "deleteNode"},
                             {"nodeId", "bad"}}}}
            })}
        });
        bool pingOk = resp["result"]["results"][0].contains("result");
        bool mutErr = resp["result"]["results"][1].contains("error");
        expect(pingOk && mutErr,
               "Permission-denied sub-query returns error, others succeed",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: MCP tool registration
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("mcp-session", AgentRole::Refactor);

        MCPBridge bridge;
        bridge.setRequestHandler([&state](const json& request) -> json {
            return state.processAgentRequest(request, "mcp-session");
        });

        json initReq = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "initialize"},
                        {"params", {{"protocolVersion", "2024-11-05"},
                                    {"clientInfo", {{"name", "test"}}}}}};
        bridge.processMessage(initReq.dump());

        json toolsReq = {{"jsonrpc", "2.0"}, {"id", 2},
                         {"method", "tools/list"}};
        std::string toolsResp = bridge.processMessage(toolsReq.dump());
        json toolsJson = json::parse(toolsResp);
        bool found = false;
        for (const auto& t : toolsJson["result"]["tools"]) {
            if (t["name"] == "whetstone_batch_query") found = true;
        }
        expect(found,
               "whetstone_batch_query registered in MCP tools/list",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Batch with detailed and lean scope queries
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json resp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getInScopeSymbols"},
                 {"params", {{"nodeId", computeId}}}},
                {{"method", "getInScopeSymbols"},
                 {"params", {{"nodeId", computeId}, {"detailed", true}}}}
            })}
        });
        auto& results = resp["result"]["results"];
        std::string leanMode = results[0]["result"].value("mode", "");
        std::string detMode = results[1]["result"].value("mode", "");
        expect(leanMode == "symbols" && detMode == "detailed",
               "Batch lean + detailed scope queries return correct modes",
               passed, failed);
    }

    std::cout << "\n=== Step 256 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
