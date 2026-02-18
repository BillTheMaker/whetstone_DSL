// Step 616: whetstone_queue_ready MCP Tool (12 tests)

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
        "- Ship queue simulation\n"
        "\n"
        "## Constraints\n"
        "- Keep APIs deterministic\n"
        "\n"
        "## Dependencies\n"
        "- cmake\n"
        "\n"
        "## Acceptance Criteria\n"
        "- queue has coverage checks\n";
}

static std::string riskySpec() {
    return
        "## Goals\n"
        "- maybe improve workflow maybe maybe\n"
        "\n"
        "## Constraints\n"
        "- Do not use dynamic allocation in hot path\n"
        "- Use dynamic allocation in hot path for warmup\n"
        "\n"
        "## Dependencies\n"
        "- maybe add another dependency\n"
        "\n"
        "## Acceptance Criteria\n"
        "- maybe pass tests\n";
}

static json intakeThenGenerate(MCPServer& mcp, const std::string& markdown) {
    json intake = callTool(mcp, "whetstone_architect_intake", {{"markdown", markdown}});
    return {
        {"intake", intake},
        {"generated", callTool(mcp, "whetstone_generate_taskitems", {
            {"normalizedRequirements", intake["normalizedRequirements"]},
            {"conflicts", intake["conflicts"]}
        })}
    };
}

void test_tool_registered_accessor() {
    TEST(tool_registered_accessor);
    MCPServer mcp;
    bool found = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name == "whetstone_queue_ready") found = true;
    }
    CHECK(found, "tool should be registered");
    PASS();
}

void test_tool_schema_requires_tasks() {
    TEST(tool_schema_requires_tasks);
    MCPServer mcp;
    bool required = false;
    for (const auto& tool : mcp.getTools()) {
        if (tool.name != "whetstone_queue_ready") continue;
        for (const auto& req : tool.inputSchema["required"]) {
            if (req == "tasks") required = true;
        }
    }
    CHECK(required, "tasks should be required");
    PASS();
}

void test_success_with_generated_tasks_and_requirements() {
    TEST(success_with_generated_tasks_and_requirements);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]},
        {"normalizedRequirements", flow["intake"]["normalizedRequirements"]}
    });
    CHECK(out.value("success", false), "success should be true");
    PASS();
}

void test_ready_count_positive_for_clear_spec() {
    TEST(ready_count_positive_for_clear_spec);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]},
        {"normalizedRequirements", flow["intake"]["normalizedRequirements"]}
    });
    CHECK(out.value("readyCount", 0) >= 1, "readyCount should be >= 1");
    PASS();
}

void test_queue_json_contains_coverage_checks() {
    TEST(queue_json_contains_coverage_checks);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]},
        {"normalizedRequirements", flow["intake"]["normalizedRequirements"]}
    });
    CHECK(out["queueJson"].is_array() && !out["queueJson"].empty(), "queueJson should be populated");
    CHECK(out["queueJson"][0].value("hasCoverage", false), "first queue item should have coverage");
    CHECK(out["queueJson"][0]["checks"].is_array() && !out["queueJson"][0]["checks"].empty(),
          "first queue item should include checks");
    PASS();
}

void test_risky_spec_surfaces_escalations() {
    TEST(risky_spec_surfaces_escalations);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, riskySpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]},
        {"normalizedRequirements", flow["intake"]["normalizedRequirements"]}
    });
    CHECK(out.value("escalateCount", 0) >= 1, "escalateCount should be >= 1");
    PASS();
}

void test_risky_spec_not_ready_due_to_blockers() {
    TEST(risky_spec_not_ready_due_to_blockers);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, riskySpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]},
        {"normalizedRequirements", flow["intake"]["normalizedRequirements"]}
    });
    bool hasEscalationBlocker = false;
    for (const auto& blocker : out["blockers"]) {
        if (blocker == "escalations_present") hasEscalationBlocker = true;
    }
    CHECK(!out.value("ready", true), "ready should be false");
    CHECK(hasEscalationBlocker, "escalation blocker expected");
    PASS();
}

void test_missing_tasks_errors() {
    TEST(missing_tasks_errors);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_queue_ready", json::object());
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "tasks_missing", "wrong error");
    PASS();
}

void test_tasks_must_be_array() {
    TEST(tasks_must_be_array);
    MCPServer mcp;
    json out = callTool(mcp, "whetstone_queue_ready", {{"tasks", "bad"}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "tasks_not_array", "wrong error");
    PASS();
}

void test_rejects_task_entries_missing_fields() {
    TEST(rejects_task_entries_missing_fields);
    MCPServer mcp;
    json badTasks = json::array({{{"taskId", "task-1"}}});
    json out = callTool(mcp, "whetstone_queue_ready", {{"tasks", badTasks}});
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "task_entry_missing_fields", "wrong error");
    PASS();
}

void test_without_requirements_reports_acceptance_blocker() {
    TEST(without_requirements_reports_acceptance_blocker);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]}
    });
    bool hasAcceptanceBlocker = false;
    for (const auto& blocker : out["blockers"]) {
        if (blocker == "acceptance_requirements_empty") hasAcceptanceBlocker = true;
    }
    CHECK(out.value("success", false), "success should be true");
    CHECK(hasAcceptanceBlocker, "acceptance blocker expected");
    CHECK(!out.value("ready", true), "ready should be false");
    PASS();
}

void test_invalid_normalized_requirements_type_errors() {
    TEST(invalid_normalized_requirements_type_errors);
    MCPServer mcp;
    json flow = intakeThenGenerate(mcp, clearSpec());
    json out = callTool(mcp, "whetstone_queue_ready", {
        {"tasks", flow["generated"]["tasks"]},
        {"normalizedRequirements", "bad"}
    });
    CHECK(!out.value("success", true), "success should be false");
    CHECK(out.value("error", "") == "normalized_requirements_not_array", "wrong error");
    PASS();
}

int main() {
    std::cout << "Step 616: whetstone_queue_ready MCP Tool\n";

    test_tool_registered_accessor();                          // 1
    test_tool_schema_requires_tasks();                        // 2
    test_success_with_generated_tasks_and_requirements();     // 3
    test_ready_count_positive_for_clear_spec();               // 4
    test_queue_json_contains_coverage_checks();               // 5
    test_risky_spec_surfaces_escalations();                   // 6
    test_risky_spec_not_ready_due_to_blockers();              // 7
    test_missing_tasks_errors();                              // 8
    test_tasks_must_be_array();                               // 9
    test_rejects_task_entries_missing_fields();               // 10
    test_without_requirements_reports_acceptance_blocker();   // 11
    test_invalid_normalized_requirements_type_errors();       // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
