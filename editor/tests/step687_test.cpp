// Step 687: whetstone_start_recording + whetstone_get_metrics MCP tools (8 tests)

#include "MCPServer.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

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

void t1() {
    TEST(start_recording_registered);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_start_recording") { found = true; break; }
    }
    CHECK(found, "start tool missing");
    PASS();
}

void t2() {
    TEST(get_metrics_registered);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_get_metrics") { found = true; break; }
    }
    CHECK(found, "get metrics tool missing");
    PASS();
}

void t3() {
    TEST(both_tools_in_tools_json);
    auto d = loadToolsJson();
    bool startFound = false, getFound = false;
    for (const auto& t : d["tools"]) {
        std::string n = t.value("name", "");
        if (n == "whetstone_start_recording") startFound = true;
        if (n == "whetstone_get_metrics") getFound = true;
    }
    CHECK(startFound && getFound, "missing tools in json");
    PASS();
}

void t4() {
    TEST(tool_count_updated_to_90);
    auto d = loadToolsJson();
    CHECK(d["tools"].size() == 90, "expected 90 tools");
    PASS();
}

void t5() {
    TEST(start_recording_returns_success);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_start_recording",
                        {{"session_id", "S1"}, {"task_description", "Task 1"}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out.value("session_id", "") == "S1", "session id mismatch");
    PASS();
}

void t6() {
    TEST(get_metrics_returns_session_record);
    MCPServer mcp;
    auto s = callTool(mcp, "whetstone_start_recording",
                      {{"session_id", "S2"}, {"task_description", "Task 2"}});
    CHECK(s.value("success", false), "start failed");
    (void)callTool(mcp, "whetstone_workspace_list", json::object());
    auto out = callTool(mcp, "whetstone_get_metrics", {{"session_id", "S2"}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out.contains("session"), "session missing");
    PASS();
}

void t7() {
    TEST(comparison_present_when_baseline_provided);
    MCPServer mcp;
    auto s = callTool(mcp, "whetstone_start_recording",
                      {{"session_id", "S3"}, {"task_description", "Task 3"}});
    CHECK(s.value("success", false), "start failed");
    (void)callTool(mcp, "whetstone_workspace_list", json::object());
    json baseline = {
        {"session_id", "B1"},
        {"task_description", "Task 3"},
        {"calls", json::array()},
        {"total_tool_calls", 3},
        {"total_estimated_tokens", 100},
        {"file_read_count", 2},
        {"total_duration_ms", 1000}
    };
    auto out = callTool(mcp, "whetstone_get_metrics",
                        {{"session_id", "S3"},
                         {"baseline", baseline},
                         {"quality_rating", 80},
                         {"baseline_quality", 75}});
    CHECK(out.contains("comparison"), "comparison missing");
    PASS();
}

void t8() {
    TEST(comparison_absent_without_baseline);
    MCPServer mcp;
    auto s = callTool(mcp, "whetstone_start_recording",
                      {{"session_id", "S4"}, {"task_description", "Task 4"}});
    CHECK(s.value("success", false), "start failed");
    auto out = callTool(mcp, "whetstone_get_metrics", {{"session_id", "S4"}});
    CHECK(!out.contains("comparison"), "comparison should be absent");
    PASS();
}

int main() {
    std::cout << "Step 687: MCP metrics tools\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

