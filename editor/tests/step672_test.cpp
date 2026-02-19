// Step 672: whetstone_mcp rebuild + smoke test (8 tests)

#include "MCPServer.h"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

static int p = 0, f = 0;
#define T(n)    { std::cout << "  " << #n << "... "; }
#define P()     { std::cout << "PASS\n"; ++p; }
#define F(m)    { std::cout << "FAIL: " << m << "\n"; ++f; }
#define C(c,m)  if (!(c)) { F(m); return; }

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

static std::string findBinary() {
    std::vector<std::string> candidates = {
        "./whetstone_mcp",
        "whetstone_mcp",
        "editor/build-native/whetstone_mcp",
        "../build-native/whetstone_mcp",
        "/home/bill/Documents/CLionProjects/whetstone_DSL/editor/build-native/whetstone_mcp"
    };
    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) return path;
    }
    return "";
}

void t1() {
    T(binary_exists_after_build);
    std::string bin = findBinary();
    C(!bin.empty(), "whetstone_mcp binary not found");
    P();
}

void t2() {
    T(binary_runs_with_help);
    std::string bin = findBinary();
    C(!bin.empty(), "whetstone_mcp binary not found");
    int rc = std::system((bin + " --help >/dev/null 2>&1").c_str());
    C(rc == 0, "--help failed");
    P();
}

void t3() {
    T(tools_list_is_valid_json);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    C(resp.contains("result"), "missing result");
    C(resp["result"].contains("tools"), "missing tools");
    C(resp["result"]["tools"].is_array(), "tools should be array");
    P();
}

void t4() {
    T(tools_list_contains_generate_project);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_generate_project") { found = true; break; }
    }
    C(found, "whetstone_generate_project missing");
    P();
}

void t5() {
    T(tools_list_contains_generate_inference_job);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_generate_inference_job") { found = true; break; }
    }
    C(found, "whetstone_generate_inference_job missing");
    P();
}

void t6() {
    T(tools_list_total_is_86);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    C(resp["result"]["tools"].size() == 86, "expected 86 tools");
    P();
}

void t7() {
    T(generate_project_call_success);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_project",
                        {{"name", "MyApp"}, {"description", "Example app"},
                         {"dependencies", json::array({"nlohmann_json"})}});
    C(out.value("success", false), "expected success");
    C(!out.value("cmakeLists", "").empty(), "cmakeLists should be non-empty");
    P();
}

void t8() {
    T(generate_inference_job_call_success);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_generate_inference_job",
                        {{"goal", "extract duplicate handlers"}, {"entropy_score", 3},
                         {"files", json::array({"main.cpp"})}});
    C(out.value("success", false), "expected success");
    C(out["job"].value("type", "") == "refactor", "wrong job type");
    P();
}

int main() {
    std::cout << "Step 672: whetstone_mcp rebuild + smoke\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
