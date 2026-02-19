// Step 682: whetstone_validate_taskitem MCP tool (8 tests)

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

static std::string makeWorkspace() {
    fs::path root = fs::temp_directory_path() / "whetstone_step682_workspace";
    std::error_code ec;
    fs::remove_all(root, ec);
    fs::create_directories(root / "editor/src", ec);
    std::ofstream(root / "editor/src/file.h") << "int x();\n";
    return root.string();
}

static json validTaskitem() {
    return {
        {"task_id", "T1"},
        {"title", "Do thing"},
        {"prerequisite_ops", json::array({"read editor/src/file.h"})},
        {"reasons", json::array({"r1", "r2", "r3"})},
        {"confidence", 90},
        {"dependency_task_ids", json::array()}
    };
}

void t1() {
    TEST(tool_registered);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/list"}};
    auto resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& t : resp["result"]["tools"]) {
        if (t.value("name", "") == "whetstone_validate_taskitem") { found = true; break; }
    }
    CHECK(found, "tool missing");
    PASS();
}

void t2() {
    TEST(empty_taskitems_returns_zero_report);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_validate_taskitem", {{"taskitems", json::array()}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["report"].value("total_taskitems", -1) == 0, "expected total 0");
    PASS();
}

void t3() {
    TEST(single_valid_taskitem_returns_score);
    MCPServer mcp;
    auto workspace = makeWorkspace();
    auto out = callTool(mcp, "whetstone_validate_taskitem",
                        {{"taskitems", json::array({validTaskitem()})}, {"workspace", workspace}});
    CHECK(out.value("success", false), "expected success");
    CHECK(out["report"]["results"].size() == 1, "expected one result");
    PASS();
}

void t4() {
    TEST(missing_task_id_returns_error);
    MCPServer mcp;
    auto out = callTool(mcp, "whetstone_validate_taskitem",
                        {{"taskitems", json::array({json::object()})}});
    CHECK(!out.value("success", true), "expected failure");
    PASS();
}

void t5() {
    TEST(prerequisite_ops_resolved_against_workspace);
    MCPServer mcp;
    auto workspace = makeWorkspace();
    auto out = callTool(mcp, "whetstone_validate_taskitem",
                        {{"taskitems", json::array({validTaskitem()})}, {"workspace", workspace}});
    CHECK(out.value("success", false), "expected success");
    int score = out["report"]["results"][0].value("score", 0);
    CHECK(score >= 80, "expected resolved-op high score");
    PASS();
}

void t6() {
    TEST(results_length_matches_input_length);
    MCPServer mcp;
    auto workspace = makeWorkspace();
    auto out = callTool(mcp, "whetstone_validate_taskitem",
                        {{"taskitems", json::array({validTaskitem(), validTaskitem()})},
                         {"workspace", workspace}});
    CHECK(out["report"]["results"].size() == 2, "expected two results");
    PASS();
}

void t7() {
    TEST(category_counts_sum_to_total);
    MCPServer mcp;
    auto workspace = makeWorkspace();
    auto out = callTool(mcp, "whetstone_validate_taskitem",
                        {{"taskitems", json::array({validTaskitem()})}, {"workspace", workspace}});
    const auto& r = out["report"];
    int total = r.value("total_taskitems", 0);
    int sum = r.value("self_contained_count", 0) + r.value("warning_count", 0) + r.value("failing_count", 0);
    CHECK(total == sum, "count sum mismatch");
    PASS();
}

void t8() {
    TEST(tool_count_updated_to_88);
    auto d = loadToolsJson();
    CHECK(d.contains("tools"), "tools missing");
    CHECK(d["tools"].size() == 88, "expected 88 tools");
    PASS();
}

int main() {
    std::cout << "Step 682: whetstone_validate_taskitem MCP tool\n";
    t1(); t2(); t3(); t4(); t5(); t6(); t7(); t8();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}

