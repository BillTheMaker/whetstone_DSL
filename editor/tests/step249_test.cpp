// Step 249 TDD Test: MCP Server Integration Tests
//
// End-to-end integration tests exercising the full MCP server stack through
// MCPBridge. Simulates a realistic agent session: handshake, tool discovery,
// AST queries, pipeline execution, diagnostics, file operations, and compact
// AST comparison.
#include "HeadlessEditorState.h"
#include "MCPBridge.h"
#include "ast/Serialization.h"

#include <iostream>
#include <cassert>
#include <string>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

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

// Resource reader (same wiring as mcp_main.cpp)
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

int main() {
    int passed = 0;
    int failed = 0;

    // --- Set up temp workspace ---
    std::string tmpDir = "/tmp/whetstone_test_249_" +
        std::to_string(std::chrono::steady_clock::now()
            .time_since_epoch().count());
    fs::create_directories(tmpDir);

    // --- Wire HeadlessEditorState + MCPBridge (same as mcp_main) ---
    HeadlessEditorState state;
    state.workspaceRoot = tmpDir;
    state.defaultLanguage = "python";
    state.verbose = false;

    // Open a buffer with multi-function Python source for AST tests
    std::string src;
    for (int i = 0; i < 10; ++i) {
        src += "def func_" + std::to_string(i) + "(x, y):\n";
        src += "    result = x + y\n";
        src += "    return result\n\n";
    }
    state.openBuffer("module.py", src, "python");
    state.setAgentRole("mcp-session", AgentRole::Refactor);

    MCPBridge bridge;
    bridge.setRequestHandler([&state](const json& request) -> json {
        return state.processAgentRequest(request, "mcp-session");
    });
    bridge.setResourceReader([&state](const std::string& uri) -> json {
        return readResource(state, uri);
    });

    // Helper: send MCP request, get parsed response
    auto mcp = [&](const std::string& method,
                    json params = json::object()) -> json {
        json req = {{"jsonrpc", "2.0"}, {"id", 1},
                    {"method", method}, {"params", params}};
        std::string resp = bridge.processMessage(req.dump());
        if (resp.empty()) return json(nullptr);
        return json::parse(resp);
    };

    // =================================================================
    //  Test 1: MCP handshake (initialize + notifications/initialized)
    // =================================================================
    {
        json initResp = mcp("initialize", {
            {"protocolVersion", "2024-11-05"},
            {"clientInfo", {{"name", "step249-integration-test"},
                            {"version", "1.0.0"}}}});
        bool hasProtocol = initResp.contains("result") &&
            initResp["result"].value("protocolVersion", "") == "2024-11-05";
        bool hasServer = initResp.contains("result") &&
            initResp["result"].contains("serverInfo") &&
            initResp["result"]["serverInfo"].value("name", "") ==
                "whetstone-mcp";
        bool hasCaps = initResp.contains("result") &&
            initResp["result"].contains("capabilities") &&
            initResp["result"]["capabilities"].contains("tools") &&
            initResp["result"]["capabilities"].contains("resources") &&
            initResp["result"]["capabilities"].contains("prompts");

        // Send notifications/initialized (no response expected)
        json notifResp = mcp("notifications/initialized");
        bool notifOk = notifResp.is_null();

        expect(hasProtocol && hasServer && hasCaps && notifOk,
               "MCP handshake: initialize + notifications/initialized",
               passed, failed);
    }

    // =================================================================
    //  Test 2: tools/list returns all 17 tools with valid schemas
    // =================================================================
    {
        json resp = mcp("tools/list");
        int toolCount = 0;
        bool allValid = true;
        if (resp.contains("result") && resp["result"].contains("tools")) {
            toolCount = (int)resp["result"]["tools"].size();
            for (const auto& tool : resp["result"]["tools"]) {
                if (!tool.contains("name") ||
                    !tool.contains("description") ||
                    !tool.contains("inputSchema")) {
                    allValid = false;
                }
                // Schema must have "type" field
                if (tool.contains("inputSchema") &&
                    !tool["inputSchema"].contains("type")) {
                    allValid = false;
                }
            }
        }
        expect(toolCount >= 17 && allValid,
               "tools/list returns 17+ tools with valid schemas (got " +
               std::to_string(toolCount) + ")",
               passed, failed);
    }

    // =================================================================
    //  Test 3: tools/call whetstone_get_ast returns valid AST
    // =================================================================
    {
        json resp = mcp("tools/call", {
            {"name", "whetstone_get_ast"},
            {"arguments", json::object()}});
        bool hasContent = resp.contains("result") &&
            resp["result"].contains("content") &&
            !resp["result"]["content"].empty();
        bool notError = resp.contains("result") &&
            resp["result"].value("isError", true) == false;
        std::string text;
        if (hasContent)
            text = resp["result"]["content"][0].value("text", "");
        // Should contain ast field and Module type
        bool hasAst = text.find("\"ast\"") != std::string::npos ||
                      text.find("\"type\"") != std::string::npos;
        expect(hasContent && notError && hasAst,
               "tools/call whetstone_get_ast returns valid AST data",
               passed, failed);
    }

    // =================================================================
    //  Test 4: tools/call whetstone_run_pipeline generates code
    // =================================================================
    {
        json resp = mcp("tools/call", {
            {"name", "whetstone_run_pipeline"},
            {"arguments", {
                {"source", "def add(a, b):\n    return a + b\n"},
                {"sourceLanguage", "python"},
                {"targetLanguage", "cpp"}}}});
        bool hasContent = resp.contains("result") &&
            resp["result"].contains("content") &&
            !resp["result"]["content"].empty();
        bool notError = resp.contains("result") &&
            resp["result"].value("isError", true) == false;
        std::string text;
        if (hasContent)
            text = resp["result"]["content"][0].value("text", "");
        // Generated C++ should contain function-like output
        bool hasCode = !text.empty() && text.size() > 10;
        expect(hasContent && notError && hasCode,
               "tools/call whetstone_run_pipeline generates code (" +
               std::to_string(text.size()) + " chars)",
               passed, failed);
    }

    // =================================================================
    //  Test 5: resources/read whetstone://diagnostics returns array
    // =================================================================
    {
        json resp = mcp("resources/read",
                         {{"uri", "whetstone://diagnostics"}});
        bool hasContents = resp.contains("result") &&
            resp["result"].contains("contents") &&
            !resp["result"]["contents"].empty();
        std::string text;
        if (hasContents)
            text = resp["result"]["contents"][0].value("text", "");
        // Diagnostics should be a JSON array (possibly empty)
        bool isArray = false;
        if (!text.empty()) {
            try {
                json diags = json::parse(text);
                isArray = diags.is_array();
            } catch (...) {}
        }
        expect(hasContents && isArray,
               "resources/read whetstone://diagnostics returns JSON array",
               passed, failed);
    }

    // =================================================================
    //  Test 6: prompts/list includes core prompts with structure
    // =================================================================
    {
        json resp = mcp("prompts/list");
        int promptCount = 0;
        bool allValid = true;
        if (resp.contains("result") && resp["result"].contains("prompts")) {
            promptCount = (int)resp["result"]["prompts"].size();
            for (const auto& p : resp["result"]["prompts"]) {
                if (!p.contains("name") || !p.contains("description")) {
                    allValid = false;
                }
            }
        }
        // Verify known prompt names exist
        bool hasAnnotate = false, hasCross = false,
             hasSecurity = false, hasRefactor = false;
        if (resp.contains("result") && resp["result"].contains("prompts")) {
            for (const auto& p : resp["result"]["prompts"]) {
                std::string name = p.value("name", "");
                if (name == "annotate_module") hasAnnotate = true;
                if (name == "cross_language_projection") hasCross = true;
                if (name == "security_audit") hasSecurity = true;
                if (name == "refactor_memory") hasRefactor = true;
            }
        }
        expect(promptCount >= 4 && allValid &&
               hasAnnotate && hasCross && hasSecurity && hasRefactor,
               "prompts/list returns core prompts with correct names",
               passed, failed);
    }

    // =================================================================
    //  Test 7: File operations cycle (create → write → read → diff)
    // =================================================================
    {
        // 7a: Create file via MCP tools/call
        json createResp = mcp("tools/call", {
            {"name", "whetstone_file_create"},
            {"arguments", {{"path", "agent_test.py"},
                           {"language", "python"},
                           {"template", "module"}}}});
        bool createOk = createResp.contains("result") &&
            createResp["result"].value("isError", true) == false;

        // 7b: Write content via MCP
        std::string newContent =
            "def compute(x):\n    return x * 2\n";
        json writeResp = mcp("tools/call", {
            {"name", "whetstone_file_write"},
            {"arguments", {{"path", "agent_test.py"},
                           {"content", newContent}}}});
        bool writeOk = writeResp.contains("result") &&
            writeResp["result"].value("isError", true) == false;

        // 7c: Read it back via MCP
        json readResp = mcp("tools/call", {
            {"name", "whetstone_file_read"},
            {"arguments", {{"path", "agent_test.py"}}}});
        bool readOk = readResp.contains("result") &&
            readResp["result"].value("isError", true) == false;
        std::string readText;
        if (readOk)
            readText = readResp["result"]["content"][0].value("text", "");
        bool contentMatch = readText.find("compute") != std::string::npos;

        // 7d: Verify file exists on disk
        bool onDisk = fs::exists(tmpDir + "/agent_test.py");

        expect(createOk && writeOk && readOk && contentMatch && onDisk,
               "File ops cycle: create → write → read → verify on disk",
               passed, failed);
    }

    // =================================================================
    //  Test 8: Compact AST vs full AST size comparison via MCP
    // =================================================================
    {
        // Full AST through MCP
        json fullResp = mcp("tools/call", {
            {"name", "whetstone_get_ast"},
            {"arguments", json::object()}});
        std::string fullText;
        if (fullResp.contains("result") &&
            fullResp["result"].contains("content") &&
            !fullResp["result"]["content"].empty()) {
            fullText = fullResp["result"]["content"][0].value("text", "");
        }

        // Compact AST through MCP
        json compactResp = mcp("tools/call", {
            {"name", "whetstone_get_ast"},
            {"arguments", {{"compact", true}}}});
        std::string compactText;
        if (compactResp.contains("result") &&
            compactResp["result"].contains("content") &&
            !compactResp["result"]["content"].empty()) {
            compactText =
                compactResp["result"]["content"][0].value("text", "");
        }

        bool bothNonEmpty = !fullText.empty() && !compactText.empty();
        double ratio = 1.0;
        if (bothNonEmpty)
            ratio = (double)compactText.size() / (double)fullText.size();
        expect(bothNonEmpty && ratio < 0.50,
               "Compact AST < 50% of full AST via MCP (" +
               std::to_string((int)(ratio * 100)) + "%, full=" +
               std::to_string(fullText.size()) + " compact=" +
               std::to_string(compactText.size()) + ")",
               passed, failed);
    }

    // --- Cleanup ---
    {
        std::error_code ec;
        fs::remove_all(tmpDir, ec);
    }

    std::cout << "\n=== Step 249 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
