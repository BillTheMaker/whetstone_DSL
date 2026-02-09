// Step 93 TDD Test: LSP hover and signature help
//
// Tests:
// 1. hover response parsed into text
// 2. signature help response parsed

#include <cassert>
#include <iostream>
#include "LSPClient.h"

struct MockTransport : public LSPTransport {
    std::vector<std::string> sent;
    void send(const std::string& msg) override { sent.push_back(msg); }
    bool receive(std::string& out) override { (void)out; return false; }
    bool isOpen() const override { return true; }
    void close() override {}
};

int main() {
    int passed = 0;
    int failed = 0;

    auto transport = std::make_shared<MockTransport>();
    LSPClient client(transport);

    int hoverId = client.requestHover("file:///tmp/a.py", 0, 1);
    assert(hoverId == 1);
    std::string hoverResp = R"json({"jsonrpc":"2.0","id":1,"result":{"contents":"int"}})json";
    client.handleMessage(hoverResp);
    assert(client.getHoverContents() == "int");
    std::cout << "Test 1 PASS: hover response parsed" << std::endl;
    ++passed;

    int sigId = client.requestSignatureHelp("file:///tmp/a.py", 0, 5);
    assert(sigId == 2);
    std::string sigResp = R"json({"jsonrpc":"2.0","id":2,"result":{"signatures":[{"label":"foo(a: int, b: int)"}],"activeSignature":0,"activeParameter":1}})json";
    client.handleMessage(sigResp);
    auto sig = client.getSignatureHelp();
    assert(sig.label == "foo(a: int, b: int)");
    assert(sig.activeParameter == 1);
    std::cout << "Test 2 PASS: signature help response parsed" << std::endl;
    ++passed;

    std::cout << "\n=== Step 93 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
