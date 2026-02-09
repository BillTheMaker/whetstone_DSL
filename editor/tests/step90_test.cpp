// Step 90 TDD Test: LSP client core
//
// Tests:
// 1. Initialize sends JSON-RPC initialize request
// 2. Shutdown sends shutdown request and exit notification

#include <cassert>
#include <iostream>
#include <string>
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

    // --- Test 1: initialize ---
    {
        auto transport = std::make_shared<MockTransport>();
        LSPClient client(transport);
        client.initialize("file:///tmp", "test-client", 1);
        assert(!transport->sent.empty());
        auto msg = transport->sent.back();
        assert(msg.find("\"method\":\"initialize\"") != std::string::npos);
        std::cout << "Test 1 PASS: initialize sends request" << std::endl;
        ++passed;
    }

    // --- Test 2: shutdown + exit ---
    {
        auto transport = std::make_shared<MockTransport>();
        LSPClient client(transport);
        client.shutdown();
        assert(transport->sent.size() >= 2);
        assert(transport->sent[transport->sent.size()-2].find("\"method\":\"shutdown\"") != std::string::npos);
        assert(transport->sent.back().find("\"method\":\"exit\"") != std::string::npos);
        std::cout << "Test 2 PASS: shutdown sends shutdown+exit" << std::endl;
        ++passed;
    }

    std::cout << "\n=== Step 90 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
