// Step 387: Orchestrator event stream (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "EventStream.h"
#include "HeadlessEditorState.h"
#include "HeadlessAgentRPCHandler.h"
#include "MCPServer.h"

static WorkItem makeItem(const std::string& id,
                         const std::string& workerType = "template",
                         const std::string& contextWidth = "local") {
    WorkItem item;
    item.id = id;
    item.nodeId = id + "_node";
    item.nodeName = "getValue";
    item.nodeType = "Function";
    item.bufferId = "main.py";
    item.workerType = workerType;
    item.contextWidth = contextWidth;
    item.priority = "medium";
    item.status = WI_PENDING;
    item.createdAt = workItemTimestamp();
    return item;
}

static HeadlessEditorState makeState() {
    HeadlessEditorState state;
    state.defaultLanguage = "python";
    state.openBuffer("main.py", "def a():\n    return 1\n", "python");
    state.setAgentRole("test-session", AgentRole::Generator);
    state.workflow = WorkflowState("events");
    return state;
}

static json rpc(HeadlessEditorState& state, const std::string& method,
                const json& params = json::object()) {
    json request = {{"jsonrpc", "2.0"}, {"id", 1}, {"method", method},
                    {"params", params}};
    return handleHeadlessAgentRequest(state, request, "test-session");
}

int main() {
    int passed = 0;

    // Test 1: emit + poll returns emitted event
    {
        EventStream stream;
        stream.emit({"task.executed", "a", json::object(), workItemTimestamp()});
        auto events = stream.poll(0);
        assert(events.size() == 1);
        assert(events[0].event.type == "task.executed");
        std::cout << "Test 1 PASSED: emit/poll basic behavior\n";
        passed++;
    }

    // Test 2: poll since version returns only new events
    {
        EventStream stream;
        stream.emit({"x", "a", json::object(), workItemTimestamp()}); // v1
        stream.emit({"y", "b", json::object(), workItemTimestamp()}); // v2
        auto newer = stream.poll(1);
        assert(newer.size() == 1);
        assert(newer[0].event.type == "y");
        std::cout << "Test 2 PASSED: poll filters by version\n";
        passed++;
    }

    // Test 3: stream version increments with emits
    {
        EventStream stream;
        assert(stream.getVersion() == 0);
        stream.emit({"a", "", json::object(), workItemTimestamp()});
        stream.emit({"b", "", json::object(), workItemTimestamp()});
        assert(stream.getVersion() == 2);
        std::cout << "Test 3 PASSED: version tracking\n";
        passed++;
    }

    // Test 4: subscribe callback is invoked
    {
        EventStream stream;
        int callbackCount = 0;
        stream.subscribe([&callbackCount](const StreamEvent&) {
            callbackCount++;
        });
        stream.emit({"a", "", json::object(), workItemTimestamp()});
        assert(callbackCount == 1);
        std::cout << "Test 4 PASSED: subscribe callback fires\n";
        passed++;
    }

    // Test 5: orchestration emits normalized task event types
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("t1", "template"));
        rpc(state, "orchestrateAdvance");
        auto resp = rpc(state, "getEventStream", {{"sinceVersion", 0}});
        assert(resp.contains("result"));
        bool hasRouted = false, hasExecuted = false, hasCompleted = false;
        for (const auto& e : resp["result"]["events"]) {
            std::string t = e.value("type", "");
            if (t == "task.routed") hasRouted = true;
            if (t == "task.executed") hasExecuted = true;
            if (t == "task.completed") hasCompleted = true;
        }
        assert(hasRouted && hasExecuted && hasCompleted);
        std::cout << "Test 5 PASSED: normalized task event types emitted\n";
        passed++;
    }

    // Test 6: getRecent returns requested event count
    {
        EventStream stream;
        stream.emit({"e1", "", json::object(), workItemTimestamp()});
        stream.emit({"e2", "", json::object(), workItemTimestamp()});
        stream.emit({"e3", "", json::object(), workItemTimestamp()});
        auto recent = stream.getRecent(2);
        assert(recent.size() == 2);
        assert(recent[0].event.type == "e2");
        assert(recent[1].event.type == "e3");
        std::cout << "Test 6 PASSED: getRecent count + ordering\n";
        passed++;
    }

    // Test 7: empty stream poll returns no events
    {
        EventStream stream;
        auto events = stream.poll(0);
        assert(events.empty());
        std::cout << "Test 7 PASSED: empty stream poll behavior\n";
        passed++;
    }

    // Test 8: high-frequency polling does not duplicate results
    {
        EventStream stream;
        stream.emit({"a", "", json::object(), workItemTimestamp()});
        auto first = stream.poll(0);
        auto second = stream.poll(stream.getVersion());
        assert(first.size() == 1);
        assert(second.empty());
        std::cout << "Test 8 PASSED: no duplicate events after poll version update\n";
        passed++;
    }

    // Test 9: stream JSON serialization includes required fields
    {
        EventStream stream;
        stream.emit({"task.executed", "x", json{{"k", "v"}}, workItemTimestamp()});
        auto j = EventStream::toJson(stream.poll(0));
        assert(j.is_array());
        assert(j[0].contains("version"));
        assert(j[0].contains("type"));
        assert(j[0].contains("timestamp"));
        std::cout << "Test 9 PASSED: stream JSON serialization\n";
        passed++;
    }

    // Test 10: MCP tool registration includes event stream tools
    {
        MCPServer server;
        bool hasStream = false, hasRecent = false;
        for (const auto& t : server.getTools()) {
            if (t.name == "whetstone_get_event_stream") hasStream = true;
            if (t.name == "whetstone_get_recent_events") hasRecent = true;
        }
        assert(hasStream && hasRecent);
        std::cout << "Test 10 PASSED: MCP event stream tools registered\n";
        passed++;
    }

    // Test 11: getEventStream RPC returns only events since version
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("t1", "template"));
        rpc(state, "orchestrateAdvance");
        auto first = rpc(state, "getEventStream", {{"sinceVersion", 0}});
        int v = first["result"]["version"].get<int>();
        auto second = rpc(state, "getEventStream", {{"sinceVersion", v}});
        assert(first["result"]["events"].size() > 0);
        assert(second["result"]["events"].empty());
        std::cout << "Test 11 PASSED: getEventStream since-version behavior\n";
        passed++;
    }

    // Test 12: getRecentEvents RPC honors count parameter
    {
        auto state = makeState();
        state.workflow->queue.enqueue(makeItem("a", "template"));
        state.workflow->queue.enqueue(makeItem("b", "template"));
        rpc(state, "orchestrateAdvance");
        auto recent = rpc(state, "getRecentEvents", {{"count", 2}});
        assert(recent.contains("result"));
        assert(recent["result"]["events"].size() <= 2);
        std::cout << "Test 12 PASSED: getRecentEvents count behavior\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
