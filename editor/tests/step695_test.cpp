// Step 695: whetstone_get_porting_contract MCP tool (8 tests)

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
    T(tool_registered);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_get_porting_contract") { found = true; break; }
    }
    C(found, "tool missing");
    P();
}

void t2() {
    T(pair_lookup_works);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "rust"}, {"targetLanguage", "cpp"}});
    C(out.value("success", false), "expected success");
    C(out.value("pairFound", false), "pair should be found");
    P();
}

void t3() {
    T(default_thresholds);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "python"}, {"targetLanguage", "go"}});
    C(out.value("success", false), "expected success");
    C(!out.value("pairFound", true), "pair should not be found");
    C(out["thresholds"].value("minimumTestPassRate", 0.0f) == 1.0f, "default min test rate mismatch");
    P();
}

void t4() {
    T(override_thresholds);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_porting_contract",
                        {{"sourceLanguage", "rust"},
                         {"targetLanguage", "cpp"},
                         {"overrides", {{"maxPerfRegressionPct", 2.5}, {"maxHighSeverityFindings", 1}}}});
    C(out.value("success", false), "expected success");
    C(out["thresholds"].value("maxPerfRegressionPct", 0.0f) == 2.5f, "override perf mismatch");
    C(out["thresholds"].value("maxHighSeverityFindings", -1) == 1, "override security mismatch");
    P();
}

void t5() {
    T(missing_pair_fallback);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "unknown"}, {"targetLanguage", "unknown"}});
    C(out.value("success", false), "expected success fallback");
    C(!out.value("pairFound", true), "expected pairFound false");
    P();
}

void t6() {
    T(error_on_invalid_payload);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "rust"}});
    C(!out.value("success", true), "expected failure");
    C(out.value("error", "") == "target_language_missing", "wrong error");
    P();
}

void t7() {
    T(machine_readable_gate_list);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "rust"}, {"targetLanguage", "cpp"}});
    C(out.contains("requiredGates"), "requiredGates missing");
    C(out["requiredGates"].is_array(), "requiredGates not array");
    C(!out["requiredGates"].empty(), "requiredGates empty");
    P();
}

void t8() {
    T(deterministic_output);
    MCPServer mcp;
    auto a = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "rust"}, {"targetLanguage", "cpp"}});
    auto b = callTool(mcp, "whetstone_get_porting_contract", {{"sourceLanguage", "rust"}, {"targetLanguage", "cpp"}});
    C(a.dump() == b.dump(), "nondeterministic output");
    P();
}

int main() {
    std::cout << "Step 695: porting contract MCP tool\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << p << "/" << (p + f) << " passed\n";
    return f ? 1 : 0;
}
