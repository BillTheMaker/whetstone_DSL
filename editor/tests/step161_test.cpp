// Step 161 TDD Test: Plugin API + loader
#include "PluginLoader.h"
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

static bool FakeInit(const PluginHost* host, PluginDescriptor* outDesc) {
    outDesc->name = "FakePlugin";
    outDesc->version = "0.1";
    outDesc->apiVersion = "1";
    outDesc->author = "Test";
    outDesc->entrySymbol = "WhetstonePluginInit";

    host->registerConcept(host->ctx, "ExternalThing");
    host->registerGenerator(host->ctx, "zig");
    host->registerGrammar(host->ctx, "zig");
    host->registerAnnotation(host->ctx, "TraceSafe");
    host->registerPanel(host->ctx, "CustomPanel");
    host->registerCommand(host->ctx, "plugin.command");
    host->registerKeybinding(host->ctx, "plugin.command", "Ctrl+Alt+P");
    return true;
}

int main() {
    int passed = 0;
    int failed = 0;

    PluginLoader loader;
    std::string error;
    expect(loader.loadFromEntryPoint(&FakeInit, "fake", error),
           "load from entry point", passed, failed);

    const auto& registry = loader.registry();
    expect(registry.concepts.size() == 1, "concept registered", passed, failed);
    expect(registry.generators.size() == 1, "generator registered", passed, failed);
    expect(registry.grammars.size() == 1, "grammar registered", passed, failed);
    expect(registry.annotations.size() == 1, "annotation registered", passed, failed);
    expect(registry.panels.size() == 1, "panel registered", passed, failed);
    expect(registry.commands.size() == 1, "command registered", passed, failed);
    expect(registry.keybindings.size() == 1, "keybinding registered", passed, failed);

    expect(loader.plugins().size() == 1, "plugin tracked", passed, failed);
    expect(loader.plugins()[0].descriptor.name == "FakePlugin",
           "descriptor stored", passed, failed);

    std::cout << "\n=== Step 161 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
