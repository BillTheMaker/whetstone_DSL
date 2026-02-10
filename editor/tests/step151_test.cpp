// Step 151 TDD Test: Go CST-to-AST + generator
#include "ast/Parser.h"
#include "ast/Generator.h"
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

    std::string go = R"(
package main

import "fmt"

type Box struct {
  value int
}

func (b *Box) Size(n int) int {
  return n + 1
}

func Add(a int, b int) int {
  c := a + b
  return c
}
)";

    auto mod = TreeSitterParser::parseGo(go);
    expect(mod != nullptr, "go module created", passed, failed);
    auto funcs = mod->getChildren("functions");
    expect(funcs.size() >= 2, "go functions discovered", passed, failed);

    bool foundAdd = false;
    bool foundMethod = false;
    for (const auto* fnNode : funcs) {
        if (fnNode->conceptType != "Function") continue;
        auto* fn = static_cast<const Function*>(fnNode);
        if (fn->name == "Add") foundAdd = true;
        if (fn->name.find("Box.") == 0) foundMethod = true;
    }
    expect(foundAdd, "go top-level function name", passed, failed);
    expect(foundMethod, "go method name with receiver", passed, failed);

    GoGenerator gen;
    std::string out = gen.generate(mod.get());
    expect(out.find("package") != std::string::npos,
           "go package generated", passed, failed);
    expect(out.find("import") != std::string::npos,
           "go imports generated", passed, failed);
    expect(out.find("func Add") != std::string::npos,
           "go function signature generated", passed, failed);
    expect(out.find("func (b *Box)") != std::string::npos,
           "go method receiver generated", passed, failed);

    std::cout << "\n=== Step 151 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
