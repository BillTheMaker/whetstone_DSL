// Step 419: Agent Workflow Loop Tests (12 tests)

#include "MCPAgentWorkflowLoop.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

struct RpcRecorder {
    std::vector<std::string> methods;
    std::vector<json> params;
    json operator()(const json& request) {
        std::string method = request.value("method", "");
        json p = request.value("params", json::object());
        methods.push_back(method);
        params.push_back(p);

        if (method == "getBlockers") {
            return {{"jsonrpc", "2.0"}, {"id", 1}, {"result", {{"blockers", json::array()}}}};
        }
        if (method == "getProgress") {
            return {{"jsonrpc", "2.0"}, {"id", 1},
                    {"result", {{"percentComplete", 100}, {"status", "complete"}}}};
        }
        return {{"jsonrpc", "2.0"}, {"id", 1}, {"result", json::object()}};
    }
};

void test_loop_calls_expected_tools_without_blockers() {
    TEST(loop_calls_expected_tools_without_blockers);
    MCPServer server;
    RpcRecorder rec;
    server.setRpcCallback([&](const json& req){ return rec(req); });
    MCPAgentWorkflowLoop loop(server);

    auto report = loop.run({}, [&](const MCPAgentWorkflowLoop::Blocker&) {
        return MCPAgentWorkflowLoop::AgentSubmission{};
    });
    CHECK(report.success, "expected success");
    CHECK(report.blockersProcessed == 0, "expected zero blockers processed");
    CHECK(rec.methods.size() == 6, "expected six RPC calls without submit");
    PASS();
}

void test_loop_processes_single_blocker_and_submits_result() {
    TEST(loop_processes_single_blocker_and_submits_result);
    MCPServer server;
    std::vector<std::string> methods;
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        methods.push_back(m);
        if (m == "getBlockers") {
            return {{"jsonrpc", "2.0"}, {"id", 1},
                    {"result", {{"blockers", json::array({
                        {{"itemId", "w1"}, {"kind", "needs-external-model"}, {"prompt", "Implement W1"}}
                    })}}}};
        }
        if (m == "getProgress") {
            return {{"jsonrpc", "2.0"}, {"id", 1},
                    {"result", {{"percentComplete", 75}, {"status", "in-progress"}}}};
        }
        return {{"jsonrpc", "2.0"}, {"id", 1}, {"result", json::object()}};
    });

    MCPAgentWorkflowLoop loop(server);
    auto report = loop.run({}, [&](const MCPAgentWorkflowLoop::Blocker& b) {
        MCPAgentWorkflowLoop::AgentSubmission s;
        s.generatedCode = "// completed " + b.itemId;
        s.reasoning = "done";
        s.confidence = 0.92;
        return s;
    });

    CHECK(report.blockersProcessed == 1, "expected one processed blocker");
    bool submitted = false;
    for (const auto& m : methods) if (m == "submitExternalResult") submitted = true;
    CHECK(submitted, "expected submitExternalResult call");
    PASS();
}

void test_loop_processes_multiple_blockers() {
    TEST(loop_processes_multiple_blockers);
    MCPServer server;
    int submits = 0;
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        if (m == "getBlockers") {
            return {{"jsonrpc", "2.0"}, {"id", 1},
                    {"result", {{"blockers", json::array({
                        {{"itemId", "w1"}, {"prompt", "p1"}},
                        {{"itemId", "w2"}, {"prompt", "p2"}}
                    })}}}};
        }
        if (m == "submitExternalResult") submits++;
        if (m == "getProgress") {
            return {{"jsonrpc", "2.0"}, {"id", 1}, {"result", {{"percentComplete", 60}}}};
        }
        return {{"jsonrpc", "2.0"}, {"id", 1}, {"result", json::object()}};
    });

    MCPAgentWorkflowLoop loop(server);
    auto report = loop.run({}, [&](const MCPAgentWorkflowLoop::Blocker&) {
        return MCPAgentWorkflowLoop::AgentSubmission{"code", 0.8, "ok"};
    });

    CHECK(report.blockersProcessed == 2, "expected two blockers processed");
    CHECK(submits == 2, "expected two submit calls");
    PASS();
}

void test_loop_propagates_config_values_to_calls() {
    TEST(loop_propagates_config_values_to_calls);
    MCPServer server;
    std::vector<json> skeletonParams;
    std::vector<json> workflowParams;
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        if (m == "createSkeleton") skeletonParams.push_back(req.value("params", json::object()));
        if (m == "createWorkflow") workflowParams.push_back(req.value("params", json::object()));
        if (m == "getBlockers") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"blockers",json::array()}}}};
        if (m == "getProgress") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"percentComplete",100}}}};
        return {{"jsonrpc","2.0"},{"id",1},{"result",json::object()}};
    });
    MCPAgentWorkflowLoop loop(server);
    MCPAgentWorkflowLoop::Config cfg;
    cfg.projectName = "Alpha";
    cfg.skeletonName = "AlphaSkeleton";
    cfg.language = "python";
    loop.run(cfg, nullptr);

    CHECK(!skeletonParams.empty(), "missing createSkeleton params");
    CHECK(!workflowParams.empty(), "missing createWorkflow params");
    CHECK(skeletonParams[0].value("name", "") == "AlphaSkeleton", "wrong skeleton name");
    CHECK(skeletonParams[0].value("language", "") == "python", "wrong language");
    CHECK(workflowParams[0].value("projectName", "") == "Alpha", "wrong project name");
    PASS();
}

void test_parse_blockers_accepts_itemId_format() {
    TEST(parse_blockers_accepts_itemId_format);
    json r = {{"blockers", json::array({{{"itemId", "w1"}, {"kind", "k"}, {"prompt", "p"}}})}};
    auto v = MCPAgentWorkflowLoop::parseBlockers(r);
    CHECK(v.size() == 1, "expected one blocker");
    CHECK(v[0].itemId == "w1", "wrong blocker id");
    PASS();
}

void test_parse_blockers_accepts_id_alias() {
    TEST(parse_blockers_accepts_id_alias);
    json r = {{"blockers", json::array({{{"id", "w9"}, {"type", "human"}, {"reason", "needs review"}}})}};
    auto v = MCPAgentWorkflowLoop::parseBlockers(r);
    CHECK(v.size() == 1, "expected one blocker");
    CHECK(v[0].itemId == "w9", "wrong alias id");
    CHECK(v[0].kind == "human", "wrong alias kind");
    PASS();
}

void test_parse_blockers_ignores_invalid_entries() {
    TEST(parse_blockers_ignores_invalid_entries);
    json r = {{"blockers", json::array({{{"prompt", "missing id"}}, {{"itemId", "ok"}}})}};
    auto v = MCPAgentWorkflowLoop::parseBlockers(r);
    CHECK(v.size() == 1, "expected one valid blocker");
    CHECK(v[0].itemId == "ok", "expected valid item");
    PASS();
}

void test_report_tracks_tool_call_order() {
    TEST(report_tracks_tool_call_order);
    MCPServer server;
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        if (m == "getBlockers") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"blockers",json::array()}}}};
        if (m == "getProgress") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"percentComplete",100}}}};
        return {{"jsonrpc","2.0"},{"id",1},{"result",json::object()}};
    });
    MCPAgentWorkflowLoop loop(server);
    auto report = loop.run({}, nullptr);
    CHECK(report.toolCalls.size() == 6, "expected six tool calls");
    CHECK(report.toolCalls.front() == "whetstone_infer_annotations", "first call mismatch");
    CHECK(report.toolCalls.back() == "whetstone_get_progress", "last call mismatch");
    PASS();
}

void test_agent_handler_receives_blocker_payload() {
    TEST(agent_handler_receives_blocker_payload);
    MCPServer server;
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        if (m == "getBlockers") {
            return {{"jsonrpc","2.0"},{"id",1},
                    {"result",{{"blockers",json::array({
                        {{"itemId","w42"},{"kind","needs-external-model"},{"prompt","Write adapter"}}
                    })}}}};
        }
        if (m == "getProgress") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"percentComplete",50}}}};
        return {{"jsonrpc","2.0"},{"id",1},{"result",json::object()}};
    });
    MCPAgentWorkflowLoop loop(server);
    std::string seenId, seenPrompt;
    loop.run({}, [&](const MCPAgentWorkflowLoop::Blocker& b) {
        seenId = b.itemId;
        seenPrompt = b.prompt;
        return MCPAgentWorkflowLoop::AgentSubmission{"code", 0.7, "reason"};
    });
    CHECK(seenId == "w42", "blocker id not delivered");
    CHECK(seenPrompt == "Write adapter", "blocker prompt not delivered");
    PASS();
}

void test_submit_result_contains_generated_code_and_confidence() {
    TEST(submit_result_contains_generated_code_and_confidence);
    MCPServer server;
    json submitParams = json::object();
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        if (m == "getBlockers") {
            return {{"jsonrpc","2.0"},{"id",1},
                    {"result",{{"blockers",json::array({{{"itemId","w1"},{"prompt","do"}}})}}}};
        }
        if (m == "submitExternalResult") submitParams = req.value("params", json::object());
        if (m == "getProgress") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"percentComplete",80}}}};
        return {{"jsonrpc","2.0"},{"id",1},{"result",json::object()}};
    });
    MCPAgentWorkflowLoop loop(server);
    loop.run({}, [&](const MCPAgentWorkflowLoop::Blocker&) {
        return MCPAgentWorkflowLoop::AgentSubmission{"int x = 1;", 0.91, "ok"};
    });
    CHECK(submitParams.value("itemId", "") == "w1", "missing item id");
    CHECK(submitParams["result"].value("generatedCode", "") == "int x = 1;", "missing code");
    CHECK(submitParams["result"].value("confidence", 0.0) > 0.9, "missing confidence");
    PASS();
}

void test_final_progress_is_returned_in_report() {
    TEST(final_progress_is_returned_in_report);
    MCPServer server;
    server.setRpcCallback([&](const json& req) -> json {
        std::string m = req.value("method", "");
        if (m == "getBlockers") return {{"jsonrpc","2.0"},{"id",1},{"result",{{"blockers",json::array()}}}};
        if (m == "getProgress") return {{"jsonrpc","2.0"},{"id",1},
                                        {"result",{{"percentComplete",100},{"etaMinutes",0}}}};
        return {{"jsonrpc","2.0"},{"id",1},{"result",json::object()}};
    });
    MCPAgentWorkflowLoop loop(server);
    auto report = loop.run({}, nullptr);
    CHECK(report.finalProgress.value("percentComplete", 0) == 100, "missing progress percent");
    CHECK(report.success, "report should mark success");
    PASS();
}

void test_tool_set_contains_required_workflow_tools() {
    TEST(tool_set_contains_required_workflow_tools);
    MCPServer server;
    auto tools = server.getTools();
    std::map<std::string, bool> needed = {
        {"whetstone_infer_annotations", false},
        {"whetstone_create_skeleton", false},
        {"whetstone_create_workflow", false},
        {"whetstone_orchestrate_run_deterministic", false},
        {"whetstone_get_blockers", false},
        {"whetstone_submit_result", false},
        {"whetstone_get_progress", false}
    };
    for (const auto& t : tools) {
        if (needed.count(t.name)) needed[t.name] = true;
    }
    for (const auto& [name, ok] : needed) {
        CHECK(ok, "missing required tool: " + name);
    }
    PASS();
}

int main() {
    std::cout << "Step 419: Agent Workflow Loop Tests\n";

    test_loop_calls_expected_tools_without_blockers();           // 1
    test_loop_processes_single_blocker_and_submits_result();     // 2
    test_loop_processes_multiple_blockers();                     // 3
    test_loop_propagates_config_values_to_calls();               // 4
    test_parse_blockers_accepts_itemId_format();                 // 5
    test_parse_blockers_accepts_id_alias();                      // 6
    test_parse_blockers_ignores_invalid_entries();               // 7
    test_report_tracks_tool_call_order();                        // 8
    test_agent_handler_receives_blocker_payload();               // 9
    test_submit_result_contains_generated_code_and_confidence(); // 10
    test_final_progress_is_returned_in_report();                 // 11
    test_tool_set_contains_required_workflow_tools();            // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
