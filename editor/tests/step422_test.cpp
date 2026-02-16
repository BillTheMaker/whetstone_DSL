// Step 422: Phase 18a Integration Tests (8 tests)

#include "MCPServer.h"
#include "MCPServerConfig.h"
#include "MCPWorkflowPrompts.h"
#include "HeadlessEditorState.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json rpcResult(const json& result, int id = 1) {
    return {{"jsonrpc", "2.0"}, {"id", id}, {"result", result}};
}

static json parseToolResult(const json& toolCallResponse) {
    if (!toolCallResponse.contains("result")) return json::object();
    auto content = toolCallResponse["result"].value("content", json::array());
    if (!content.is_array() || content.empty()) return json::object();
    std::string text = content[0].value("text", "{}");
    try {
        return json::parse(text);
    } catch (...) {
        return json::object();
    }
}

static json callTool(MCPServer& server, const std::string& name,
                     const json& args = json::object()) {
    auto resp = server.handleRequest({
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", "tools/call"},
        {"params", {{"name", name}, {"arguments", args}}}
    });
    return parseToolResult(resp);
}

static std::string makeTempDir(const std::string& name) {
    auto dir = std::filesystem::temp_directory_path() / ("whetstone_step422_" + name);
    std::filesystem::create_directories(dir);
    return dir.string();
}

static HeadlessEditorState makeStateWithReviewItem(const std::string& status = WI_REVIEW) {
    HeadlessEditorState state;
    state.setAgentRole("sess", AgentRole::Refactor);
    state.workflow = WorkflowState("phase18a-integration");

    WorkItem wi;
    wi.id = "w-review-1";
    wi.nodeId = "node1";
    wi.nodeName = "processOrder";
    wi.nodeType = "Function";
    wi.bufferId = "orders.py";
    wi.workerType = "llm";
    wi.priority = "high";
    wi.reviewRequired = true;
    wi.status = status;
    wi.result.generatedCode = "def process_order(x):\n    return x\n";
    wi.result.confidence = 0.85f;
    wi.result.reasoning = "Generated from workflow blocker.";
    state.workflow->queue.enqueue(wi);
    return state;
}

void test_mcp_config_build_for_phase18a() {
    TEST(mcp_config_build_for_phase18a);
    json cfg = MCPServerConfig::buildMcpConfig("/bin/whetstone_mcp", "/repo", "cpp");
    CHECK(cfg.contains("mcpServers"), "missing mcpServers");
    CHECK(cfg["mcpServers"].contains("whetstone"), "missing whetstone server");
    auto args = cfg["mcpServers"]["whetstone"]["args"].dump();
    CHECK(args.find("--workspace") != std::string::npos, "missing workspace arg");
    CHECK(args.find("--language") != std::string::npos, "missing language arg");
    PASS();
}

void test_phase18a_tools_and_counts_present() {
    TEST(phase18a_tools_and_counts_present);
    MCPServer server;
    CHECK((int)server.getTools().size() >= 75, "expected 75+ tools");
    bool onboard = false, reviewQ = false, approve = false, progress = false;
    for (const auto& t : server.getTools()) {
        if (t.name == "whetstone_onboard_workspace") onboard = true;
        if (t.name == "whetstone_get_review_queue") reviewQ = true;
        if (t.name == "whetstone_approve_item") approve = true;
        if (t.name == "whetstone_get_progress") progress = true;
    }
    CHECK(onboard && reviewQ && approve && progress, "missing required phase tools");
    PASS();
}

void test_phase18a_workflow_prompt_templates_available() {
    TEST(phase18a_workflow_prompt_templates_available);
    auto templates = MCPWorkflowPrompts::templates();
    bool architect = false, modernize = false, reviewPending = false;
    for (const auto& t : templates) {
        if (t.name == "architect_project") architect = true;
        if (t.name == "modernize_module") modernize = true;
        if (t.name == "review_pending") reviewPending = true;
    }
    CHECK(architect && modernize && reviewPending, "missing workflow prompts");
    PASS();
}

void test_onboard_then_workflow_tool_sequence_mocked() {
    TEST(onboard_then_workflow_tool_sequence_mocked);
    MCPServer server;
    std::vector<std::string> calls;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        calls.push_back(method);
        if (method == "indexWorkspace") return rpcResult({{"root", "/repo"}, {"fileCount", 1}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array({{{"path", "src/main.cpp"}, {"isDir", false}}})}});
        if (method == "openFile") return rpcResult(req["params"]);
        if (method == "inferAnnotations") return rpcResult({{"count", 2}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        if (method == "createSkeleton") return rpcResult({{"bufferId", "skel_demo"}});
        if (method == "addSkeletonNode") return rpcResult({{"nodeId", "fn1"}});
        if (method == "createWorkflow") return rpcResult({{"itemCount", 1}});
        return rpcResult(json::object());
    });

    auto onboard = callTool(server, "whetstone_onboard_workspace");
    auto skel = callTool(server, "whetstone_create_skeleton",
                         {{"name", "demo"}, {"language", "cpp"}});
    auto add = callTool(server, "whetstone_add_skeleton_node",
                        {{"nodeType", "function"}, {"name", "process"},
                         {"automatability", "llm"}, {"contextWidth", "file"}});
    auto wf = callTool(server, "whetstone_create_workflow", {{"projectName", "demo"}});;

    CHECK(onboard.value("success", false), "onboard failed");
    CHECK(skel.contains("bufferId"), "create skeleton failed");
    CHECK(add.contains("nodeId"), "add node failed");
    CHECK(wf.value("itemCount", 0) >= 1, "create workflow failed");
    CHECK(!calls.empty() && calls.front() == "indexWorkspace", "expected onboarding start");
    PASS();
}

void test_orchestrate_submit_review_progress_sequence_mocked() {
    TEST(orchestrate_submit_review_progress_sequence_mocked);
    MCPServer server;
    bool reviewPending = false;
    bool complete = false;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        if (method == "orchestrateRunDeterministic")
            return rpcResult({{"blocked", 1}, {"processed", 0}});
        if (method == "getBlockers")
            return rpcResult({{"external", json::array({{{"itemId", "w1"}}})}});
        if (method == "submitExternalResult") {
            reviewPending = true;
            return rpcResult({{"status", "submitted"}, {"reviewRequired", true}});
        }
        if (method == "getReviewQueue")
            return rpcResult({{"items", reviewPending
                ? json::array({{{"itemId", "w1"}, {"status", "review"}}})
                : json::array()}, {"count", reviewPending ? 1 : 0}});
        if (method == "approveReviewItem") {
            reviewPending = false;
            complete = true;
            return rpcResult({{"success", true}});
        }
        if (method == "getProgress")
            return rpcResult({{"completionPercent", complete ? 100.0 : 60.0}});
        return rpcResult(json::object());
    });

    callTool(server, "whetstone_orchestrate_run_deterministic");
    auto blockers = callTool(server, "whetstone_get_blockers");
    CHECK(blockers["external"].is_array() && !blockers["external"].empty(),
          "expected blocker");
    auto submit = callTool(server, "whetstone_submit_result", {
        {"itemId", "w1"},
        {"result", {{"generatedCode", "x=1"}, {"confidence", 0.9}, {"reasoning", "done"}}}
    });
    CHECK(submit.value("reviewRequired", false), "expected reviewRequired after submit");
    auto q1 = callTool(server, "whetstone_get_review_queue");
    CHECK(q1.value("count", 0) == 1, "expected one review item");
    auto approve = callTool(server, "whetstone_approve_item", {{"itemId", "w1"}});
    CHECK(approve.value("success", false), "approve failed");
    auto progress = callTool(server, "whetstone_get_progress");
    CHECK(progress.value("completionPercent", 0.0) >= 100.0, "expected completion");
    PASS();
}

void test_full_phase18a_protocol_mocked_end_to_end() {
    TEST(full_phase18a_protocol_mocked_end_to_end);
    MCPServer server;
    std::vector<std::string> methods;
    bool reviewPending = false;
    bool complete = false;
    server.setRpcCallback([&](const json& req) {
        std::string method = req.value("method", "");
        methods.push_back(method);
        if (method == "indexWorkspace") return rpcResult({{"root", "/repo"}, {"fileCount", 2}});
        if (method == "workspaceList")
            return rpcResult({{"files", json::array({
                {{"path", "src/main.cpp"}, {"isDir", false}},
                {{"path", "src/util.py"}, {"isDir", false}}
            })}});
        if (method == "openFile") return rpcResult(req["params"]);
        if (method == "inferAnnotations") return rpcResult({{"count", 2}});
        if (method == "saveAnnotatedAST") return rpcResult({{"success", true}});
        if (method == "fileCreate") return rpcResult({{"created", true}});
        if (method == "createSkeleton") return rpcResult({{"bufferId", "skel_demo"}});
        if (method == "addSkeletonNode") return rpcResult({{"nodeId", "n1"}});
        if (method == "createWorkflow") return rpcResult({{"itemCount", 1}});
        if (method == "orchestrateRunDeterministic") return rpcResult({{"blocked", 1}});
        if (method == "getBlockers")
            return rpcResult({{"external", json::array({{{"itemId", "w1"}}})}});
        if (method == "submitExternalResult") {
            reviewPending = true;
            return rpcResult({{"status", "submitted"}, {"reviewRequired", true}});
        }
        if (method == "getReviewQueue")
            return rpcResult({{"items", reviewPending
                ? json::array({{{"itemId", "w1"}}})
                : json::array()}, {"count", reviewPending ? 1 : 0}});
        if (method == "approveReviewItem") {
            reviewPending = false;
            complete = true;
            return rpcResult({{"success", true}});
        }
        if (method == "getProgress")
            return rpcResult({{"completionPercent", complete ? 100.0 : 40.0}});
        return rpcResult(json::object());
    });

    callTool(server, "whetstone_onboard_workspace");
    callTool(server, "whetstone_create_skeleton", {{"name", "demo"}, {"language", "cpp"}});
    callTool(server, "whetstone_add_skeleton_node",
             {{"nodeType", "function"}, {"name", "process"}, {"automatability", "llm"}});
    callTool(server, "whetstone_create_workflow", {{"projectName", "demo"}});
    callTool(server, "whetstone_orchestrate_run_deterministic");
    callTool(server, "whetstone_get_blockers");
    callTool(server, "whetstone_submit_result",
             {{"itemId", "w1"}, {"result", {{"generatedCode", "x=1"}, {"confidence", 0.91}}}});
    callTool(server, "whetstone_get_review_queue");
    callTool(server, "whetstone_approve_item", {{"itemId", "w1"}});
    auto progress = callTool(server, "whetstone_get_progress");

    CHECK(progress.value("completionPercent", 0.0) >= 100.0, "expected complete progress");
    CHECK(!methods.empty() && methods.front() == "indexWorkspace", "flow should start with onboarding");
    CHECK(std::find(methods.begin(), methods.end(), "approveReviewItem") != methods.end(),
          "missing review approval phase");
    PASS();
}

void test_headless_review_completion_via_mcp() {
    TEST(headless_review_completion_via_mcp);
    auto state = makeStateWithReviewItem();
    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        return state.processAgentRequest(req, "sess");
    });

    auto queue = callTool(server, "whetstone_get_review_queue");
    CHECK(queue.value("count", 0) == 1, "expected one review item");
    auto approve = callTool(server, "whetstone_approve_item",
                            {{"itemId", "w-review-1"}, {"feedback", "ok"}});
    CHECK(approve.value("success", false), "approval failed");
    auto queueAfter = callTool(server, "whetstone_get_review_queue");
    CHECK(queueAfter.value("count", -1) == 0, "queue should be empty after approve");
    PASS();
}

void test_headless_onboard_workspace_smoke() {
    TEST(headless_onboard_workspace_smoke);
    auto root = std::filesystem::path(makeTempDir("onboard"));
    std::filesystem::create_directories(root / "src");
    std::ofstream(root / "src" / "main.cpp") << "int main(){return 0;}\n";

    HeadlessEditorState state;
    state.setAgentRole("sess", AgentRole::Refactor);
    state.workspaceRoot = root.string();

    MCPServer server;
    server.setRpcCallback([&](const json& req) {
        return state.processAgentRequest(req, "sess");
    });

    auto res = callTool(server, "whetstone_onboard_workspace",
                        {{"root", root.string()}, {"maxFiles", 1}});
    CHECK(res.value("success", false), "expected onboarding success");
    CHECK(std::filesystem::exists(root / ".whetstone" / "README.md"),
          "expected .whetstone bootstrap");
    PASS();
}

int main() {
    std::cout << "Step 422: Phase 18a Integration Tests\n";

    test_mcp_config_build_for_phase18a();                    // 1
    test_phase18a_tools_and_counts_present();                // 2
    test_phase18a_workflow_prompt_templates_available();    // 3
    test_onboard_then_workflow_tool_sequence_mocked();       // 4
    test_orchestrate_submit_review_progress_sequence_mocked();// 5
    test_full_phase18a_protocol_mocked_end_to_end();         // 6
    test_headless_review_completion_via_mcp();               // 7
    test_headless_onboard_workspace_smoke();                 // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
