// Step 111 TDD Test: Document symbol parsing
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
        int id = client.requestDocumentSymbols("file:///test.py");
        std::string msg = std::string("{\"jsonrpc\":\"2.0\",\"id\":") +
            std::to_string(id) +
            ",\"result\":[{\"name\":\"foo\",\"kind\":12,\"range\":{\"start\":{\"line\":1,\"character\":0},\"end\":{\"line\":3,\"character\":0}},\"selectionRange\":{\"start\":{\"line\":1,\"character\":4},\"end\":{\"line\":1,\"character\":7}},\"children\":[{\"name\":\"x\",\"kind\":13,\"range\":{\"start\":{\"line\":2,\"character\":2},\"end\":{\"line\":2,\"character\":3}},\"selectionRange\":{\"start\":{\"line\":2,\"character\":2},\"end\":{\"line\":2,\"character\":3}}}]}]}";
        client.handleMessage(msg);
        auto symbols = client.getDocumentSymbols();
        bool ok = symbols.size() == 1 && symbols[0].name == "foo" && symbols[0].children.size() == 1 &&
                  symbols[0].selectionRange.start.line == 1 && symbols[0].selectionRange.start.character == 4 &&
                  symbols[0].children[0].name == "x" && symbols[0].children[0].range.start.line == 2;
        expect(ok, "document symbol tree", passed, failed);
    }

    {
        auto transport = std::make_shared<DummyTransport>();
        LSPClient client(transport);
        int id = client.requestDocumentSymbols("file:///main.py");
        std::string msg = std::string("{\"jsonrpc\":\"2.0\",\"id\":") +
            std::to_string(id) +
            ",\"result\":[{\"name\":\"bar\",\"kind\":12,\"location\":{\"uri\":\"file:///main.py\",\"range\":{\"start\":{\"line\":4,\"character\":0},\"end\":{\"line\":4,\"character\":3}}}}]}";
        client.handleMessage(msg);
        auto symbols = client.getDocumentSymbols();
        bool ok = symbols.size() == 1 && symbols[0].name == "bar" &&
                  symbols[0].range.start.line == 4 && symbols[0].range.start.character == 0 &&
                  symbols[0].selectionRange.start.line == 4;
        expect(ok, "symbol information list", passed, failed);
    }

    std::cout << "\n=== Step 111 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
