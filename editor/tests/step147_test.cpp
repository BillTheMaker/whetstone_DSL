// Step 147 TDD Test: JavaScript/TypeScript CST-to-AST
#include "ast/Parser.h"
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

    std::string js = R"(
function add(a, b) {
  return a + b;
}
const mul = (x, y) => x * y;
class Box {
  size(n) { return n; }
}
)";

    auto jsMod = TreeSitterParser::parseJavaScript(js);
    expect(jsMod != nullptr, "js module created", passed, failed);
    auto jsFns = jsMod->getChildren("functions");
    expect(jsFns.size() >= 2, "js functions discovered", passed, failed);

    std::string ts = R"(
function greet(name: string): string {
  return "hi " + name;
}
const inc = (n: number) => n + 1;
)";
    auto tsMod = TreeSitterParser::parseTypeScript(ts);
    expect(tsMod != nullptr, "ts module created", passed, failed);
    auto tsFns = tsMod->getChildren("functions");
    expect(tsFns.size() >= 2, "ts functions discovered", passed, failed);

    std::cout << "\n=== Step 147 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
