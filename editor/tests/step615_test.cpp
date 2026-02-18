// Step 615: whetstone_generate_taskitems MCP Tool (12 tests)

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
        {"params", {{"name", name}, {"arguments", args}}}
    };
    json resp = mcp.handleRequest(req);
    std::string text = resp["result"]["content"][0].value("text", "{}");
    return json::parse(text);
}

static std::string clearSpec() {
    return
        "## Goals\n"
        "- Ship intake parser\n"
        "\n"
        "## Constraints\n"
        "- Keep modules header only\n"
        "\n"
        "## Dependencies\n"
        "- nlohmann json\n"
        "\n"
        "## Acceptance Criteria\n"
        "- tests pass\n";
}

static std::string riskySpec() {
    return
        "## Goals\n"
        "- maybe improve intake and maybe improve review\n"
        "\n"
        "## Constraints\n"
        "- Do not use dynamic allocation in hot path\n"
        "- Use dynamic allocation in hot path for warmup\n"
        "\n"
        "## Dependencies\n"
        "- maybe add cache lib\n"
        "\n"
        "## Acceptance Criteria\n"
        "- maybe latency under 50ms\n";
}

static json intakeOutput(MCPServer& mcp, const std::string& markdown) {
    return callTool(mcp, "whetstone_architect_intake", {{"markdown", markdown}});
}

static json taskitemArgsFromIntake(const json& intake) {
    return {
        {"normalizedRequirements", intake["normalizedRequirements"]},
        {"conflicts", intake["conflicts"]}
    };
}

void test_tool_registered_accessor() {
    TEST(tool_registered_accessor);
    MCPServer mcp;
    bool found = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name == "whetstone_generate_taskitems") found = true;
    }
    CHECK(found, "tool should be registered");
    PASS();
}

void test_tool_schema_requires_normalized_requirements() {
    TEST(tool_schema_requires_normalized_requirements);
    MCPServer mcp;
    bool required = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name != "whetstone_generate_taskitems") continue;
        for (const auto& req : tool.inputSchema["required"]) {
            if (req == "normalizedRequirements") required = true;
        }
    }
    CHECK(required, "normalizedRequirements should be required");
    PASS();
}

void test_success_for_intake_output() {
    TEST(success_for_intake_output);
    MCPServer mcp;
    json intake = intakeOutput(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_generate_taskitems", taskitemArgsFromIntake(intake));
    CHECK(out.value("success", false), "success should be true");
    PASS();
}

void test_returns_non_empty_tasks() {
    TEST(returns_non_empty_tasks);
    MCPServer mcp;
    json intake = intakeOutput(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_generate_taskitems", taskitemArgsFromIntake(intake));
    CHECK(out["tasks"].is_array() && !out["tasks"].empty(), "tasks should be non-empty array");
    PASS();
}

void test_task_contains_annotation_fields() {
    TEST(task_contains_annotation_fields);
    MCPServer mcp;
    json intake = intakeOutput(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_generate_taskitems", taskitemArgsFromIntake(intake));
    const auto& task = out["tasks"][0];
    CHECK(task.contains("confidence"), "confidence missing");
    CHECK(task.contains("ambiguityCount"), "ambiguityCount missing");
    CHECK(task.contains("escalate"), "escalate missing");
    CHECK(task.contains("reasons"), "reasons missing");
    PASS();
}

void test_conflict_count_roundtrips_from_input() {
    TEST(conflict_count_roundtrips_from_input);
    MCPServer mcp;
    json intake = intakeOutput(mcp, riskySpec());
    json out = callTool(mcp, "whetstone_generate_taskitems", taskitemArgsFromIntake(intake));
    CHECK(out.value("conflictCount", 0) >= 1, "expected conflict count >= 1");
    PASS();
}

void test_risky_input_triggers_escalation() {
    TEST(risky_input_triggers_escalation);
    MCPServer mcp;
    json intake = intakeOutput(mcp, riskySpec());
    json out = callTool(mcp, "whetstone_generate_taskitems", taskitemArgsFromIntake(intake));
    CHECK(out.value("escalateCount", 0) >= 1, "expected escalations");
    PASS();
}

void test_clear_input_has_non_escalated_task() {
    TEST(clear_input_has_non_escalated_task);
    MCPServer mcp;
    json intake = intakeOutput(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_generate_taskitems", taskitemArgsFromIntake(intake));
    bool foundNonEscalated = false;
    for (const auto& task : out["tasks"]) {
        if (!task.value("escalate", true)) foundNonEscalated = true;
    }
    CHECK(foundNonEscalated, "expected at least one non-escalated task");
    PASS();
}

void test_missing_normalized_requirements_errors() {
    TEST(missing_normalized_requirements_errors);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_generate_taskitems", json::object());
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "normalized_requirements_missing", "wrong error");
    PASS();
}

void test_normalized_requirements_must_be_array() {
    TEST(normalized_requirements_must_be_array);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_generate_taskitems",
                        {{"normalizedRequirements", "bad"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "normalized_requirements_not_array", "wrong error");
    PASS();
}

void test_rejects_empty_normalized_requirements() {
    TEST(rejects_empty_normalized_requirements);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_generate_taskitems",
                        {{"normalizedRequirements", json::array()}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "normalized_requirements_empty", "wrong error");
    PASS();
}

void test_rejects_invalid_requirement_kind() {
    TEST(rejects_invalid_requirement_kind);
    MCPServer mcp;
    json bad = json::array({{
        {"requirementId", "goal-1"},
        {"kind", "invalid-kind"},
        {"normalizedText", "ship it"},
        {"anchor", "goals"},
        {"sourceLine", 2},
        {"ambiguous", false}
    }});
    json out = callTool(mcp, "whetstone_generate_taskitems",
                        {{"normalizedRequirements", bad}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "requirement_kind_invalid", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 615: whetstone_generate_taskitems MCP Tool\n";

    test_tool_registered_accessor();                           // 1
    test_tool_schema_requires_normalized_requirements();      // 2
    test_success_for_intake_output();                         // 3
    test_returns_non_empty_tasks();                           // 4
    test_task_contains_annotation_fields();                   // 5
    test_conflict_count_roundtrips_from_input();             // 6
    test_risky_input_triggers_escalation();                  // 7
    test_clear_input_has_non_escalated_task();               // 8
    test_missing_normalized_requirements_errors();           // 9
    test_normalized_requirements_must_be_array();            // 10
    test_rejects_empty_normalized_requirements();            // 11
    test_rejects_invalid_requirement_kind();                 // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
