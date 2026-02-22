// Step 694: whetstone_get_language_matrix MCP tool (8 tests)

#include "MCPServer.h"

#include <iostream>

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

void t1() {
    T(tool_registration);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_get_language_matrix") { found = true; break; }
    }
    C(found, "tool missing");
    P();
}

void t2() {
    T(schema_validation);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool ok = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") != "whetstone_get_language_matrix") continue;
        ok = t.contains("inputSchema") && t["inputSchema"].is_object();
    }
    C(ok, "inputSchema missing");
    P();
}

void t3() {
    T(single_language_query);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_language_matrix", {{"language", "rust"}});
    C(out.value("success", false), "expected success");
    C(out["rows"].size() == 1, "expected single row");
    C(out["rows"][0].value("language", "") == "rust", "wrong language");
    P();
}

void t4() {
    T(all_language_query);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_language_matrix", json::object());
    C(out.value("success", false), "expected success");
    C(out["rows"].is_array(), "rows not array");
    C(out["rows"].size() >= 5, "expected default rows");
    P();
}

void t5() {
    T(includes_tier_field);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_language_matrix", {{"language", "cpp"}});
    C(out["rows"][0].contains("tier"), "tier missing");
    C(out["rows"][0].value("tier", "") == "stable", "cpp should be stable");
    P();
}

void t6() {
    T(includes_gate_status);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_language_matrix", {{"language", "cpp"}});
    C(out["rows"][0].contains("gateStatus"), "gateStatus missing");
    C(out["rows"][0]["gateStatus"].value("performanceGates", false), "stable perf gate should be true");
    P();
}

void t7() {
    T(unknown_language_response);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_language_matrix", {{"language", "unknown_lang"}});
    C(out.value("success", false), "expected success with warning");
    C(out["rows"].empty(), "unknown should return empty rows");
    C(out.value("warning", "") == "unknown_language", "warning missing");
    P();
}

void t8() {
    T(deterministic_sort_order);
    MCPServer mcp;
    auto a = callTool(mcp, "whetstone_get_language_matrix", json::object());
    auto b = callTool(mcp, "whetstone_get_language_matrix", json::object());
    C(a.dump() == b.dump(), "nondeterministic tool output");
    P();
}

int main() {
    std::cout << "Step 694: language matrix MCP tool\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
