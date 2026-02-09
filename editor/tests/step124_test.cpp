// Step 124 TDD Test: Agent server wiring with mock transport
#include "WebSocketServer.h"
#include <iostream>

static void expect(bool cond, const std::string& name, int& passed, int& failed) {
    if (cond) {
        std::cout << "Test " << (passed + failed + 1) << " PASS: " << name << "\n";
        ++passed;
    } else {
        std::cout << "Test " << (passed + failed + 1) << " FAIL: " << name << "\n";
        ++failed;
    }
}

int main() {
    int passed = 0;
    int failed = 0;

    auto transport = std::make_unique<MockWebSocketTransport>();
    MockWebSocketTransport* raw = transport.get();
    WebSocketAgentServer server(std::move(transport));

    server.setRequestHandler([](const json& req) {
        json res;
        res["result"] = {{"ok", true}, {"echo", req.value("method", "")}};
        return res;
    });

    expect(server.start(8765), "server starts", passed, failed);
    std::string sid = raw->simulateConnect();
    expect(!sid.empty(), "session id assigned", passed, failed);

    json ping = {{"jsonrpc","2.0"},{"id",1},{"method","ping"}};
    raw->simulateMessage(sid, ping.dump());
    std::string reply = raw->getLastSentMessage(sid);
    expect(reply.find("pong") != std::string::npos, "ping response", passed, failed);

    json custom = {{"jsonrpc","2.0"},{"id",2},{"method","getAST"}};
    raw->simulateMessage(sid, custom.dump());
    std::string reply2 = raw->getLastSentMessage(sid);
    expect(reply2.find("\"ok\":true") != std::string::npos, "custom handler", passed, failed);

    raw->simulateDisconnect(sid);
    expect(server.getActiveSessionCount() == 0, "session disconnect", passed, failed);

    std::cout << "\n=== Step 124 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
