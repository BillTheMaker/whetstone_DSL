// Step 125 TDD Test: Request log callback
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

    bool logCalled = false;
    std::string lastMethod;
    server.setRequestLogCallback([&](const std::string&,
                                     const json& req,
                                     const json&) {
        logCalled = true;
        lastMethod = req.value("method", "");
    });

    server.start(9001);
    std::string sid = raw->simulateConnect();
    json msg = {{"jsonrpc","2.0"},{"id",1},{"method","ping"}};
    raw->simulateMessage(sid, msg.dump());
    expect(logCalled, "request log callback", passed, failed);
    expect(lastMethod == "ping", "method logged", passed, failed);

    std::cout << "\n=== Step 125 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
