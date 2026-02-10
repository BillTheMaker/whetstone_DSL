// Step 213: MCP Server tests
//
// Verifies:
//   1. Initialize handshake returns correct capabilities
//   2. tools/list returns all registered tools with schemas
//   3. tools/call for each tool produces correct results
//   4. resources/list returns all resources
//   5. resources/read returns valid data
//   6. prompts/list returns all prompts with argument schemas
//   7. prompts/get returns messages for each prompt
//   8. Invalid tool calls return MCP-compliant errors
//   9. MCPBridge processes message strings correctly

#include <cassert>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "MCPServer.h"
#include "MCPBridge.h"
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Serialization.h"
#include "ContextAPI.h"
#include "Pipeline.h"

using json = nlohmann::json;

static int passed = 0;
static int failed = 0;

static void expect(bool cond, const char* msg) {
    if (cond) { ++passed; }
    else { ++failed; printf("  FAIL: %s\n", msg); }
}

// Simple mock RPC handler that simulates a minimal Whetstone editor
static json mockRpcHandler(const json& request) {
    json response;
    response["jsonrpc"] = "2.0";
    response["id"] = request.value("id", json(nullptr));

    std::string method = request.value("method", "");

    if (method == "getAST") {
        response["result"] = {
            {"ast", {{"conceptType", "Module"}, {"id", "mod1"}, {"name", "test"}}},
            {"annotationCount", 0},
            {"diagnostics", json::array()}
        };
    } else if (method == "getInScopeSymbols") {
        response["result"] = {{"symbols", json::array({
            {{"name", "x"}, {"kind", "variable"}, {"nodeId", "v1"}}
        })}};
    } else if (method == "getCallHierarchy") {
        response["result"] = {
            {"functionId", "fn1"}, {"functionName", "test"},
            {"callerIds", json::array()}, {"calleeIds", json::array()}
        };
    } else if (method == "getAnnotationSuggestions") {
        response["result"] = {
            {"scopeId", "mod1"},
            {"suggestions", json::array()},
            {"diagnostics", json::array()}
        };
    } else if (method == "applyAnnotationSuggestion") {
        response["result"] = {{"success", true}, {"warning", ""}};
    } else if (method == "generateCode") {
        response["result"] = {
            {"node", {{"conceptType", "Function"}}},
            {"note", "generated"}, {"usedSymbols", json::array()},
            {"language", "python"}
        };
    } else if (method == "runPipeline") {
        response["result"] = {
            {"success", true}, {"generatedCode", "def f(): pass"},
            {"parseDiagnostics", json::array()},
            {"validationDiagnostics", json::array()},
            {"violations", json::array()},
            {"suggestions", json::array()},
            {"foldCount", 0}, {"dceCount", 0}
        };
    } else if (method == "projectLanguage") {
        response["result"] = {
            {"ast", {{"conceptType", "Module"}}},
            {"generatedCode", "void f() {}"},
            {"sourceLanguage", "python"}, {"targetLanguage", "cpp"}
        };
    } else if (method == "applyMutation") {
        response["result"] = {{"success", true}, {"warning", ""}};
    } else if (method == "applyBatch") {
        response["result"] = {{"success", true}, {"appliedCount", 2}};
    } else {
        response["error"] = {{"code", -32601}, {"message", "Unknown method"}};
    }
    return response;
}

// --- Test 1: Initialize handshake ---
static void test_initialize() {
    printf("Test 1: Initialize handshake...\n");
    MCPServer server;
    server.setRpcCallback(mockRpcHandler);

    expect(!server.isInitialized(), "not initialized before handshake");

    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}, {"params", json::object()}};
    json resp = server.handleRequest(req);

    expect(server.isInitialized(), "initialized after handshake");
    expect(resp.contains("result"), "response has result");
    expect(resp["result"]["protocolVersion"] == "2024-11-05", "protocol version correct");
    expect(resp["result"]["serverInfo"]["name"] == "whetstone-mcp", "server name correct");
    expect(resp["result"]["capabilities"].contains("tools"), "has tools capability");
    expect(resp["result"]["capabilities"].contains("resources"), "has resources capability");
    expect(resp["result"]["capabilities"].contains("prompts"), "has prompts capability");
}

// --- Test 2: tools/list ---
static void test_toolsList() {
    printf("Test 2: tools/list...\n");
    MCPServer server;

    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = server.handleRequest(req);

    expect(resp.contains("result"), "response has result");
    auto tools = resp["result"]["tools"];
    expect(tools.is_array(), "tools is array");
    expect(tools.size() >= 10, "at least 10 tools registered");

    // Check specific tools exist
    bool hasGetAST = false, hasMutate = false, hasBatch = false;
    bool hasScope = false, hasSuggest = false, hasPipeline = false;
    bool hasProject = false, hasGenerate = false, hasApplyAnno = false;
    bool hasCallHierarchy = false;
    for (const auto& t : tools) {
        std::string name = t["name"];
        if (name == "whetstone_get_ast") hasGetAST = true;
        if (name == "whetstone_mutate") hasMutate = true;
        if (name == "whetstone_batch_mutate") hasBatch = true;
        if (name == "whetstone_get_scope") hasScope = true;
        if (name == "whetstone_suggest_annotations") hasSuggest = true;
        if (name == "whetstone_run_pipeline") hasPipeline = true;
        if (name == "whetstone_project_language") hasProject = true;
        if (name == "whetstone_generate_code") hasGenerate = true;
        if (name == "whetstone_apply_annotation") hasApplyAnno = true;
        if (name == "whetstone_get_call_hierarchy") hasCallHierarchy = true;

        // Every tool must have description and inputSchema
        expect(t.contains("description") && !t["description"].get<std::string>().empty(),
               ("tool " + name + " has description").c_str());
        expect(t.contains("inputSchema"), ("tool " + name + " has inputSchema").c_str());
    }
    expect(hasGetAST, "whetstone_get_ast registered");
    expect(hasMutate, "whetstone_mutate registered");
    expect(hasBatch, "whetstone_batch_mutate registered");
    expect(hasScope, "whetstone_get_scope registered");
    expect(hasSuggest, "whetstone_suggest_annotations registered");
    expect(hasPipeline, "whetstone_run_pipeline registered");
    expect(hasProject, "whetstone_project_language registered");
    expect(hasGenerate, "whetstone_generate_code registered");
    expect(hasApplyAnno, "whetstone_apply_annotation registered");
    expect(hasCallHierarchy, "whetstone_get_call_hierarchy registered");
}

// --- Test 3: tools/call — various tools ---
static void test_toolsCall() {
    printf("Test 3: tools/call...\n");
    MCPServer server;
    server.setRpcCallback(mockRpcHandler);

    // Call whetstone_get_ast
    json req = {{"jsonrpc", "2.0"}, {"id", 3}, {"method", "tools/call"},
                {"params", {{"name", "whetstone_get_ast"}, {"arguments", json::object()}}}};
    json resp = server.handleRequest(req);
    expect(resp.contains("result"), "get_ast response has result");
    expect(resp["result"]["isError"] == false, "get_ast not error");
    expect(resp["result"]["content"].is_array(), "get_ast has content array");
    expect(resp["result"]["content"][0]["type"] == "text", "content is text type");

    // Call whetstone_get_scope
    req = {{"jsonrpc", "2.0"}, {"id", 4}, {"method", "tools/call"},
           {"params", {{"name", "whetstone_get_scope"}, {"arguments", {{"nodeId", "v1"}}}}}};
    resp = server.handleRequest(req);
    expect(resp["result"]["isError"] == false, "get_scope not error");

    // Call whetstone_run_pipeline
    req = {{"jsonrpc", "2.0"}, {"id", 5}, {"method", "tools/call"},
           {"params", {{"name", "whetstone_run_pipeline"},
                       {"arguments", {{"source", "def f(): pass"}, {"sourceLanguage", "python"}, {"targetLanguage", "python"}}}}}};
    resp = server.handleRequest(req);
    expect(resp["result"]["isError"] == false, "run_pipeline not error");

    // Call unknown tool
    req = {{"jsonrpc", "2.0"}, {"id", 6}, {"method", "tools/call"},
           {"params", {{"name", "nonexistent_tool"}, {"arguments", json::object()}}}};
    resp = server.handleRequest(req);
    expect(resp["result"]["isError"] == true, "unknown tool returns error");
}

// --- Test 4: resources/list ---
static void test_resourcesList() {
    printf("Test 4: resources/list...\n");
    MCPServer server;

    json req = {{"jsonrpc", "2.0"}, {"id", 7}, {"method", "resources/list"}};
    json resp = server.handleRequest(req);

    expect(resp.contains("result"), "response has result");
    auto resources = resp["result"]["resources"];
    expect(resources.is_array(), "resources is array");
    expect(resources.size() >= 5, "at least 5 resources");

    bool hasAST = false, hasDiags = false, hasLibs = false;
    for (const auto& r : resources) {
        if (r["uri"] == "whetstone://ast") hasAST = true;
        if (r["uri"] == "whetstone://diagnostics") hasDiags = true;
        if (r["uri"] == "whetstone://libraries") hasLibs = true;
        expect(r.contains("name"), "resource has name");
        expect(r.contains("description"), "resource has description");
    }
    expect(hasAST, "ast resource registered");
    expect(hasDiags, "diagnostics resource registered");
    expect(hasLibs, "libraries resource registered");
}

// --- Test 5: resources/read ---
static void test_resourcesRead() {
    printf("Test 5: resources/read...\n");
    MCPServer server;
    server.setResourceReader([](const std::string& uri) -> json {
        if (uri == "whetstone://ast") return {{"conceptType", "Module"}, {"name", "test"}};
        if (uri == "whetstone://diagnostics") return json::array();
        return {{"error", "unknown resource"}};
    });

    json req = {{"jsonrpc", "2.0"}, {"id", 8}, {"method", "resources/read"},
                {"params", {{"uri", "whetstone://ast"}}}};
    json resp = server.handleRequest(req);
    expect(resp.contains("result"), "read response has result");
    expect(resp["result"]["contents"].is_array(), "contents is array");
    expect(resp["result"]["contents"][0]["uri"] == "whetstone://ast", "correct uri");
}

// --- Test 6: prompts/list ---
static void test_promptsList() {
    printf("Test 6: prompts/list...\n");
    MCPServer server;

    json req = {{"jsonrpc", "2.0"}, {"id", 9}, {"method", "prompts/list"}};
    json resp = server.handleRequest(req);

    expect(resp.contains("result"), "response has result");
    auto prompts = resp["result"]["prompts"];
    expect(prompts.is_array(), "prompts is array");
    expect(prompts.size() >= 4, "at least 4 prompts");

    bool hasAnnotate = false, hasCross = false, hasSecurity = false, hasRefactor = false;
    for (const auto& p : prompts) {
        if (p["name"] == "annotate_module") hasAnnotate = true;
        if (p["name"] == "cross_language_projection") hasCross = true;
        if (p["name"] == "security_audit") hasSecurity = true;
        if (p["name"] == "refactor_memory") hasRefactor = true;
        expect(p.contains("description"), "prompt has description");
    }
    expect(hasAnnotate, "annotate_module prompt");
    expect(hasCross, "cross_language_projection prompt");
    expect(hasSecurity, "security_audit prompt");
    expect(hasRefactor, "refactor_memory prompt");
}

// --- Test 7: prompts/get ---
static void test_promptsGet() {
    printf("Test 7: prompts/get...\n");
    MCPServer server;

    json req = {{"jsonrpc", "2.0"}, {"id", 10}, {"method", "prompts/get"},
                {"params", {{"name", "annotate_module"}, {"arguments", {{"scope", "all"}}}}}};
    json resp = server.handleRequest(req);
    expect(resp.contains("result"), "get response has result");
    expect(resp["result"]["messages"].is_array(), "messages is array");
    expect(resp["result"]["messages"].size() >= 1, "at least 1 message");
    expect(resp["result"]["messages"][0]["role"] == "user", "first message from user");

    // Unknown prompt
    req = {{"jsonrpc", "2.0"}, {"id", 11}, {"method", "prompts/get"},
           {"params", {{"name", "nonexistent"}}}};
    resp = server.handleRequest(req);
    expect(resp.contains("error"), "unknown prompt returns error");
}

// --- Test 8: MCPBridge string processing ---
static void test_bridge() {
    printf("Test 8: MCPBridge...\n");
    MCPBridge bridge;
    bridge.setRequestHandler(mockRpcHandler);

    // Valid message
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "initialize"}};
    std::string resp = bridge.processMessage(req.dump());
    expect(!resp.empty(), "bridge returns response");
    json parsed = json::parse(resp);
    expect(parsed.contains("result"), "bridge response has result");

    // Invalid JSON
    resp = bridge.processMessage("not json");
    parsed = json::parse(resp);
    expect(parsed.contains("error"), "parse error for invalid JSON");
    expect(parsed["error"]["code"] == -32700, "correct parse error code");
}

// --- Test 9: Method not found ---
static void test_unknownMethod() {
    printf("Test 9: unknown method...\n");
    MCPServer server;

    json req = {{"jsonrpc", "2.0"}, {"id", 12}, {"method", "nonexistent/method"}};
    json resp = server.handleRequest(req);
    expect(resp.contains("error"), "unknown method returns error");
    expect(resp["error"]["code"] == -32601, "correct error code");
}

// --- Test 10: ping ---
static void test_ping() {
    printf("Test 10: ping...\n");
    MCPServer server;

    json req = {{"jsonrpc", "2.0"}, {"id", 13}, {"method", "ping"}};
    json resp = server.handleRequest(req);
    expect(resp.contains("result"), "ping returns result");
}

int main() {
    printf("=== Step 213: MCP Server Tests ===\n\n");

    test_initialize();
    test_toolsList();
    test_toolsCall();
    test_resourcesList();
    test_resourcesRead();
    test_promptsList();
    test_promptsGet();
    test_bridge();
    test_unknownMethod();
    test_ping();

    printf("\n=== Results: %d passed, %d failed ===\n", passed, failed);
    return failed ? 1 : 0;
}
