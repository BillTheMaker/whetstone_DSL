// Step 254 TDD Test: Response Budget System
//
// Tests budget parameter on getAST and getDiagnostics: truncation,
// continuation tokens, pagination, and backward compatibility.
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

    // Build a large module for testing
    std::string bigSrc;
    for (int i = 0; i < 50; ++i) {
        bigSrc += "def func_" + std::to_string(i) + "(a, b):\n";
        bigSrc += "    return a + b\n\n";
    }

    // ---------------------------------------------------------------
    //  Test 1: getAST without budget → full response (backward compat)
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("big.py", bigSrc, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "getAST");
        bool hasAst = resp.contains("result") &&
            resp["result"].contains("ast");
        bool noTruncated = !resp["result"].contains("truncated") ||
            resp["result"].value("truncated", false) == false;
        expect(hasAst && noTruncated,
               "getAST without budget returns full response",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: getAST with small budget → truncated response
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("big.py", bigSrc, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "getAST",
                          {{"compact", true}, {"budget", 500}});
        bool isTruncated = resp.contains("result") &&
            resp["result"].value("truncated", false) == true;
        bool hasContinuation = resp.contains("result") &&
            resp["result"].contains("continuation");
        int size = (int)resp["result"].dump().size();
        expect(isTruncated && hasContinuation && size <= 600,
               "getAST budget=500 truncated (size=" +
               std::to_string(size) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Truncated response includes totalCount and returnedCount
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("big.py", bigSrc, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "getAST",
                          {{"compact", true}, {"budget", 500}});
        bool hasTotalCount = resp["result"].contains("totalCount");
        bool hasReturnedCount = resp["result"].contains("returnedCount");
        int total = resp["result"].value("totalCount", 0);
        int returned = resp["result"].value("returnedCount", 0);
        expect(hasTotalCount && hasReturnedCount &&
               returned < total && returned > 0,
               "Truncated has totalCount=" + std::to_string(total) +
               " returnedCount=" + std::to_string(returned),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Continuation token can be used for follow-up
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("big.py", bigSrc, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Get first page
        json resp1 = rpc(state, "s1", "getAST",
                           {{"compact", true}, {"budget", 500}});
        std::string cont = resp1["result"].value("continuation", "");
        int returned1 = resp1["result"].value("returnedCount", 0);

        // Use continuation token to get the full result and apply
        json fullResult;
        {
            json compactNodes = toJsonCompactSummary(state.activeAST());
            fullResult = {{"nodes", compactNodes},
                          {"nodeCount", (int)compactNodes.size()}};
        }
        json page2 = applyContinuation(fullResult, cont, 2000);
        bool hasOffset = page2.contains("offset");
        int offset = page2.value("offset", 0);
        expect(!cont.empty() && hasOffset && offset == returned1,
               "Continuation token resumes from offset=" +
               std::to_string(offset),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Budget on getAST full mode also truncates
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("big.py", bigSrc, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json fullResp = rpc(state, "s1", "getAST");
        int fullSize = (int)fullResp["result"].dump().size();

        json budgetResp = rpc(state, "s1", "getAST",
                                {{"budget", 1000}});
        bool truncated = budgetResp["result"].value("truncated", false);
        // Full response is huge for 50 functions, budget should set truncated
        expect(fullSize > 1000 && truncated,
               "Full-mode getAST with budget sets truncated flag (full=" +
               std::to_string(fullSize) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: getDiagnostics with budget truncates
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("diag.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Add multiple annotations to create multiple diagnostics
        Module* ast = state.activeAST();
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        // Without budget → normal response
        json fullResp = rpc(state, "s1", "getDiagnostics");
        int fullCount = fullResp["result"].value("count", 0);

        // With very small budget
        json budgetResp = rpc(state, "s1", "getDiagnostics",
                                {{"budget", 100}});
        bool hasDiags = budgetResp.contains("result") &&
            budgetResp["result"].contains("diagnostics");
        expect(fullCount > 0 && hasDiags,
               "getDiagnostics with budget returns diagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: applyBudget with no array returns truncated flag only
    // ---------------------------------------------------------------
    {
        json simple = {{"message", "hello"}, {"value", 42}};
        auto br = applyBudget(simple, 10);
        expect(br.truncated,
               "applyBudget on non-array result sets truncated=true",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: applyBudget with large budget returns full result
    // ---------------------------------------------------------------
    {
        json result = {{"items", json::array({1, 2, 3})}};
        auto br = applyBudget(result, 10000);
        expect(!br.truncated,
               "applyBudget with large budget returns full result",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: applyContinuation with invalid token returns error
    // ---------------------------------------------------------------
    {
        json full = {{"items", json::array({1, 2, 3})}};
        json result = applyContinuation(full, "invalid", 1000);
        bool hasError = result.contains("error");
        expect(hasError,
               "applyContinuation with invalid token returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: sortDiagnosticsByPriority puts errors first
    // ---------------------------------------------------------------
    {
        json diags = json::array({
            {{"code", "W001"}, {"severity", "warning"},
             {"severityLevel", 2}},
            {{"code", "E001"}, {"severity", "error"},
             {"severityLevel", 1}},
            {{"code", "I001"}, {"severity", "info"},
             {"severityLevel", 3}}
        });
        sortDiagnosticsByPriority(diags);
        bool sorted = diags[0].value("severity", "") == "error" &&
                      diags[1].value("severity", "") == "warning" &&
                      diags[2].value("severity", "") == "info";
        expect(sorted,
               "sortDiagnosticsByPriority: error → warning → info",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: Budget parameter zero treated as unlimited
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("zero.py", bigSrc, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "getAST",
                          {{"compact", true}, {"budget", 0}});
        bool noTruncated = !resp["result"].contains("truncated") ||
            resp["result"].value("truncated", false) == false;
        expect(noTruncated,
               "budget=0 treated as unlimited (no truncation)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Multiple pages cover all data
    // ---------------------------------------------------------------
    {
        // Test that continuation covers remaining items
        json items = json::array();
        for (int i = 0; i < 100; ++i)
            items.push_back({{"id", i}, {"value", "item_" + std::to_string(i)}});
        json full = {{"items", items}};
        auto br = applyBudget(full, 200);
        int firstReturned = 0;
        if (br.truncated && br.result.contains("returnedCount"))
            firstReturned = br.result.value("returnedCount", 0);

        json page2 = applyContinuation(full, br.continuation, 10000);
        int secondReturned = page2.value("returnedCount", 0);
        bool coversAll = (firstReturned + secondReturned) == 100;
        expect(br.truncated && coversAll,
               "Two pages cover all 100 items (first=" +
               std::to_string(firstReturned) + " second=" +
               std::to_string(secondReturned) + ")",
               passed, failed);
    }

    std::cout << "\n=== Step 254 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
