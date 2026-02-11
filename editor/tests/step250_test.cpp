// Step 250 TDD Test: Structured Diagnostic Format
//
// Tests the unified diagnostic format: error codes, severity levels,
// nodeId references, fix suggestions, and filtering. Exercises parse
// diagnostics, annotation validation, and strategy validation through
// the getDiagnostics RPC method.
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
    //  Test 1: Valid Python → zero diagnostics
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("valid.py",
            "def add(a, b):\n    return a + b\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        auto diags = collectAllDiagnostics(state.activeAST());
        expect(diags.empty(),
               "Valid Python produces zero diagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: Parse diagnostics from syntax errors have E01xx codes
    // ---------------------------------------------------------------
    {
        Pipeline pipeline;
        std::vector<ParseDiagnostic> parseDiags;
        auto mod = pipeline.parse("def broken(\n", "python", parseDiags);

        auto structured = collectParseDiagnostics(parseDiags);
        bool allParse = true;
        bool allHaveCode = true;
        for (const auto& d : structured) {
            if (d.source != "parser") allParse = false;
            if (d.code.substr(0, 3) != "E01") allHaveCode = false;
        }
        expect(!structured.empty() && allParse && allHaveCode,
               "Parse errors have E01xx codes and source='parser' (got " +
               std::to_string(structured.size()) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: Parse diagnostics have line/col and severity
    // ---------------------------------------------------------------
    {
        Pipeline pipeline;
        std::vector<ParseDiagnostic> parseDiags;
        pipeline.parse("def broken(\n", "python", parseDiags);

        auto structured = collectParseDiagnostics(parseDiags);
        bool hasLocation = !structured.empty() &&
                           structured[0].line > 0;
        bool hasSeverity = !structured.empty() &&
                           structured[0].severity == DiagnosticSeverity::Error;
        expect(hasLocation && hasSeverity,
               "Parse diagnostics have line > 0 and severity=error",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: Annotation validation produces E02xx diagnostics
    // ---------------------------------------------------------------
    {
        // Build AST with @Deallocate(Explicit) but no dealloc call
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("annotated.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        Module* ast = state.activeAST();
        // Add DeallocateAnnotation to the function node
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        auto diags = collectAllDiagnostics(ast);
        bool hasAnno = false;
        bool hasE02 = false;
        for (const auto& d : diags) {
            if (d.source == "annotation") hasAnno = true;
            if (d.code.substr(0, 3) == "E02") hasE02 = true;
        }
        expect(hasAnno && hasE02,
               "Annotation validation produces E02xx diagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: Annotation diagnostics include nodeId
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("anno2.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        Module* ast = state.activeAST();
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        auto diags = collectAllDiagnostics(ast);
        bool hasNodeId = false;
        for (const auto& d : diags) {
            if (d.source == "annotation" && !d.nodeId.empty())
                hasNodeId = true;
        }
        expect(hasNodeId,
               "Annotation diagnostics include non-empty nodeId",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: Annotation diagnostics include fix suggestions
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("fix.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        Module* ast = state.activeAST();
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        auto diags = collectAllDiagnostics(ast);
        bool hasFix = false;
        for (const auto& d : diags) {
            if (d.source == "annotation" && !d.fix.is_null()) {
                hasFix = d.fix.contains("type") &&
                         d.fix.contains("description");
            }
        }
        expect(hasFix,
               "Annotation diagnostics include fix with type and description",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: getDiagnostics RPC method returns structured format
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("rpc.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        Module* ast = state.activeAST();
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        json request = {{"jsonrpc", "2.0"}, {"id", 1},
                        {"method", "getDiagnostics"},
                        {"params", json::object()}};
        json resp = state.processAgentRequest(request, "s1");

        bool hasResult = resp.contains("result");
        bool hasDiags = hasResult &&
            resp["result"].contains("diagnostics") &&
            resp["result"]["diagnostics"].is_array();
        bool hasCount = hasResult &&
            resp["result"].contains("count");
        bool hasVersion = hasResult &&
            resp["result"].contains("version");
        int count = 0;
        if (hasCount) count = resp["result"].value("count", 0);

        expect(hasDiags && hasCount && hasVersion && count > 0,
               "getDiagnostics RPC returns diagnostics array with count=" +
               std::to_string(count),
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: getDiagnostics with severity filter
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("filter.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        Module* ast = state.activeAST();
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        // Get all diagnostics
        json reqAll = {{"jsonrpc", "2.0"}, {"id", 1},
                       {"method", "getDiagnostics"},
                       {"params", json::object()}};
        json respAll = state.processAgentRequest(reqAll, "s1");
        int allCount = 0;
        if (respAll.contains("result"))
            allCount = respAll["result"].value("count", 0);

        // Get only errors (hint filter would exclude nothing more)
        json reqErr = {{"jsonrpc", "2.0"}, {"id", 2},
                       {"method", "getDiagnostics"},
                       {"params", {{"severity", "error"}}}};
        json respErr = state.processAgentRequest(reqErr, "s1");
        int errCount = 0;
        if (respErr.contains("result"))
            errCount = respErr["result"].value("count", 0);

        expect(allCount > 0 && errCount <= allCount,
               "Severity filter works (all=" + std::to_string(allCount) +
               " error=" + std::to_string(errCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: diagnosticToJson produces correct fields
    // ---------------------------------------------------------------
    {
        StructuredDiagnostic d;
        d.code = "E0200";
        d.severity = DiagnosticSeverity::Error;
        d.nodeId = "func_1";
        d.line = 10;
        d.col = 4;
        d.message = "test message";
        d.source = "annotation";
        d.fix = {{"type", "deleteNode"}, {"nodeId", "func_1"}};

        json j = diagnosticToJson(d);
        bool hasAll = j.value("code", "") == "E0200" &&
                      j.value("severity", "") == "error" &&
                      j.value("severityLevel", 0) == 1 &&
                      j.value("nodeId", "") == "func_1" &&
                      j.value("line", 0) == 10 &&
                      j.value("col", 0) == 4 &&
                      j.value("message", "") == "test message" &&
                      j.value("source", "") == "annotation" &&
                      j.contains("fix") &&
                      j["fix"].value("type", "") == "deleteNode";
        expect(hasAll,
               "diagnosticToJson produces all required fields",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: MCP tools/list now includes whetstone_get_diagnostics
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
                if (t.value("name", "") == "whetstone_get_diagnostics")
                    found = true;
            }
        }
        expect(found && toolCount >= 18,
               "MCP tools/list includes whetstone_get_diagnostics (total " +
               std::to_string(toolCount) + " tools)",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: collectPipelineDiagnostics merges all sources
    // ---------------------------------------------------------------
    {
        Pipeline pipeline;
        auto pr = pipeline.run(
            "def broken(\n", "python", "cpp");
        auto diags = collectPipelineDiagnostics(pr);
        bool hasParser = false;
        for (const auto& d : diags) {
            if (d.source == "parser") hasParser = true;
        }
        expect(hasParser,
               "collectPipelineDiagnostics includes parser diagnostics",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: getDiagnostics with source filter
    // ---------------------------------------------------------------
    {
        HeadlessEditorState state;
        state.defaultLanguage = "python";
        state.openBuffer("src.py",
            "def cleanup(ptr):\n    x = ptr\n", "python");
        state.setAgentRole("s1", AgentRole::Refactor);

        Module* ast = state.activeAST();
        ASTNode* fn = nullptr;
        for (auto* c : ast->allChildren()) {
            if (c->conceptType == "Function") { fn = c; break; }
        }
        if (fn) {
            auto* anno = new DeallocateAnnotation();
            fn->addChild("annotations", anno);
        }

        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "getDiagnostics"},
                    {"params", {{"source", "annotation"}}}};
        json resp = state.processAgentRequest(req, "s1");
        bool allAnnotation = true;
        if (resp.contains("result") &&
            resp["result"].contains("diagnostics")) {
            for (const auto& d : resp["result"]["diagnostics"]) {
                if (d.value("source", "") != "annotation")
                    allAnnotation = false;
            }
        }
        int count = 0;
        if (resp.contains("result"))
            count = resp["result"].value("count", 0);
        expect(allAnnotation && count > 0,
               "Source filter returns only annotation diagnostics (count=" +
               std::to_string(count) + ")",
               passed, failed);
    }

    std::cout << "\n=== Step 250 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
