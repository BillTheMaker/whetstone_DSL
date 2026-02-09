// Step 96 TDD Test: LSP server configuration settings
//
// Tests:
// 1. Default LSP configs are present
// 2. Schema validation flags illegal child

#include <cassert>
#include <iostream>
#include "SettingsManager.h"
#include "ast/Module.h"
#include "ast/Function.h"

int main() {
    int passed = 0;
    int failed = 0;

    SettingsManager settings;
    auto* py = settings.getServer("python");
    auto* cpp = settings.getServer("cpp");
    assert(py && cpp);
    assert(py->server == "pylsp");
    assert(cpp->server == "clangd");
    std::cout << "Test 1 PASS: default LSP configs" << std::endl;
    ++passed;

    Module* mod = new Module("m1", "mod", "python");
    Function* fn = new Function("f1", "foo");
    mod->addChild("variables", fn); // illegal: variables should be Variable

    std::vector<std::string> errors;
    bool ok = settings.validateSchema(mod, errors);
    assert(!ok);
    assert(!errors.empty());
    std::cout << "Test 2 PASS: schema validation catches illegal child" << std::endl;
    ++passed;

    std::cout << "\n=== Step 96 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
