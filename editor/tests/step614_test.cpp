// Step 614: whetstone_architect_intake MCP Tool (12 tests)

#include "MCPServer.h"

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
        {"params", {
            {"name", name},
            {"arguments", args}
        }}
    };
    json resp = mcp.handleRequest(req);
    std::string text = resp["result"]["content"][0].value("text", "{}");
    return json::parse(text);
}

static std::string validSpec() {
    return
        "## Goals\n"
        "- Build reporting dashboard\n"
        "\n"
        "## Constraints\n"
        "- Do not use dynamic allocation in hot path\n"
        "- Use dynamic allocation in hot path for cache warmup\n"
        "\n"
        "## Dependencies\n"
        "- sqlite\n"
        "\n"
        "## Acceptance Criteria\n"
        "- p95 latency under 50ms\n"
        "- maybe include export csv support\n";
}

void test_tool_registered_accessor() {
    TEST(tool_registered_accessor);
    MCPServer mcp;
    bool found = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name == "whetstone_architect_intake") found = true;
    }
    CHECK(found, "tool should be registered");
    PASS();
}

void test_tool_visible_in_tools_list() {
    TEST(tool_visible_in_tools_list);
    MCPServer mcp;
    json req = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/list"}};
    json resp = mcp.handleRequest(req);
    bool found = false;
    for (const auto& tool : resp["result"]["tools"]) {
        if (tool.value("name", "") == "whetstone_architect_intake") found = true;
    }
    CHECK(found, "tools/list should include tool");
    PASS();
}

void test_tool_schema_requires_markdown() {
    TEST(tool_schema_requires_markdown);
    MCPServer mcp;
    bool hasRequired = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name != "whetstone_architect_intake") continue;
        if (tool.inputSchema.contains("required")) {
            for (const auto& req : tool.inputSchema["required"]) {
                if (req == "markdown") hasRequired = true;
            }
        }
    }
    CHECK(hasRequired, "input schema should require markdown");
    PASS();
}

void test_success_for_valid_markdown() {
    TEST(success_for_valid_markdown);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", validSpec()}});
    CHECK(out.value("success", false), "success should be true");
    PASS();
}

void test_parsed_spec_has_expected_sections() {
    TEST(parsed_spec_has_expected_sections);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", validSpec()}});
    int sections = (int)out["parsedSpec"]["sections"].size();
    CHECK(sections == 4, "expected 4 sections");
    PASS();
}

void test_normalized_requirement_count_matches_inputs() {
    TEST(normalized_requirement_count_matches_inputs);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", validSpec()}});
    int count = (int)out["normalizedRequirements"].size();
    CHECK(count == 6, "expected 6 normalized requirements");
    PASS();
}

void test_detects_constraint_conflict_signal() {
    TEST(detects_constraint_conflict_signal);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", validSpec()}});
    CHECK(out["conflictSignals"].value("hasConflicts", false), "expected conflict signal");
    CHECK(out["conflictSignals"].value("conflictCount", 0) >= 1, "expected conflict count >= 1");
    PASS();
}

void test_detects_ambiguous_requirement_count() {
    TEST(detects_ambiguous_requirement_count);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", validSpec()}});
    CHECK(out["conflictSignals"].value("ambiguousRequirementCount", 0) >= 1,
          "expected ambiguous requirement count >= 1");
    PASS();
}

void test_missing_markdown_field_errors() {
    TEST(missing_markdown_field_errors);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake", json::object());
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "markdown_missing", "wrong error");
    PASS();
}

void test_markdown_must_be_string() {
    TEST(markdown_must_be_string);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake", {{"markdown", 123}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "markdown_not_string", "wrong error");
    PASS();
}

void test_parser_rejects_no_sections() {
    TEST(parser_rejects_no_sections);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", "- only bullet no section"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "no_sections_found", "wrong error");
    PASS();
}

void test_normalize_rejects_no_requirements() {
    TEST(normalize_rejects_no_requirements);
    MCPServer mcp;
    std::string spec = "## Goals\nNarrative only.\n";
    json out = callTool(mcp, "whetstone_architect_intake",
                        {{"markdown", spec}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "no_requirements_found", "wrong error");
    CHECK(out.contains("parsedSpec"), "parsed spec should be included on normalize failure");
    PASS();
}

int main() {
    std::cout << "Step 614: whetstone_architect_intake MCP Tool\n";

    test_tool_registered_accessor();                       // 1
    test_tool_visible_in_tools_list();                    // 2
    test_tool_schema_requires_markdown();                 // 3
    test_success_for_valid_markdown();                    // 4
    test_parsed_spec_has_expected_sections();             // 5
    test_normalized_requirement_count_matches_inputs();   // 6
    test_detects_constraint_conflict_signal();            // 7
    test_detects_ambiguous_requirement_count();           // 8
    test_missing_markdown_field_errors();                 // 9
    test_markdown_must_be_string();                       // 10
    test_parser_rejects_no_sections();                    // 11
    test_normalize_rejects_no_requirements();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
