// Step 59 TDD Test: WebSocket agent endpoint
//
// Tests the WebSocketAgentServer with MockWebSocketTransport:
//  1. Server starts and reports running
//  2. Agent connects and receives a session ID
//  3. Agent sends JSON-RPC `ping`, receives `pong` with session ID
//  4. Multiple agents get unique session IDs
//  5. Agent disconnect is tracked (session marked inactive)
//  6. Session event callbacks fire on connect/disconnect
//  7. Invalid JSON produces a parse-error response
//  8. Unknown method produces method-not-found error
//  9. Custom request handler delegation works
// 10. listSessions returns all active agents

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "WebSocketServer.h"

using json = nlohmann::json;

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Server starts and reports running ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();  // keep raw ptr for test helpers
        WebSocketAgentServer server(std::move(transport));

        assert(!server.isRunning() && "Server should not be running before start");

        bool ok = server.start(9090);
        assert(ok && "Server should start successfully");
        assert(server.isRunning() && "Server should be running after start");
        assert(server.getPort() == 9090 && "Port should match");

        server.stop();
        assert(!server.isRunning() && "Server should stop");

        std::cout << "Test 1 PASS: Server starts and stops correctly" << std::endl;
        ++passed;
    }

    // --- Test 2: Agent connects and gets session ID ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid = tp->simulateConnect();
        assert(!sid.empty() && "Session ID should not be empty");

        const AgentSession* session = server.getSession(sid);
        assert(session != nullptr && "Session should exist after connect");
        assert(session->connected && "Session should be marked connected");
        assert(session->sessionId == sid && "Session ID should match");
        assert(server.getActiveSessionCount() == 1 && "Should have 1 active session");

        std::cout << "Test 2 PASS: Agent connects and gets session ID" << std::endl;
        ++passed;
    }

    // --- Test 3: Agent sends ping, receives pong with session ID ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid = tp->simulateConnect();

        json pingRequest = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "ping"}
        };
        tp->simulateMessage(sid, pingRequest.dump());

        std::string response = tp->getLastSentMessage(sid);
        assert(!response.empty() && "Should receive a response");

        json resp = json::parse(response);
        assert(resp.contains("result") && "Response should have result");
        assert(resp["result"]["message"] == "pong" && "Result message should be pong");
        assert(resp["result"]["sessionId"] == sid && "Result should include session ID");
        assert(resp["id"] == 1 && "Response ID should match request");

        std::cout << "Test 3 PASS: ping returns pong with session ID" << std::endl;
        ++passed;
    }

    // --- Test 4: Multiple agents get unique session IDs ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid1 = tp->simulateConnect();
        std::string sid2 = tp->simulateConnect();
        std::string sid3 = tp->simulateConnect();

        assert(sid1 != sid2 && "Session IDs should be unique");
        assert(sid2 != sid3 && "Session IDs should be unique");
        assert(sid1 != sid3 && "Session IDs should be unique");
        assert(server.getActiveSessionCount() == 3 && "Should have 3 active sessions");

        std::cout << "Test 4 PASS: Multiple agents get unique session IDs" << std::endl;
        ++passed;
    }

    // --- Test 5: Agent disconnect is tracked ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid = tp->simulateConnect();
        assert(server.getActiveSessionCount() == 1);

        tp->simulateDisconnect(sid);

        const AgentSession* session = server.getSession(sid);
        assert(session != nullptr && "Session should still exist");
        assert(!session->connected && "Session should be marked disconnected");
        assert(server.getActiveSessionCount() == 0 && "No active sessions after disconnect");

        std::cout << "Test 5 PASS: Agent disconnect tracked correctly" << std::endl;
        ++passed;
    }

    // --- Test 6: Session event callbacks fire ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));

        std::vector<std::string> events;
        server.setSessionEventCallback(
            [&events](const AgentSession& s, const std::string& event) {
                events.push_back(s.sessionId + ":" + event);
            });

        server.start(9090);

        std::string sid = tp->simulateConnect();
        assert(events.size() == 1 && "Should have 1 event after connect");
        assert(contains(events[0], "connected") && "Event should be 'connected'");

        tp->simulateDisconnect(sid);
        assert(events.size() == 2 && "Should have 2 events after disconnect");
        assert(contains(events[1], "disconnected") && "Event should be 'disconnected'");

        std::cout << "Test 6 PASS: Session event callbacks fire correctly" << std::endl;
        ++passed;
    }

    // --- Test 7: Invalid JSON produces parse-error response ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid = tp->simulateConnect();
        tp->simulateMessage(sid, "this is not json!!!");

        std::string response = tp->getLastSentMessage(sid);
        json resp = json::parse(response);
        assert(resp.contains("error") && "Should have error for invalid JSON");
        assert(resp["error"]["code"] == -32700 && "Error code should be -32700 (Parse error)");

        std::cout << "Test 7 PASS: Invalid JSON returns parse error" << std::endl;
        ++passed;
    }

    // --- Test 8: Unknown method produces method-not-found error ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid = tp->simulateConnect();
        json req = {
            {"jsonrpc", "2.0"},
            {"id", 42},
            {"method", "nonExistentMethod"}
        };
        tp->simulateMessage(sid, req.dump());

        std::string response = tp->getLastSentMessage(sid);
        json resp = json::parse(response);
        assert(resp.contains("error") && "Should have error for unknown method");
        assert(resp["error"]["code"] == -32601 && "Error code should be -32601 (Method not found)");
        assert(resp["id"] == 42 && "Response ID should match request");

        std::cout << "Test 8 PASS: Unknown method returns method-not-found error" << std::endl;
        ++passed;
    }

    // --- Test 9: Custom request handler delegation ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));

        // Register a custom handler that knows about "getAST"
        server.setRequestHandler([](const json& request) -> json {
            std::string method = request.at("method").get<std::string>();
            json response;
            response["jsonrpc"] = "2.0";
            response["id"] = request.contains("id") ? request["id"] : json(nullptr);
            if (method == "getAST") {
                response["result"] = {{"concept", "Module"}, {"name", "TestModule"}};
            } else {
                response["error"] = {{"code", -32601}, {"message", "Method not found"}};
            }
            return response;
        });

        server.start(9090);
        std::string sid = tp->simulateConnect();

        json req = {
            {"jsonrpc", "2.0"},
            {"id", 10},
            {"method", "getAST"}
        };
        tp->simulateMessage(sid, req.dump());

        std::string response = tp->getLastSentMessage(sid);
        json resp = json::parse(response);
        assert(resp.contains("result") && "Delegated handler should return result");
        assert(resp["result"]["concept"] == "Module" && "Should get Module from handler");
        assert(resp["id"] == 10 && "Response ID should match");

        std::cout << "Test 9 PASS: Custom request handler delegation works" << std::endl;
        ++passed;
    }

    // --- Test 10: listSessions returns all active agents ---
    {
        auto transport = std::make_unique<MockWebSocketTransport>();
        auto* tp = transport.get();
        WebSocketAgentServer server(std::move(transport));
        server.start(9090);

        std::string sid1 = tp->simulateConnect();
        std::string sid2 = tp->simulateConnect();

        // Set agent name on sid1
        json nameReq = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "setAgentName"},
            {"params", {{"name", "CodeAssistant"}}}
        };
        tp->simulateMessage(sid1, nameReq.dump());

        // Disconnect sid2
        tp->simulateDisconnect(sid2);

        // Ask for session list from sid1
        json listReq = {
            {"jsonrpc", "2.0"},
            {"id", 2},
            {"method", "listSessions"}
        };
        tp->simulateMessage(sid1, listReq.dump());

        std::string response = tp->getLastSentMessage(sid1);
        json resp = json::parse(response);
        assert(resp.contains("result") && "Should have result");
        assert(resp["result"].is_array() && "Result should be an array");
        // Only sid1 should be active (sid2 disconnected)
        assert(resp["result"].size() == 1 && "Should have 1 active session");
        assert(resp["result"][0]["agentName"] == "CodeAssistant" &&
               "Agent name should be set");

        std::cout << "Test 10 PASS: listSessions returns active agents" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 59 Results: " << passed << " passed, "
              << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
