// Step 91 TDD Test: LSP diagnostics integration
//
// Tests:
// 1. handleMessage parses publishDiagnostics

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

    std::string msg = R"({"jsonrpc":"2.0","method":"textDocument/publishDiagnostics","params":{"uri":"file:///tmp/a.py","diagnostics":[{"range":{"start":{"line":1,"character":2},"end":{"line":1,"character":3}},"severity":1,"message":"err"}]}})";
    client.handleMessage(msg);

    auto diags = client.getDiagnostics();
    assert(diags.size() == 1);
    assert(diags[0].uri == "file:///tmp/a.py");
    assert(diags[0].message == "err");
    std::cout << "Test 1 PASS: publishDiagnostics parsed" << std::endl;
    ++passed;

    std::cout << "\n=== Step 91 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
