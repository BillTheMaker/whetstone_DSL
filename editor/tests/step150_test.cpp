// Step 150 TDD Test: Rust CST-to-AST + generator
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

    std::string rust = R"(
use std::collections::HashMap;

struct Boxy { value: i32 }

impl Boxy {
  fn size(&self, n: i32) -> i32 { return n + 1; }
}

fn add(a: i32, b: i32) -> i32 {
  let c = a + b;
  return c;
}
)";

    auto mod = TreeSitterParser::parseRust(rust);
    expect(mod != nullptr, "rust module created", passed, failed);
    auto funcs = mod->getChildren("functions");
    expect(funcs.size() >= 2, "rust functions discovered", passed, failed);

    bool foundAdd = false;
    bool foundImpl = false;
    for (const auto* fnNode : funcs) {
        if (fnNode->conceptType != "Function") continue;
        auto* fn = static_cast<const Function*>(fnNode);
        if (fn->name == "add") foundAdd = true;
        if (fn->name == "Boxy.size") foundImpl = true;
    }
    expect(foundAdd, "rust top-level function name", passed, failed);
    expect(foundImpl, "rust impl method name", passed, failed);

    RustGenerator gen;
    std::string out = gen.generate(mod.get());
    expect(out.find("use std::collections::HashMap;") != std::string::npos,
           "rust imports generated", passed, failed);
    expect(out.find("fn add") != std::string::npos,
           "rust function signature generated", passed, failed);
    expect(out.find("impl Boxy") != std::string::npos,
           "rust impl block generated", passed, failed);

    std::cout << "\n=== Step 150 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
