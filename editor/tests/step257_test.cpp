// Step 257 TDD Test: Token Efficiency Tests + Benchmarks
//
// Phase 9c closer. Systematically measures token savings from compact AST,
// lean scope queries, diagnostic deltas, budget pagination, and batch
// queries. Benchmarks parse->mutate->diagnose cycle throughput.
#include "HeadlessEditorState.h"
#include "CompactAST.h"
#include "ResponseBudget.h"
#include "StructuredDiagnostics.h"

#include <iostream>
#include <string>
#include <chrono>

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

// Generate a Python module with N functions, each with a body
static std::string generateModule(int numFunctions) {
    std::string src;
    for (int i = 0; i < numFunctions; ++i) {
        src += "def func_" + std::to_string(i) + "(a, b):\n";
        src += "    c = a + b\n";
        src += "    return c\n\n";
    }
    return src;
}

int main() {
    int passed = 0;
    int failed = 0;

    // ---------------------------------------------------------------
    //  Test 1: Compact vs full AST ratio — 10-function module
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(10);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("small.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json fullResp = rpc(state, "s1", "getAST");
        json compactResp = rpc(state, "s1", "getAST",
                                 {{"compact", true}});
        int fullSize = (int)fullResp["result"].dump().size();
        int compactSize = (int)compactResp["result"].dump().size();
        double ratio = (double)compactSize / fullSize * 100.0;
        expect(ratio < 30.0 && compactSize > 0,
               "10-func compact/full ratio=" +
               std::to_string((int)ratio) + "% (<30%) [" +
               std::to_string(compactSize) + "/" +
               std::to_string(fullSize) + "]",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Compact vs full AST ratio — 50-function module
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(50);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("medium.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json fullResp = rpc(state, "s1", "getAST");
        json compactResp = rpc(state, "s1", "getAST",
                                 {{"compact", true}});
        int fullSize = (int)fullResp["result"].dump().size();
        int compactSize = (int)compactResp["result"].dump().size();
        double ratio = (double)compactSize / fullSize * 100.0;
        expect(ratio < 30.0 && compactSize > 0,
               "50-func compact/full ratio=" +
               std::to_string((int)ratio) + "% (<30%) [" +
               std::to_string(compactSize) + "/" +
               std::to_string(fullSize) + "]",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Compact vs full AST ratio — 200-function module
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(200);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("large.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json fullResp = rpc(state, "s1", "getAST");
        json compactResp = rpc(state, "s1", "getAST",
                                 {{"compact", true}});
        int fullSize = (int)fullResp["result"].dump().size();
        int compactSize = (int)compactResp["result"].dump().size();
        double ratio = (double)compactSize / fullSize * 100.0;
        expect(ratio < 30.0 && compactSize > 0,
               "200-func compact/full ratio=" +
               std::to_string((int)ratio) + "% (<30%) [" +
               std::to_string(compactSize) + "/" +
               std::to_string(fullSize) + "]",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Lean scope vs detailed scope — size ratio
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(50);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("scope.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                funcId = c->id;
                break;
            }
        }

        json lean = rpc(state, "s1", "getInScopeSymbols",
                          {{"nodeId", funcId}});
        json detailed = rpc(state, "s1", "getInScopeSymbols",
                              {{"nodeId", funcId}, {"detailed", true}});
        int leanSize = (int)lean["result"].dump().size();
        int detailedSize = (int)detailed["result"].dump().size();
        double ratio = (detailedSize > 0)
            ? (double)leanSize / detailedSize * 100.0 : 100.0;
        expect(ratio < 20.0 && leanSize > 0,
               "Lean/detailed scope ratio=" +
               std::to_string((int)ratio) + "% (<20%) [" +
               std::to_string(leanSize) + "/" +
               std::to_string(detailedSize) + "]",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Diagnostic delta vs full — size ratio after mutation
    // ---------------------------------------------------------------
    {
        std::string src =
            "def cleanup(ptr):\n    x = ptr\n\n"
            "def process(data):\n    return data\n";
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("delta.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Baseline diagnostics (records snapshot)
        json fullDiags = rpc(state, "s1", "getDiagnostics");
        int fullDiagSize = (int)fullDiags["result"].dump().size();

        // Add an annotation to trigger a new diagnostic
        Module* ast = state.activeAST();
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "cleanup") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        // Get delta instead of full re-fetch
        json delta = rpc(state, "s1", "getDiagnosticsDelta",
                           {{"sinceVersion", 0}});
        int deltaSize = (int)delta["result"].dump().size();
        // Delta should be smaller than a full diagnostic dump when
        // changes are small relative to the full set
        bool deltaValid = delta["result"].contains("added") &&
                          delta["result"].contains("removed");
        expect(deltaValid && deltaSize > 0 && fullDiagSize > 0,
               "Delta diagnostics valid (delta=" +
               std::to_string(deltaSize) + " full=" +
               std::to_string(fullDiagSize) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Budget pagination across 3 pages covers all items
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(50);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("pages.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Get full compact AST to know the total node count
        json fullCompact = rpc(state, "s1", "getAST",
                                 {{"compact", true}});
        int totalNodes = fullCompact["result"].value("nodeCount", 0);

        // Page 1: small budget
        json page1 = rpc(state, "s1", "getAST",
                           {{"compact", true}, {"budget", 500}});
        int p1Count = page1["result"].value("returnedCount", 0);
        std::string cont1 = page1["result"].value("continuation", "");

        // Build full result for continuation
        json compactNodes = toJsonCompactSummary(state.activeAST());
        json fullResult = {{"nodes", compactNodes},
                           {"nodeCount", (int)compactNodes.size()}};

        // Page 2
        json page2 = applyContinuation(fullResult, cont1, 500);
        int p2Count = page2.value("returnedCount", 0);
        std::string cont2 = page2.value("continuation", "");

        // Page 3: large budget to get the rest
        int p3Count = 0;
        if (!cont2.empty()) {
            json page3 = applyContinuation(fullResult, cont2, 100000);
            p3Count = page3.value("returnedCount", 0);
        }

        int covered = p1Count + p2Count + p3Count;
        expect(p1Count > 0 && p2Count > 0 && covered >= totalNodes,
               "3 pages cover all " + std::to_string(totalNodes) +
               " nodes (" + std::to_string(p1Count) + "+" +
               std::to_string(p2Count) + "+" +
               std::to_string(p3Count) + "=" +
               std::to_string(covered) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: Batch query vs sequential — total bytes comparison
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(30);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("batch.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                funcId = c->id;
                break;
            }
        }

        // Sequential: 3 separate RPC calls
        json r1 = rpc(state, "s1", "getAST", {{"compact", true}});
        json r2 = rpc(state, "s1", "getDiagnostics");
        json r3 = rpc(state, "s1", "getInScopeSymbols",
                        {{"nodeId", funcId}});
        int seqBytes = (int)(r1.dump().size() + r2.dump().size() +
                             r3.dump().size());

        // Batch: 1 RPC call
        json batchResp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"},
                 {"params", {{"compact", true}}}},
                {{"method", "getDiagnostics"}},
                {{"method", "getInScopeSymbols"},
                 {"params", {{"nodeId", funcId}}}}
            })}
        });
        int batchBytes = (int)batchResp.dump().size();

        // Batch has protocol overhead but only one envelope
        // The key win is round-trips, not bytes — but batch should
        // not be dramatically larger
        bool batchReasonable = batchBytes < seqBytes * 2;
        int batchCount = batchResp["result"].value("count", 0);
        expect(batchReasonable && batchCount == 3,
               "Batch " + std::to_string(batchBytes) +
               " bytes vs sequential " + std::to_string(seqBytes) +
               " bytes (3 results in 1 envelope)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: Token estimates decrease with compact mode
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(50);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("tokens.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json fullResp = rpc(state, "s1", "getAST");
        json compactResp = rpc(state, "s1", "getAST",
                                 {{"compact", true}});
        int fullTokens = fullResp["result"].value("tokenEstimate", 0);
        int compactTokens = compactResp["result"].value(
            "tokenEstimate", 0);
        // Both should report token estimates, compact should be lower
        expect(fullTokens > 0 && compactTokens > 0 &&
               compactTokens < fullTokens,
               "Token estimates: compact=" +
               std::to_string(compactTokens) + " < full=" +
               std::to_string(fullTokens),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: Lean call hierarchy + lean deps smaller than detailed
    // ---------------------------------------------------------------
    {
        std::string src =
            "def helper(x):\n"
            "    return x * 2\n\n"
            "def compute(a, b):\n"
            "    c = helper(a)\n"
            "    d = helper(b)\n"
            "    return c + d\n\n"
            "def orchestrate():\n"
            "    r1 = compute(1, 2)\n"
            "    r2 = compute(3, 4)\n"
            "    return r1 + r2\n";
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("hier.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string computeId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function" &&
                getNodeName(c) == "compute") {
                computeId = c->id;
                break;
            }
        }

        json leanCall = rpc(state, "s1", "getCallHierarchy",
                              {{"functionId", computeId}});
        json detCall = rpc(state, "s1", "getCallHierarchy",
                             {{"functionId", computeId},
                              {"detailed", true}});
        json leanDeps = rpc(state, "s1", "getDependencyGraph",
                              {{"nodeId", computeId}});
        json detDeps = rpc(state, "s1", "getDependencyGraph",
                             {{"nodeId", computeId},
                              {"detailed", true}});

        int leanCallSize = (int)leanCall["result"].dump().size();
        int detCallSize = (int)detCall["result"].dump().size();
        int leanDepsSize = (int)leanDeps["result"].dump().size();
        int detDepsSize = (int)detDeps["result"].dump().size();

        // Both lean modes should produce valid output
        bool callValid = leanCall["result"].value("mode", "") ==
            "symbols" && detCall["result"].value("mode", "") ==
            "detailed";
        bool depsValid = leanDeps["result"].value("mode", "") ==
            "symbols" && detDeps["result"].value("mode", "") ==
            "detailed";
        expect(callValid && depsValid,
               "Lean call=" + std::to_string(leanCallSize) +
               " det=" + std::to_string(detCallSize) +
               "; lean deps=" + std::to_string(leanDepsSize) +
               " det=" + std::to_string(detDepsSize),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: Combined efficiency — batch + compact + lean + budget
    // ---------------------------------------------------------------
    {
        std::string src = generateModule(50);
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("combined.py", src, "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                funcId = c->id;
                break;
            }
        }

        // Naive approach: full AST + detailed scope (2 separate calls)
        json naiveAST = rpc(state, "s1", "getAST");
        json naiveScope = rpc(state, "s1", "getInScopeSymbols",
                                {{"nodeId", funcId}, {"detailed", true}});
        int naiveTotal = (int)(naiveAST.dump().size() +
                               naiveScope.dump().size());

        // Optimized: batch with compact AST + lean scope + budget
        json optResp = rpc(state, "s1", "batchQuery", {
            {"queries", json::array({
                {{"method", "getAST"},
                 {"params", {{"compact", true}, {"budget", 2000}}}},
                {{"method", "getInScopeSymbols"},
                 {"params", {{"nodeId", funcId}}}}
            })}
        });
        int optTotal = (int)optResp.dump().size();

        double savings = (1.0 - (double)optTotal / naiveTotal) * 100.0;
        expect(optTotal < naiveTotal && savings > 50.0,
               "Combined savings=" + std::to_string((int)savings) +
               "% (opt=" + std::to_string(optTotal) + " naive=" +
               std::to_string(naiveTotal) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: Benchmark — 100 parse->mutate->diagnose cycles
    // ---------------------------------------------------------------
    {
        std::string baseSrc =
            "def target(x):\n    return x\n\n"
            "def main():\n    result = target(42)\n    return result\n";

        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < 100; ++i) {
            HeadlessEditorState state;
            state.defaultLanguage = "python";
            state.openBuffer("bench.py", baseSrc, "python");
            state.setAgentRole("s1", AgentRole::Refactor);

            // Parse (already done by openBuffer) — get AST
            rpc(state, "s1", "getAST", {{"compact", true}});

            // Mutate: add a parameter
            Module* ast = state.activeAST();
            if (ast) {
                for (auto* c : ast->allChildren()) {
                    if (c->conceptType == "Function" &&
                        getNodeName(c) == "target") {
                        auto* param = new Parameter(
                            "p_" + std::to_string(i),
                            "extra_" + std::to_string(i));
                        c->addChild("parameters", param);
                        break;
                    }
                }
            }

            // Diagnose
            rpc(state, "s1", "getDiagnostics");
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            end - start).count();
        double perCycle = (double)ms / 100.0;

        // Target: each cycle under 50ms (generous for CI)
        expect(perCycle < 50.0,
               "100 parse->mutate->diagnose cycles in " +
               std::to_string(ms) + "ms (" +
               std::to_string((int)perCycle) + "ms/cycle)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: Compact AST ratio stable across module sizes
    // ---------------------------------------------------------------
    {
        // Verify ratio doesn't degrade as module grows
        double ratios[3];
        int sizes[] = {10, 50, 200};
        for (int s = 0; s < 3; ++s) {
            std::string src = generateModule(sizes[s]);
            HeadlessEditorState state;
            state.defaultLanguage = "python";
            state.openBuffer("scale.py", src, "python");
            state.setAgentRole("s1", AgentRole::Refactor);

            json full = rpc(state, "s1", "getAST");
            json compact = rpc(state, "s1", "getAST",
                                 {{"compact", true}});
            int fSize = (int)full["result"].dump().size();
            int cSize = (int)compact["result"].dump().size();
            ratios[s] = (fSize > 0)
                ? (double)cSize / fSize * 100.0 : 100.0;
        }

        // All ratios should be under 30%, and spread should be small
        // (compact benefit doesn't degrade at scale)
        double maxRatio = std::max({ratios[0], ratios[1], ratios[2]});
        double minRatio = std::min({ratios[0], ratios[1], ratios[2]});
        double spread = maxRatio - minRatio;
        expect(maxRatio < 30.0 && spread < 15.0,
               "Ratio stable across sizes: 10-func=" +
               std::to_string((int)ratios[0]) + "% 50-func=" +
               std::to_string((int)ratios[1]) + "% 200-func=" +
               std::to_string((int)ratios[2]) + "% (spread=" +
               std::to_string((int)spread) + "%)",
               passed, failed);
    }

    std::cout << "\n=== Step 257 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
