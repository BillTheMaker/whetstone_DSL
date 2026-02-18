// Step 621: whetstone_set_workspace MCP tool (12 tests)

#include "HeadlessEditorState.h"
#include "MCPServer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json callTool(MCPServer& mcp, const std::string& name, const json& args) {
    json req = {
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "tools/call"},
        {"params", {{"name", name}, {"arguments", args}}}
    };
    json resp = mcp.handleRequest(req);
    std::string text = resp["result"]["content"][0].value("text", "{}");
    return json::parse(text);
}

static std::filesystem::path tempWorkspace() {
    std::filesystem::path root = std::filesystem::temp_directory_path() / "whetstone_step621";
    std::filesystem::remove_all(root);
    std::filesystem::create_directories(root);
    return root;
}

static void writeFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path);
    out << content;
}

void test_tool_registered_accessor() {
    TEST(tool_registered_accessor);
    MCPServer mcp;
    bool found = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name == "whetstone_set_workspace") found = true;
    }
    CHECK(found, "tool should be registered");
    PASS();
}

void test_tool_schema_requires_workspace() {
    TEST(tool_schema_requires_workspace);
    MCPServer mcp;
    bool required = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name != "whetstone_set_workspace") continue;
        for (const auto& req : tool.inputSchema["required"]) {
            if (req == "workspace") required = true;
        }
    }
    CHECK(required, "workspace should be required");
    PASS();
}

void test_tool_schema_requires_language() {
    TEST(tool_schema_requires_language);
    MCPServer mcp;
    bool required = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name != "whetstone_set_workspace") continue;
        for (const auto& req : tool.inputSchema["required"]) {
            if (req == "language") required = true;
        }
    }
    CHECK(required, "language should be required");
    PASS();
}

void test_missing_workspace_errors() {
    TEST(missing_workspace_errors);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_set_workspace", {{"language", "cpp"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "workspace_missing", "wrong error");
    PASS();
}

void test_missing_language_errors() {
    TEST(missing_language_errors);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_set_workspace", {{"workspace", "/tmp/ws"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "language_missing", "wrong error");
    PASS();
}

void test_workspace_must_be_string() {
    TEST(workspace_must_be_string);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_set_workspace",
                        {{"workspace", 42}, {"language", "cpp"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "workspace_not_string", "wrong error");
    PASS();
}

void test_language_must_be_string() {
    TEST(language_must_be_string);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_set_workspace",
                        {{"workspace", "/tmp/ws"}, {"language", 42}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "language_not_string", "wrong error");
    PASS();
}

void test_handler_calls_set_workspace_context_method() {
    TEST(handler_calls_set_workspace_context_method);
    MCPServer mcp;
    std::string capturedMethod;
    json capturedParams;
    mcp.setRpcCallback([&](const json& request) {
        capturedMethod = request.value("method", "");
        capturedParams = request.value("params", json::object());
        return json{{"result", {{"workspace", "/tmp/a"}, {"language", "go"}, {"fileCount", 3}, {"dirCount", 1}}}};
    });

    json out = callTool(mcp, "whetstone_set_workspace",
                        {{"workspace", "/tmp/a"}, {"language", "go"}});
    CHECK(out.value("success", false), "success should be true");
    CHECK(capturedMethod == "setWorkspaceContext", "wrong rpc method");
    CHECK(capturedParams.value("workspace", "") == "/tmp/a", "workspace not forwarded");
    CHECK(capturedParams.value("language", "") == "go", "language not forwarded");
    PASS();
}

void test_handler_returns_rpc_values() {
    TEST(handler_returns_rpc_values);
    MCPServer mcp;
    mcp.setRpcCallback([&](const json&) {
        return json{{"result", {{"workspace", "/tmp/custom"}, {"language", "rust"},
                                {"fileCount", 9}, {"dirCount", 4}}}};
    });
    json out = callTool(mcp, "whetstone_set_workspace",
                        {{"workspace", "/tmp/a"}, {"language", "go"}});
    CHECK(out.value("workspace", "") == "/tmp/custom", "workspace mismatch");
    CHECK(out.value("language", "") == "rust", "language mismatch");
    CHECK(out.value("fileCount", 0) == 9, "fileCount mismatch");
    CHECK(out.value("dirCount", 0) == 4, "dirCount mismatch");
    PASS();
}

void test_handler_surfaces_rpc_error() {
    TEST(handler_surfaces_rpc_error);
    MCPServer mcp;
    mcp.setRpcCallback([&](const json&) {
        return json{{"error", {{"message", "bad workspace"}}}};
    });
    json out = callTool(mcp, "whetstone_set_workspace",
                        {{"workspace", "/tmp/a"}, {"language", "go"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "bad workspace", "error message mismatch");
    PASS();
}

void test_integration_switches_headless_workspace_and_language() {
    TEST(integration_switches_headless_workspace_and_language);
    auto root = tempWorkspace();
    writeFile(root / "main.py", "print('ok')\n");

    HeadlessEditorState state;
    state.setAgentRole("mcp-session", AgentRole::Refactor);
    MCPServer mcp;
    mcp.setRpcCallback([&](const json& request) {
        return state.processAgentRequest(request, "mcp-session");
    });

    json out = callTool(mcp, "whetstone_set_workspace",
                        {{"workspace", root.string()}, {"language", "python"}});
    CHECK(out.value("success", false), "success should be true");
    CHECK(out.value("workspace", "") == root.string(), "workspace mismatch");
    CHECK(out.value("language", "") == "python", "language mismatch");
    PASS();
}

void test_integration_affects_workspace_list_tool() {
    TEST(integration_affects_workspace_list_tool);
    auto root = tempWorkspace();
    writeFile(root / "notes.txt", "hello\n");

    HeadlessEditorState state;
    state.setAgentRole("mcp-session", AgentRole::Refactor);
    MCPServer mcp;
    mcp.setRpcCallback([&](const json& request) {
        return state.processAgentRequest(request, "mcp-session");
    });

    (void)callTool(mcp, "whetstone_set_workspace",
                   {{"workspace", root.string()}, {"language", "python"}});
    json listed = callTool(mcp, "whetstone_workspace_list", json::object());
    CHECK(listed.contains("files"), "workspace list should return files");
    bool foundNotes = false;
    for (const auto& item : listed["files"]) {
        if (item.value("path", "") == "notes.txt") foundNotes = true;
    }
    CHECK(foundNotes, "expected notes.txt in workspace list");
    PASS();
}

int main() {
    std::cout << "Step 621: whetstone_set_workspace MCP tool\n";

    test_tool_registered_accessor();                          // 1
    test_tool_schema_requires_workspace();                    // 2
    test_tool_schema_requires_language();                     // 3
    test_missing_workspace_errors();                          // 4
    test_missing_language_errors();                           // 5
    test_workspace_must_be_string();                          // 6
    test_language_must_be_string();                           // 7
    test_handler_calls_set_workspace_context_method();        // 8
    test_handler_returns_rpc_values();                        // 9
    test_handler_surfaces_rpc_error();                        // 10
    test_integration_switches_headless_workspace_and_language();// 11
    test_integration_affects_workspace_list_tool();           // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
