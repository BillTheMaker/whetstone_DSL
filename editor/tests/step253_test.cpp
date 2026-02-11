// Step 253 TDD Test: Diagnostic and Delta Integration Tests
//
// Comprehensive end-to-end tests for the full diagnostic pipeline:
// structured diagnostics, quick fixes, delta streaming, and filtering.
// Exercises the complete flow: introduce error → diagnose → fix → verify.
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

// Helper: send RPC
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

    // =================================================================
    //  Test 1: Parse valid code → zero diagnostics
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("valid.py",
            "def add(a, b):\n    return a + b\n\n"
            "def mul(x, y):\n    return x * y\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        json resp = rpc(state, "s1", "getDiagnostics");
        int count = resp["result"].value("count", -1);
        expect(count == 0,
               "Valid Python code → zero diagnostics",
               passed, failed);
    }

    // =================================================================
    //  Test 2: Invalid code → structured diagnostics with nodeIds
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("anno.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Add annotation to create a diagnostic with nodeId
        Module* ast = state.activeAST();
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        json resp = rpc(state, "s1", "getDiagnostics");
        int count = resp["result"].value("count", 0);
        bool hasNodeId = false;
        bool hasCode = false;
        for (const auto& d : resp["result"]["diagnostics"]) {
            if (!d.value("nodeId", "").empty()) hasNodeId = true;
            if (!d.value("code", "").empty()) hasCode = true;
        }
        expect(count > 0 && hasNodeId && hasCode,
               "Annotation error has structured diagnostics with nodeId " +
               std::string("and code (count=") + std::to_string(count) + ")",
               passed, failed);
    }

    // =================================================================
    //  Test 3: Diagnostics include fix suggestions where applicable
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("fix.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        json resp = rpc(state, "s1", "getQuickFixes");
        int fixCount = resp["result"].value("count", 0);
        bool hasMutation = false;
        if (!resp["result"]["fixes"].empty()) {
            hasMutation = resp["result"]["fixes"][0].contains("mutation") &&
                resp["result"]["fixes"][0]["mutation"].contains("type");
        }
        expect(fixCount > 0 && hasMutation,
               "Quick fixes include concrete mutation objects (fixes=" +
               std::to_string(fixCount) + ")",
               passed, failed);
    }

    // =================================================================
    //  Test 4: applyQuickFix resolves the diagnostic
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("resolve.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                funcId = c->id;
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        // Verify diagnostic exists before fix
        json before = rpc(state, "s1", "getDiagnostics");
        int beforeCount = before["result"].value("count", 0);

        // Apply quick fix
        json fixResp = rpc(state, "s1", "applyQuickFix",
            {{"diagCode", "E0200"}, {"nodeId", funcId}});
        bool cleared = fixResp.contains("result") &&
            fixResp["result"].value("diagnosticCleared", false);
        int remaining = 0;
        if (fixResp.contains("result"))
            remaining = fixResp["result"].value("remainingDiagnostics", -1);

        expect(beforeCount > 0 && cleared && remaining == 0,
               "applyQuickFix resolves diagnostic (before=" +
               std::to_string(beforeCount) + " cleared=" +
               (cleared ? "true" : "false") + " remaining=" +
               std::to_string(remaining) + ")",
               passed, failed);
    }

    // =================================================================
    //  Test 5: Delta after fix shows removal
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("delta1.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        std::string funcId;
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                funcId = c->id;
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        // Get diagnostics (establishes version with error)
        json diagResp = rpc(state, "s1", "getDiagnostics");
        int ver = diagResp["result"].value("version", 0);

        // Remove the annotation (fix the error manually)
        Module* ast = state.activeAST();
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") {
                auto annos = c->getChildren("annotations");
                for (auto* a : annos) {
                    if (a->conceptType == "DeallocateAnnotation") {
                        c->removeChild(a);
                        delete a;
                        break;
                    }
                }
                break;
            }
        }

        // Get delta → should show removal
        json delta = rpc(state, "s1", "getDiagnosticsDelta",
                          {{"sinceVersion", ver}});
        int removed = delta["result"].value("removedCount", 0);
        int added = delta["result"].value("addedCount", 0);
        expect(removed > 0 && added == 0,
               "Delta after fix: removed=" + std::to_string(removed) +
               " added=" + std::to_string(added),
               passed, failed);
    }

    // =================================================================
    //  Test 6: Delta after introducing new error shows addition
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("delta2.py",
            "def foo():\n    return 1\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        // Get diagnostics (clean baseline)
        json diagResp = rpc(state, "s1", "getDiagnostics");
        int ver = diagResp["result"].value("version", 0);
        int initialCount = diagResp["result"].value("count", 0);

        // Introduce an error
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        // Get delta → should show addition
        json delta = rpc(state, "s1", "getDiagnosticsDelta",
                          {{"sinceVersion", ver}});
        int added = delta["result"].value("addedCount", 0);
        int removed = delta["result"].value("removedCount", 0);
        expect(initialCount == 0 && added > 0 && removed == 0,
               "Delta after new error: added=" + std::to_string(added) +
               " removed=" + std::to_string(removed),
               passed, failed);
    }

    // =================================================================
    //  Test 7: Combined tree-sitter + Whetstone diagnostics in one stream
    // =================================================================
    {
        // Use the pipeline to get both parse and validation diagnostics
        Pipeline pipeline;
        auto pr = pipeline.run("def broken(\n", "python", "cpp");
        auto allDiags = collectPipelineDiagnostics(pr);

        bool hasParser = false;
        for (const auto& d : allDiags) {
            if (d.source == "parser") hasParser = true;
        }

        // Also test annotation diagnostics in a separate AST
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("combined.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }
        auto astDiags = collectAllDiagnostics(state.activeAST());
        bool hasAnnotation = false;
        for (const auto& d : astDiags) {
            if (d.source == "annotation") hasAnnotation = true;
        }

        expect(hasParser && hasAnnotation,
               "Both parser and annotation diagnostics available in streams",
               passed, failed);
    }

    // =================================================================
    //  Test 8: Severity filtering works across the diagnostic pipeline
    // =================================================================
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("sev.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        for (auto* c : state.activeAST()->allChildren()) {
            if (c->conceptType == "Function") {
                c->addChild("annotations", new DeallocateAnnotation());
                break;
            }
        }

        // All diagnostics
        json all = rpc(state, "s1", "getDiagnostics");
        int allCount = all["result"].value("count", 0);

        // Errors only
        json errOnly = rpc(state, "s1", "getDiagnostics",
                            {{"severity", "error"}});
        int errCount = errOnly["result"].value("count", 0);

        // All diagnostics with hint level (should include everything)
        json hint = rpc(state, "s1", "getDiagnostics",
                         {{"severity", "hint"}});
        int hintCount = hint["result"].value("count", 0);

        expect(allCount > 0 && errCount <= allCount &&
               hintCount >= errCount,
               "Severity filtering: all=" + std::to_string(allCount) +
               " error=" + std::to_string(errCount) +
               " hint=" + std::to_string(hintCount),
               passed, failed);
    }

    std::cout << "\n=== Step 253 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
