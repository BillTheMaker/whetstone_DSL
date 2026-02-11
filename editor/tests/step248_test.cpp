// Step 248 TDD Test: Compact AST Response Format
//
// Tests compact mode, subtree extraction, AST diff, version tracking,
// and tokenEstimate via HeadlessEditorState + RPC dispatch.
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

    // Build a module with 50 functions for size comparison
    std::string src;
    for (int i = 0; i < 50; ++i) {
        src += "def func_" + std::to_string(i) + "(a, b, c):\n";
        src += "    x = a + b\n";
        src += "    y = x * c\n";
        src += "    if y > 0:\n";
        src += "        return y\n";
        src += "    return x\n\n";
    }

    HeadlessEditorState state;
    state.workspaceRoot = "/tmp/step248-test";
    state.defaultLanguage = "python";
    state.verbose = false;
    state.openBuffer("multi.py", src, "python");
    state.setAgentRole("s1", AgentRole::Refactor);

    auto rpc = [&](const std::string& method,
                    json params = json::object()) {
        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", method}, {"params", params}};
        return state.processAgentRequest(request, "s1");
    };

    // ---------------------------------------------------------------
    //  Test 1: Full AST still works (backward compat)
    // ---------------------------------------------------------------
    json fullResp = rpc("getAST");
    {
        bool hasFull = fullResp.contains("result") &&
                       fullResp["result"].contains("ast");
        expect(hasFull, "Full AST response has 'ast' field (backward compat)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Compact AST returns flat node list
    // ---------------------------------------------------------------
    json compactResp = rpc("getAST", {{"compact", true}});
    {
        bool hasNodes = compactResp.contains("result") &&
                        compactResp["result"].contains("nodes") &&
                        compactResp["result"]["nodes"].is_array();
        int nodeCount = 0;
        if (hasNodes)
            nodeCount = (int)compactResp["result"]["nodes"].size();
        expect(hasNodes && nodeCount > 0,
               "Compact AST returns flat node list (got " +
               std::to_string(nodeCount) + " nodes)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Compact AST is <30% the size of full AST
    // ---------------------------------------------------------------
    {
        std::string fullDump = fullResp["result"].dump();
        std::string compactDump = compactResp["result"].dump();
        double ratio = (double)compactDump.size() / (double)fullDump.size();
        expect(ratio < 0.30,
               "Compact AST is <30% size of full (" +
               std::to_string((int)(ratio * 100)) + "%)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Compact nodes have expected fields
    // ---------------------------------------------------------------
    {
        bool valid = false;
        if (compactResp.contains("result") &&
            compactResp["result"].contains("nodes") &&
            !compactResp["result"]["nodes"].empty()) {
            const auto& first = compactResp["result"]["nodes"][0];
            valid = first.contains("id") && first.contains("type");
        }
        expect(valid,
               "Compact nodes have id and type fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: tokenEstimate is present and > 0
    // ---------------------------------------------------------------
    {
        int te = 0;
        if (fullResp.contains("result"))
            te = fullResp["result"].value("tokenEstimate", 0);
        expect(te > 0,
               "tokenEstimate is present and > 0 (got " +
               std::to_string(te) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Version counter starts at 0
    // ---------------------------------------------------------------
    {
        int ver = -1;
        if (fullResp.contains("result"))
            ver = fullResp["result"].value("version", -1);
        expect(ver == 0,
               "Version counter starts at 0",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: getASTSubtree returns subtree for valid nodeId
    // ---------------------------------------------------------------
    {
        // Find a function node ID from the compact response
        std::string funcId;
        if (compactResp.contains("result") &&
            compactResp["result"].contains("nodes")) {
            for (const auto& n : compactResp["result"]["nodes"]) {
                if (n.value("type", "") == "Function") {
                    funcId = n.value("id", "");
                    break;
                }
            }
        }
        json subResp = rpc("getASTSubtree", {{"nodeId", funcId}});
        bool hasSub = subResp.contains("result") &&
                      subResp["result"].contains("subtree") &&
                      subResp["result"]["subtree"].contains("id");
        std::string subId;
        if (hasSub)
            subId = subResp["result"]["subtree"].value("id", "");
        expect(hasSub && subId == funcId,
               "getASTSubtree returns subtree for function node",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: getASTSubtree with invalid nodeId returns error
    // ---------------------------------------------------------------
    {
        json resp = rpc("getASTSubtree", {{"nodeId", "nonexistent"}});
        bool isErr = resp.contains("error");
        expect(isErr,
               "getASTSubtree with invalid nodeId returns error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: getASTDiff with no changes returns empty diff
    // ---------------------------------------------------------------
    {
        int curVer = 0;
        if (fullResp.contains("result"))
            curVer = fullResp["result"].value("version", 0);
        json diffResp = rpc("getASTDiff", {{"sinceVersion", curVer}});
        int changedCount = -1;
        if (diffResp.contains("result"))
            changedCount = diffResp["result"].value("changedCount", -1);
        expect(changedCount == 0,
               "getASTDiff with no changes returns 0 changedCount",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Version increments after mutation
    // ---------------------------------------------------------------
    {
        // Find a function node to mutate
        std::string funcId;
        Module* ast = state.activeAST();
        if (ast) {
            for (auto* child : ast->allChildren()) {
                if (child->conceptType == "Function") {
                    funcId = child->id;
                    break;
                }
            }
        }
        int verBefore = state.active()->versionTracker.version;
        json mutResp = rpc("applyMutation", {
            {"type", "setProperty"}, {"nodeId", funcId},
            {"property", "name"}, {"value", "renamed_func"}});
        int verAfter = -1;
        if (mutResp.contains("result"))
            verAfter = mutResp["result"].value("version", -1);
        expect(verAfter == verBefore + 1,
               "Version increments after mutation (" +
               std::to_string(verBefore) + " -> " +
               std::to_string(verAfter) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: getASTDiff after mutation returns changed nodes
    // ---------------------------------------------------------------
    {
        json diffResp = rpc("getASTDiff", {{"sinceVersion", 0}});
        int changedCount = 0;
        if (diffResp.contains("result"))
            changedCount = diffResp["result"].value("changedCount", 0);
        expect(changedCount > 0,
               "getASTDiff after mutation returns changed nodes (got " +
               std::to_string(changedCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Subtree tokenEstimate < full AST tokenEstimate
    // ---------------------------------------------------------------
    {
        std::string funcId;
        if (compactResp.contains("result") &&
            compactResp["result"].contains("nodes")) {
            for (const auto& n : compactResp["result"]["nodes"]) {
                if (n.value("type", "") == "Function") {
                    funcId = n.value("id", "");
                    break;
                }
            }
        }
        json subResp = rpc("getASTSubtree", {{"nodeId", funcId}});
        int subTokens = 0, fullTokens = 0;
        if (subResp.contains("result"))
            subTokens = subResp["result"].value("tokenEstimate", 0);
        if (fullResp.contains("result"))
            fullTokens = fullResp["result"].value("tokenEstimate", 0);
        expect(subTokens > 0 && subTokens < fullTokens,
               "Subtree tokenEstimate (" + std::to_string(subTokens) +
               ") < full (" + std::to_string(fullTokens) + ")",
               passed, failed);
    }

    std::cout << "\n=== Step 248 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
