// Step 110 TDD Test: Go-to-definition parsing
#include "LSPClient.h"
#include <iostream>
#include <memory>

struct DummyTransport : public LSPTransport {
    void send(const std::string& msg) override { (void)msg; }
    bool receive(std::string& out) override { (void)out; return false; }
    bool isOpen() const override { return true; }
    void close() override {}
};

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

    {
        auto transport = std::make_shared<DummyTransport>();
        LSPClient client(transport);
        int id = client.requestDefinition("file:///test.py", 1, 2);
        std::string msg = std::string("{\"jsonrpc\":\"2.0\",\"id\":") +
            std::to_string(id) +
            ",\"result\":{\"uri\":\"file:///test.py\",\"range\":{\"start\":{\"line\":5,\"character\":1},\"end\":{\"line\":5,\"character\":3}}}}";
        client.handleMessage(msg);
        auto defs = client.getDefinitionLocations();
        bool ok = defs.size() == 1 && defs[0].uri == "file:///test.py" &&
                  defs[0].range.start.line == 5 && defs[0].range.start.character == 1 &&
                  defs[0].range.end.line == 5 && defs[0].range.end.character == 3;
        expect(ok, "location object", passed, failed);
    }

    {
        auto transport = std::make_shared<DummyTransport>();
        LSPClient client(transport);
        int id = client.requestDefinition("file:///main.py", 0, 0);
        std::string msg = std::string("{\"jsonrpc\":\"2.0\",\"id\":") +
            std::to_string(id) +
            ",\"result\":[{\"targetUri\":\"file:///lib.py\",\"targetRange\":{\"start\":{\"line\":2,\"character\":0},\"end\":{\"line\":2,\"character\":4}}}]}";
        client.handleMessage(msg);
        auto defs = client.getDefinitionLocations();
        bool ok = defs.size() == 1 && defs[0].uri == "file:///lib.py" &&
                  defs[0].range.start.line == 2 && defs[0].range.start.character == 0 &&
                  defs[0].range.end.line == 2 && defs[0].range.end.character == 4;
        expect(ok, "location link array", passed, failed);
    }

    std::cout << "\n=== Step 110 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
