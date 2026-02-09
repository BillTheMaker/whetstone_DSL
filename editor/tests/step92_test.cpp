// Step 92 TDD Test: LSP auto-completion
//
// Tests:
// 1. completion response parsed into items

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

    int reqId = client.requestCompletion("file:///tmp/a.py", 2, 5);
    assert(reqId == 1);
    std::string resp = R"({"jsonrpc":"2.0","id":1,"result":{"isIncomplete":false,"items":[{"label":"printf","kind":3,"detail":"fn","insertText":"printf"}]}})";
    client.handleMessage(resp);

    auto items = client.getCompletionItems();
    assert(items.size() == 1);
    assert(items[0].label == "printf");
    assert(items[0].kind == 3);
    std::cout << "Test 1 PASS: completion response parsed" << std::endl;
    ++passed;

    std::cout << "\n=== Step 92 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
