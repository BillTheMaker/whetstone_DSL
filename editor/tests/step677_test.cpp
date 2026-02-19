// Step 677: whetstone_assemble_context MCP tool (8 tests)

#include "MCPServer.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;
namespace fs = std::filesystem;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; }

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

static json loadToolsJson() {
    std::ifstream in("tools/claude/tools.json");
    if (!in.good()) in.open("../tools/claude/tools.json");
    if (!in.good()) in.open("/home/bill/Documents/CLionProjects/whetstone_DSL/tools/claude/tools.json");
    if (!in.good()) return json::object();
    json d;
    in >> d;
    return d;
}

static std::string writeFixtureFile() {
    const fs::path p = fs::path("editor/tests/step677_fixture.cpp");
    std::ofstream out(p);
    out << "int target_fn() {\n";
    out << "  return 7;\n";
    out << "}\n";
    out << "int other_fn() { return 1; }\n";
    out.close();
    return p.generic_string();
}

void t1() {
    TEST(tool_registered);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_assemble_context") { found = true; break; }
    }
    CHECK(found, "tool missing");
    PASS();
}

void t2() {
    TEST(empty_files_returns_success_empty_slices);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_assemble_context", {{"files", json::array()}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out.contains("slices"), "slices missing");
    CHECK(out["slices"].empty(), "slices should be empty");
    PASS();
}

void t3() {
    TEST(valid_file_path_returns_slice);
    MCPServer mcp;
    const std::string fixture = writeFixtureFile();
    auto out = callTool(mcp, "whetstone_assemble_context",
                        {{"files", json::array({{{"path", fixture}, {"head_lines", 2}}})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(!out["slices"].empty(), "expected non-empty slices");
    PASS();
}

void t4() {
    TEST(missing_path_arg_returns_error);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_assemble_context",
                        {{"files", json::array({{{"symbol", "target_fn"}}})}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t5() {
    TEST(symbol_slicing_returns_content);
    MCPServer mcp;
    const std::string fixture = writeFixtureFile();
    auto out = callTool(mcp, "whetstone_assemble_context",
                        {{"files", json::array({{{"path", fixture}, {"symbol", "target_fn"}}})}});
    CHECK(out.value("success", false), "expected success");
    CHECK(!out["slices"].empty(), "slice missing");
    CHECK(out["slices"][0].value("content", "").find("target_fn") != std::string::npos,
          "symbol content missing");
    PASS();
}

void t6() {
    TEST(budget_report_present_in_output);
    MCPServer mcp;
    const std::string fixture = writeFixtureFile();
    auto out = callTool(mcp, "whetstone_assemble_context",
                        {{"files", json::array({{{"path", fixture}, {"head_lines", 2}}})}});
    CHECK(out.contains("budget_report"), "budget_report missing");
    PASS();
}

void t7() {
    TEST(budget_exceeded_false_when_fit);
    MCPServer mcp;
    const std::string fixture = writeFixtureFile();
    auto out = callTool(mcp, "whetstone_assemble_context",
                        {{"files", json::array({{{"path", fixture}, {"head_lines", 2}}})},
                         {"max_tokens", 1000}});
    CHECK(out["budget_report"].value("budget_exceeded", true) == false, "budget should fit");
    PASS();
}

void t8() {
    TEST(tools_json_tool_count_is_87);
    auto d = loadToolsJson();
    CHECK(d.contains("tools"), "tools missing");
    CHECK(d["tools"].size() == 87, "expected 87 tools");
    PASS();
}

int main() {
    std::cout << "Step 677: whetstone_assemble_context MCP tool\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

