// Step 420: Human Review Interface via MCP Tests (12 tests)

#include "MCPServer.h"
#include "HeadlessEditorState.h"

#include <iostream>
#include <string>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static HeadlessEditorState makeStateWithReviewItem(const std::string& status = WI_REVIEW) {
    HeadlessEditorState state;
    state.setAgentRole("sess", AgentRole::Refactor);
    state.workflow = WorkflowState("review-proj");

    WorkItem wi;
    wi.id = "w1";
    wi.nodeId = "node1";
    wi.nodeName = "processOrder";
    wi.nodeType = "Function";
    wi.bufferId = "orders.py";
    wi.workerType = "llm";
    wi.priority = "high";
    wi.reviewRequired = true;
    wi.status = status;
    wi.result.generatedCode = "def process_order(x):\n    return x\n";
    wi.result.confidence = 0.77f;
    wi.result.reasoning = "Generated from blocker context.";
    state.workflow->queue.enqueue(wi);
    return state;
}

static json call(HeadlessEditorState& state, const std::string& method, const json& params = json::object()) {
    return state.processAgentRequest({
        {"jsonrpc", "2.0"},
        {"id", 1},
        {"method", method},
        {"params", params}
    }, "sess");
}

void test_mcp_review_tools_registered() {
    TEST(mcp_review_tools_registered);
    MCPServer server;
    bool q = false, c = false, a = false, r = false;
    for (const auto& t : server.getTools()) {
        if (t.name == "whetstone_get_review_queue") q = true;
        if (t.name == "whetstone_get_review_context") c = true;
        if (t.name == "whetstone_approve_item") a = true;
        if (t.name == "whetstone_reject_item") r = true;
    }
    CHECK(q && c && a && r, "expected all review MCP tools");
    PASS();
}

void test_mcp_tool_mapping_get_review_queue() {
    TEST(mcp_tool_mapping_get_review_queue);
    MCPServer server;
    std::string calledMethod;
    server.setRpcCallback([&](const json& req) {
        calledMethod = req.value("method", "");
        return json{{"jsonrpc", "2.0"}, {"id", 1}, {"result", {{"items", json::array()}, {"count", 0}}}};
    });
    auto resp = server.handleRequest({
        {"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/call"},
        {"params", {{"name", "whetstone_get_review_queue"}, {"arguments", json::object()}}}
    });
    CHECK(resp.contains("result"), "missing result");
    CHECK(calledMethod == "getReviewQueue", "wrong backend method mapping");
    PASS();
}

void test_get_review_queue_returns_items() {
    TEST(get_review_queue_returns_items);
    auto state = makeStateWithReviewItem();
    auto resp = call(state, "getReviewQueue");
    CHECK(resp.contains("result"), "expected result");
    auto result = resp["result"];
    CHECK(result.value("count", 0) == 1, "expected one review item");
    CHECK(result["items"][0].value("itemId", "") == "w1", "wrong item id");
    PASS();
}

void test_get_review_queue_empty_when_no_review_items() {
    TEST(get_review_queue_empty_when_no_review_items);
    auto state = makeStateWithReviewItem(WI_READY);
    auto resp = call(state, "getReviewQueue");
    CHECK(resp.contains("result"), "expected result");
    CHECK(resp["result"].value("count", -1) == 0, "expected empty review queue");
    PASS();
}

void test_get_review_context_contains_human_summary() {
    TEST(get_review_context_contains_human_summary);
    auto state = makeStateWithReviewItem();
    auto resp = call(state, "getReviewContext", {{"itemId", "w1"}});
    CHECK(resp.contains("result"), "expected result");
    std::string summary = resp["result"].value("humanSummary", "");
    CHECK(summary.find("Review Item: w1") != std::string::npos, "missing review header");
    CHECK(summary.find("Generated Code:") != std::string::npos, "missing generated code section");
    PASS();
}

void test_get_review_context_missing_item_errors() {
    TEST(get_review_context_missing_item_errors);
    auto state = makeStateWithReviewItem();
    auto resp = call(state, "getReviewContext", {{"itemId", "missing"}});
    CHECK(resp.contains("error"), "expected error");
    PASS();
}

void test_approve_item_marks_complete() {
    TEST(approve_item_marks_complete);
    auto state = makeStateWithReviewItem();
    auto resp = call(state, "approveReviewItem", {{"itemId", "w1"}, {"feedback", "looks good"}});
    CHECK(resp.contains("result"), "expected result");
    auto item = state.workflow->queue.getItem("w1");
    CHECK(item.has_value(), "missing item after approval");
    CHECK(item->status == WI_COMPLETE, "expected complete status");
    PASS();
}

void test_approve_item_wrong_status_errors() {
    TEST(approve_item_wrong_status_errors);
    auto state = makeStateWithReviewItem(WI_READY);
    auto resp = call(state, "approveReviewItem", {{"itemId", "w1"}});
    CHECK(resp.contains("error"), "expected error for wrong status");
    PASS();
}

void test_reject_item_requires_feedback() {
    TEST(reject_item_requires_feedback);
    auto state = makeStateWithReviewItem();
    auto resp = call(state, "rejectReviewItem", {{"itemId", "w1"}});
    CHECK(resp.contains("error"), "expected missing feedback error");
    PASS();
}

void test_reject_item_moves_back_to_ready() {
    TEST(reject_item_moves_back_to_ready);
    auto state = makeStateWithReviewItem();
    auto resp = call(state, "rejectReviewItem", {{"itemId", "w1"}, {"feedback", "rename variable"}});
    CHECK(resp.contains("result"), "expected result");
    auto item = state.workflow->queue.getItem("w1");
    CHECK(item.has_value(), "missing item");
    CHECK(item->status == WI_READY, "expected item re-queued to ready");
    PASS();
}

void test_reject_item_wrong_status_errors() {
    TEST(reject_item_wrong_status_errors);
    auto state = makeStateWithReviewItem(WI_READY);
    auto resp = call(state, "rejectReviewItem", {{"itemId", "w1"}, {"feedback", "nope"}});
    CHECK(resp.contains("error"), "expected error for wrong status");
    PASS();
}

void test_mcp_end_to_end_review_tools() {
    TEST(mcp_end_to_end_review_tools);
    auto state = makeStateWithReviewItem();
    MCPServer server;
    server.setRpcCallback([&](const json& req) { return state.processAgentRequest(req, "sess"); });

    auto queueResp = server.handleRequest({
        {"jsonrpc", "2.0"}, {"id", 1}, {"method", "tools/call"},
        {"params", {{"name", "whetstone_get_review_queue"}, {"arguments", json::object()}}}
    });
    CHECK(queueResp.contains("result"), "missing queue result");

    auto approveResp = server.handleRequest({
        {"jsonrpc", "2.0"}, {"id", 2}, {"method", "tools/call"},
        {"params", {{"name", "whetstone_approve_item"},
                    {"arguments", {{"itemId", "w1"}, {"feedback", "approved"}}}}}
    });
    CHECK(approveResp.contains("result"), "missing approve result");
    auto item = state.workflow->queue.getItem("w1");
    CHECK(item.has_value() && item->status == WI_COMPLETE, "approve should complete item");
    PASS();
}

int main() {
    std::cout << "Step 420: Human Review Interface via MCP Tests\n";

    test_mcp_review_tools_registered();                // 1
    test_mcp_tool_mapping_get_review_queue();          // 2
    test_get_review_queue_returns_items();             // 3
    test_get_review_queue_empty_when_no_review_items();// 4
    test_get_review_context_contains_human_summary();  // 5
    test_get_review_context_missing_item_errors();     // 6
    test_approve_item_marks_complete();                // 7
    test_approve_item_wrong_status_errors();           // 8
    test_reject_item_requires_feedback();              // 9
    test_reject_item_moves_back_to_ready();            // 10
    test_reject_item_wrong_status_errors();            // 11
    test_mcp_end_to_end_review_tools();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
