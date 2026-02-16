// Step 421: Workspace Onboarding Flow Tests (12 tests)

#include "MCPServer.h"
#include "HeadlessEditorState.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json rpcResult(const json& result, int id = 1) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

static json parseToolResult(const json& toolCallResponse) {
    if (!toolCallResponse.contains("result")) return json::object();
    auto content = toolCallResponse["result"].value("content", json::array());
    if (!content.is_array() || content.empty()) return json::object();
    std::string text = content[0].value("text", "{}");
    try {
        return json::parse(text);
    } catch (...) {
        return json::object();
    }
}

static json callTool(MCPServer& server, const std::string& name,
                     const json& args = json::object()) {
    auto resp = server.handleRequest({
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "tools/call"},
        {"params", {{"name", name}, {"arguments", args}}}
    });
    return parseToolResult(resp);
}

static std::string makeTempDir(const std::string& name) {
    auto dir = std::filesystem::temp_directory_path() / ("whetstone_step421_" + name);
    std::filesystem::create_directories(dir);
    return dir.string();
}

void test_onboard_tool_registered() {
    TEST(onboard_tool_registered);
    MCPServer server;
    bool found = false;
    for (const auto& t : server.getTools()) {
        if (t.name == "whetstone_onboard_workspace") found = true;
    }
    CHECK(found, "missing whetstone_onboard_workspace tool");
    PASS();
}

void test_onboard_calls_index_workspace_and_list() {
    TEST(onboard_calls_index_workspace_and_list);
    MCPServer server;
    std::vector<std::string> methods;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        methods.push_back(method);
        if (method == "indexWorkspace")
            return rpcResult({{"root", "/repo"}, {"fileCount", 3}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array()}});
        if (method == "fileCreate")
            return rpcResult({{"created", ".whetstone/README.md"}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(res.value("success", false), "expected success");
    CHECK(methods.size() >= 2, "expected at least index+list calls");
    CHECK(methods[0] == "indexWorkspace", "first call should indexWorkspace");
    CHECK(methods[1] == "workspaceList", "second call should workspaceList");
    PASS();
}

void test_onboard_passes_root_to_index_workspace() {
    TEST(onboard_passes_root_to_index_workspace);
    MCPServer server;
    std::string rootParam;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace") {
            rootParam = req["params"].value("root", "");
            return rpcResult({{"root", rootParam}, {"fileCount", 1}});
        }
        if (method == "workspaceList") return rpcResult({{"files", json::array()}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace", {{"root", "/tmp/repo"}});
    CHECK(res.value("success", false), "expected success");
    CHECK(rootParam == "/tmp/repo", "root param not forwarded");
    PASS();
}

void test_onboard_respects_max_files_limit() {
    TEST(onboard_respects_max_files_limit);
    MCPServer server;
    int openCount = 0;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace")
            return rpcResult({{"root", "/repo"}, {"fileCount", 4}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array({
                {{"path", "a.cpp"}, {"isDir", false}},
                {{"path", "b.cpp"}, {"isDir", false}},
                {{"path", "c.py"}, {"isDir", false}},
                {{"path", "d.ts"}, {"isDir", false}}
            })}});
        if (method == "openFile") { ++openCount; return rpcResult(req["params"]); }
        if (method == "inferAnnotations") return rpcResult({{"count", 2}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace", {{"maxFiles", 2}});
    CHECK(res.value("success", false), "expected success");
    CHECK(openCount == 2, "expected openFile called exactly maxFiles times");
    PASS();
}

void test_onboard_filters_ignored_and_non_source_files() {
    TEST(onboard_filters_ignored_and_non_source_files);
    MCPServer server;
    int openCount = 0;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace")
            return rpcResult({{"root", "/repo"}, {"fileCount", 6}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array({
                {{"path", "src/main.cpp"}, {"isDir", false}},
                {{"path", "README.md"}, {"isDir", false}},
                {{"path", "build/tmp.cpp"}, {"isDir", false}},
                {{"path", "node_modules/pkg/index.js"}, {"isDir", false}},
                {{"path", ".whetstone/cache.py"}, {"isDir", false}},
                {{"path", "scripts/run.sh"}, {"isDir", false}}
            })}});
        if (method == "openFile") { ++openCount; return rpcResult(req["params"]); }
        if (method == "inferAnnotations") return rpcResult({{"count", 1}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(res.value("success", false), "expected success");
    CHECK(openCount == 1, "expected only source file outside ignored dirs");
    PASS();
}

void test_onboard_reports_language_counts() {
    TEST(onboard_reports_language_counts);
    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace")
            return rpcResult({{"root", "/repo"}, {"fileCount", 4}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array({
                {{"path", "a.cpp"}, {"isDir", false}},
                {{"path", "b.cpp"}, {"isDir", false}},
                {{"path", "c.py"}, {"isDir", false}},
                {{"path", "d.go"}, {"isDir", false}}
            })}});
        if (method == "openFile") return rpcResult(req["params"]);
        if (method == "inferAnnotations") return rpcResult({{"count", 1}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(res["languageCounts"].value("cpp", 0) == 2, "cpp count mismatch");
    CHECK(res["languageCounts"].value("python", 0) == 1, "python count mismatch");
    CHECK(res["languageCounts"].value("go", 0) == 1, "go count mismatch");
    PASS();
}

void test_onboard_aggregates_inference_and_sidecar_counts() {
    TEST(onboard_aggregates_inference_and_sidecar_counts);
    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace")
            return rpcResult({{"root", "/repo"}, {"fileCount", 2}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array({
                {{"path", "a.cpp"}, {"isDir", false}},
                {{"path", "b.py"}, {"isDir", false}}
            })}});
        if (method == "openFile") return rpcResult(req["params"]);
        if (method == "inferAnnotations") return rpcResult({{"count", 3}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(res.value("inferredAnnotationCount", -1) == 6, "inference count mismatch");
    CHECK(res.value("savedSidecarCount", -1) == 2, "saved sidecar count mismatch");
    PASS();
}

void test_onboard_warns_and_continues_on_open_failure() {
    TEST(onboard_warns_and_continues_on_open_failure);
    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace") return rpcResult({{"root", "/repo"}, {"fileCount", 2}});
        if (method == "workspaceList") return rpcResult({{"files", json::array({
            {{"path", "a.cpp"}, {"isDir", false}},
            {{"path", "b.py"}, {"isDir", false}}
        })}});
        if (method == "openFile") {
            std::string path = req["params"].value("path", "");
            if (path == "a.cpp")
                return json{{"jsonrpc", "2.0"}, {"id", 1},
                            {"error", {{"code", -1}, {"message", "open fail"}}}};
            return rpcResult(req["params"]);
        }
        if (method == "inferAnnotations") return rpcResult({{"count", 2}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(res.value("success", false), "expected overall success");
    CHECK(res.value("processedFileCount", -1) == 1, "expected one processed file");
    CHECK(res["warnings"].is_array() && !res["warnings"].empty(), "expected warnings");
    PASS();
}

void test_onboard_warns_and_continues_on_infer_failure() {
    TEST(onboard_warns_and_continues_on_infer_failure);
    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace") return rpcResult({{"root", "/repo"}, {"fileCount", 2}});
        if (method == "workspaceList") return rpcResult({{"files", json::array({
            {{"path", "a.cpp"}, {"isDir", false}},
            {{"path", "b.py"}, {"isDir", false}}
        })}});
        if (method == "openFile") return rpcResult(req["params"]);
        if (method == "inferAnnotations") {
            static int callIdx = 0;
            ++callIdx;
            if (callIdx == 1)
                return json{{"jsonrpc", "2.0"}, {"id", 1},
                            {"error", {{"code", -1}, {"message", "infer fail"}}}};
            return rpcResult({{"count", 4}});
        }
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(res.value("success", false), "expected overall success");
    CHECK(res.value("processedFileCount", -1) == 1, "expected one processed file");
    CHECK(res.value("savedSidecarCount", -1) == 1, "expected one saved sidecar");
    CHECK(res["warnings"].is_array() && !res["warnings"].empty(), "expected warnings");
    PASS();
}

void test_onboard_returns_failure_when_index_fails() {
    TEST(onboard_returns_failure_when_index_fails);
    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        if (req.value("method", "") == "indexWorkspace")
            return json{{"jsonrpc", "2.0"}, {"id", 1},
                        {"error", {{"code", -32602}, {"message", "missing root"}}}};
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(!res.value("success", true), "expected failure");
    CHECK(res.contains("error"), "expected error payload");
    PASS();
}

void test_onboard_creates_whetstone_readme_and_suggestion() {
    TEST(onboard_creates_whetstone_readme_and_suggestion);
    MCPServer server;
    std::string createdPath;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "indexWorkspace") return rpcResult({{"root", "/repo"}, {"fileCount", 1}});
        if (method == "workspaceList") return rpcResult({{"files", json::array({{{"path", "a.cpp"}, {"isDir", false}}})}});
        if (method == "openFile") return rpcResult(req["params"]);
        if (method == "inferAnnotations") return rpcResult({{"count", 1}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") {
            createdPath = req["params"].value("path", "");
            return rpcResult({{"created", true}});
        }
        return rpcResult(json::object());
    });
    auto res = callTool(server, "whetstone_onboard_workspace");
    CHECK(createdPath == ".whetstone/README.md", "expected readme bootstrap path");
    CHECK(res["workflowSuggestion"]["nextTools"].is_array(), "missing nextTools suggestion");
    CHECK(!res["workflowSuggestion"].value("note", "").empty(), "missing suggestion note");
    PASS();
}

void test_onboard_headless_end_to_end() {
    TEST(onboard_headless_end_to_end);
    auto root = std::filesystem::path(makeTempDir("e2e"));
    std::filesystem::create_directories(root / "src");
    std::ofstream(root / "src" / "main.cpp") << "int add(int a,int b){return a+b;}\n";
    std::ofstream(root / "src" / "util.py") << "def inc(x):\n    return x + 1\n";

    HeadlessEditorState state;
    state.setAgentRole("sess", AgentRole::Refactor);
    state.workspaceRoot = root.string();

    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        return state.processAgentRequest(req, "sess");
    });

    auto res = callTool(server, "whetstone_onboard_workspace",
                        {{"root", root.string()}, {"maxFiles", 2}});
    CHECK(res.value("success", false), "expected onboarding success");
    CHECK(res.value("processedFileCount", 0) >= 1, "expected at least one processed file");
    CHECK(std::filesystem::exists(root / ".whetstone" / "README.md"),
          "expected .whetstone readme");
    PASS();
}

int main() {
    std::cout << "Step 421: Workspace Onboarding Flow Tests\n";

    test_onboard_tool_registered();                         // 1
    test_onboard_calls_index_workspace_and_list();          // 2
    test_onboard_passes_root_to_index_workspace();          // 3
    test_onboard_respects_max_files_limit();                // 4
    test_onboard_filters_ignored_and_non_source_files();    // 5
    test_onboard_reports_language_counts();                 // 6
    test_onboard_aggregates_inference_and_sidecar_counts(); // 7
    test_onboard_warns_and_continues_on_open_failure();     // 8
    test_onboard_warns_and_continues_on_infer_failure();    // 9
    test_onboard_returns_failure_when_index_fails();        // 10
    test_onboard_creates_whetstone_readme_and_suggestion(); // 11
    test_onboard_headless_end_to_end();                     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
