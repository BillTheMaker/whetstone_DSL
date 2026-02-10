// Step 154 TDD Test: Agent library context payload
#include "WebSocketServer.h"
#include "PrimitivesRegistry.h"
#include "ast/Module.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
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

    Module mod;
    mod.id = "mod_test";
    mod.name = "Sample";
    mod.targetLanguage = "python";

    auto* ext = new ExternalModule("ext_numpy", "numpy", "python");
    auto* sig = new TypeSignature("sig_array", "array");
    ext->addChild("signatures", sig);
    mod.addChild("externalModules", ext);

    PrimitivesRegistry registry;
    registry.setRoot(&mod);
    registry.setLanguage("python");

    auto transport = std::make_unique<MockWebSocketTransport>();
    MockWebSocketTransport* raw = transport.get();
    WebSocketAgentServer server(std::move(transport));

    server.setLibraryContextProvider([&](const std::string&) -> json {
        auto ctx = WebSocketAgentServer::LibraryContext::fromRegistry(registry, &mod, "python");
        return ctx.data;
    });

    server.start(8123);
    std::string sid = raw->simulateConnect();
    std::string response = raw->getLastSentMessage(sid);

    expect(!response.empty(), "library context sent", passed, failed);
    expect(response.find("libraryContext") != std::string::npos,
           "library context payload present", passed, failed);
    expect(response.find("numpy") != std::string::npos,
           "library name included", passed, failed);
    expect(response.find("array") != std::string::npos,
           "signature included", passed, failed);

    std::cout << "\n=== Step 154 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
