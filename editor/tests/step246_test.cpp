// Step 246 TDD Test: mcp_main.cpp — Standalone MCP Server Entry Point
//
// Tests the MCP server wiring end-to-end (in-process, not subprocess):
// HeadlessEditorState + MCPBridge connected via the same lambdas as mcp_main.
#include "HeadlessEditorState.h"
#include "MCPBridge.h"
#include "ast/Serialization.h"

#include <iostream>
#include <cassert>
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

// -----------------------------------------------------------------------
//  Resource reader (same as mcp_main.cpp)
// -----------------------------------------------------------------------
static json readResource(HeadlessEditorState& state,
                          const std::string& uri) {
    if (uri == "whetstone://ast") {
        Module* ast = state.activeAST();
        if (ast) return toJson(ast);
        return {{"error", "No active AST"}};
    }
    if (uri == "whetstone://diagnostics") {
        return state.buildDiagnosticsJson();
    }
    if (uri == "whetstone://libraries") {
        state.library.primitives.setRoot(state.activeAST());
        state.library.primitives.setLanguage(state.defaultLanguage);
        auto funcs = state.library.primitives.getAvailableFunctions();
        json libs = json::array();
        for (const auto& p : funcs) {
            libs.push_back(json{{"name", p.name}, {"kind", p.kind},
                                {"source", p.source}});
        }
        return libs;
    }
    if (uri == "whetstone://annotations") {
        Module* ast = state.activeAST();
        if (!ast) return json::array();
        json annotations = json::array();
        for (auto* child : ast->allChildren()) {
            if (isAnnotationNode(child)) {
                annotations.push_back(toJson(child));
            }
        }
        return annotations;
    }
    if (uri == "whetstone://settings") {
        return {
            {"workspace", state.workspaceRoot},
            {"language", state.defaultLanguage},
            {"verbose", state.verbose},
            {"bufferCount", (int)state.bufferStates.size()},
            {"mode", "headless-mcp"}
        };
    }
    return {{"error", "Unknown resource: " + uri}};
}

// -----------------------------------------------------------------------
//  CLI arg helpers (same signatures as mcp_main.cpp, for test #12)
// -----------------------------------------------------------------------
static std::string getArg(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc - 1; ++i) {
        if (argv[i] == key) return argv[i + 1];
    }
    return "";
}

static bool hasFlag(int argc, char** argv, const std::string& key) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == key) return true;
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Set up HeadlessEditorState + MCPBridge (same wiring as mcp_main) ---
    HeadlessEditorState state;
    state.workspaceRoot = "/tmp/mcp-test";
    state.defaultLanguage = "python";
    state.verbose = false;

    // Open a buffer with Python source for AST tests
    std::string src = "def greet(name):\n    return 'hello ' + name\n";
    state.openBuffer("test.py", src, "python");
    state.setAgentRole("mcp-session", AgentRole::Refactor);

    MCPBridge bridge;
    bridge.setRequestHandler([&state](const json& request) -> json {
        return state.processAgentRequest(request, "mcp-session");
    });
    bridge.setResourceReader([&state](const std::string& uri) -> json {
        return readResource(state, uri);
    });

    // ---------------------------------------------------------------
    //  Test 1: initialize request
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", "initialize"},
                    {"params", {{"protocolVersion", "2024-11-05"},
                                {"clientInfo", {{"name", "test"}}}}}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        expect(r.contains("result") &&
               r["result"].contains("protocolVersion") &&
               r["result"].contains("serverInfo"),
               "initialize returns protocolVersion and serverInfo",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 2: notifications/initialized (no crash, empty response)
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"},
                    {"method", "notifications/initialized"}};
        std::string resp = bridge.processMessage(req.dump());
        expect(resp.empty(), "notifications/initialized returns empty",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 3: tools/list returns 10+ tools
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 3},
                    {"method", "tools/list"}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        int toolCount = 0;
        if (r.contains("result") && r["result"].contains("tools"))
            toolCount = (int)r["result"]["tools"].size();
        expect(toolCount >= 10,
               "tools/list returns 10+ tools (got " +
               std::to_string(toolCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 4: resources/list returns 5 resources
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 4},
                    {"method", "resources/list"}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        int resCount = 0;
        if (r.contains("result") && r["result"].contains("resources"))
            resCount = (int)r["result"]["resources"].size();
        expect(resCount == 5,
               "resources/list returns 5 resources (got " +
               std::to_string(resCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 5: prompts/list returns 4 prompts
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 5},
                    {"method", "prompts/list"}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        int promptCount = 0;
        if (r.contains("result") && r["result"].contains("prompts"))
            promptCount = (int)r["result"]["prompts"].size();
        expect(promptCount == 4,
               "prompts/list returns 4 prompts (got " +
               std::to_string(promptCount) + ")",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 6: tools/call whetstone_run_pipeline
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 6},
                    {"method", "tools/call"},
                    {"params", {{"name", "whetstone_run_pipeline"},
                                {"arguments", {
                                    {"source", "def foo(): pass"},
                                    {"sourceLanguage", "python"},
                                    {"targetLanguage", "cpp"}
                                }}}}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        bool hasContent = r.contains("result") &&
                          r["result"].contains("content") &&
                          !r["result"]["content"].empty();
        std::string text = "";
        if (hasContent)
            text = r["result"]["content"][0].value("text", "");
        expect(hasContent && !text.empty() &&
               r["result"].value("isError", true) == false,
               "whetstone_run_pipeline generates code",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 7: tools/call whetstone_get_ast
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 7},
                    {"method", "tools/call"},
                    {"params", {{"name", "whetstone_get_ast"},
                                {"arguments", json::object()}}}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        bool hasContent = r.contains("result") &&
                          r["result"].contains("content") &&
                          !r["result"]["content"].empty();
        std::string text = "";
        if (hasContent)
            text = r["result"]["content"][0].value("text", "");
        expect(hasContent && !text.empty() &&
               text.find("ast") != std::string::npos,
               "whetstone_get_ast returns AST data",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 8: resources/read whetstone://ast
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 8},
                    {"method", "resources/read"},
                    {"params", {{"uri", "whetstone://ast"}}}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        bool hasContents = r.contains("result") &&
                           r["result"].contains("contents") &&
                           !r["result"]["contents"].empty();
        std::string text = "";
        if (hasContents)
            text = r["result"]["contents"][0].value("text", "");
        expect(hasContents && !text.empty(),
               "resources/read whetstone://ast returns content",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 9: resources/read whetstone://diagnostics
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 9},
                    {"method", "resources/read"},
                    {"params", {{"uri", "whetstone://diagnostics"}}}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        bool hasContents = r.contains("result") &&
                           r["result"].contains("contents") &&
                           !r["result"]["contents"].empty();
        expect(hasContents,
               "resources/read whetstone://diagnostics returns content",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 10: ping
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 10},
                    {"method", "ping"}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        expect(r.contains("result"),
               "ping returns result",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 11: resources/read whetstone://settings
    // ---------------------------------------------------------------
    {
        json req = {{"jsonrpc", "2.0"}, {"id", 11},
                    {"method", "resources/read"},
                    {"params", {{"uri", "whetstone://settings"}}}};
        std::string resp = bridge.processMessage(req.dump());
        json r = json::parse(resp);
        bool hasContents = r.contains("result") &&
                           r["result"].contains("contents") &&
                           !r["result"]["contents"].empty();
        std::string text = "";
        if (hasContents)
            text = r["result"]["contents"][0].value("text", "");
        expect(hasContents && text.find("headless-mcp") != std::string::npos,
               "resources/read whetstone://settings has headless-mcp mode",
               passed, failed);
    }

    // ---------------------------------------------------------------
    //  Test 12: CLI arg parsing helpers
    // ---------------------------------------------------------------
    {
        const char* testArgs[] = {
            "whetstone_mcp",
            "--workspace", "/tmp/ws",
            "--language", "rust",
            "--verbose"
        };
        int testArgc = 6;
        char** testArgv = const_cast<char**>(testArgs);

        std::string ws = getArg(testArgc, testArgv, "--workspace");
        std::string lang = getArg(testArgc, testArgv, "--language");
        bool verb = hasFlag(testArgc, testArgv, "--verbose");
        std::string missing = getArg(testArgc, testArgv, "--missing");

        expect(ws == "/tmp/ws" && lang == "rust" && verb &&
               missing.empty(),
               "CLI arg parsing works correctly",
               passed, failed);
    }

    std::cout << "\n=== Step 246 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
