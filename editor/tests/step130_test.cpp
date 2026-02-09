// Step 130 TDD Test: Library API indexing via LSP
#include "LSPClient.h"
#include "LibraryIndexer.h"
#include <iostream>
#include <sstream>
#include <filesystem>

struct FakeTransport : public LSPTransport {
    std::vector<std::string> sent;
    void send(const std::string& msg) override { sent.push_back(msg); }
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

    auto transport = std::make_shared<FakeTransport>();
    LSPClient client(transport);

    int symReq = client.requestWorkspaceSymbols("numpy");
    {
        nlohmann::json resp;
        resp["jsonrpc"] = "2.0";
        resp["id"] = symReq;
        resp["result"] = nlohmann::json::array({
            {{"name", "numpy.array"}, {"kind", 12}, {"containerName", "numpy"}}
        });
        client.handleMessage(resp.dump());
    }
    std::vector<LSPClient::WorkspaceSymbol> symbols;
    expect(client.takeWorkspaceSymbols(symReq, symbols), "workspace symbols retrieved", passed, failed);
    expect(symbols.size() == 1, "workspace symbols count", passed, failed);
    if (!symbols.empty()) {
        expect(symbols[0].name == "numpy.array", "workspace symbol name", passed, failed);
    }

    int compReq = client.requestLibraryCompletion("file:///tmp.py", 0, 0, "numpy.");
    {
        nlohmann::json resp;
        resp["jsonrpc"] = "2.0";
        resp["id"] = compReq;
        resp["result"] = nlohmann::json::array({
            {{"label", "numpy.zeros"}, {"kind", 3}, {"detail", "func"}}
        });
        client.handleMessage(resp.dump());
    }
    std::vector<LSPClient::CompletionItem> completions;
    expect(client.takeLibraryCompletions(compReq, completions), "library completion retrieved", passed, failed);
    expect(!completions.empty() && completions[0].label == "numpy.zeros",
           "library completion label", passed, failed);

    // Index rebuild into ExternalModule nodes
    Module mod("m1", "Module", "python");
    std::vector<DependencySpec> deps;
    deps.push_back({"numpy", "1.26.0", "requirements.txt"});
    deps.push_back({"serde", "1.0", "Cargo.toml"});

    LibraryIndexData index;
    index.symbolsByLibrary["numpy"] = symbols;
    index.completionsByLibrary["numpy"] = {"numpy.zeros"};
    rebuildExternalModules(&mod, deps, index);
    auto mods = mod.getChildren("externalModules");
    expect(mods.size() == 2, "external modules created", passed, failed);
    if (!mods.empty()) {
        auto* ext = static_cast<ExternalModule*>(mods[0]);
        auto sigs = ext->getChildren("signatures");
        expect(!sigs.empty(), "external module signatures", passed, failed);
    }

    std::cout << "\n=== Step 130 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
