// Step 94a TDD Test: AST source span tracking
//
// Tests:
// 1. Parser assigns spans to AST nodes
// 2. Serialization preserves spans

#include <cassert>
#include <iostream>
#include "ast/Parser.h"
#include "ast/Serialization.h"

int main() {
    int passed = 0;
    int failed = 0;

    std::string source = "def foo(x):\n  return x\n";
    auto mod = TreeSitterParser::parsePython(source);
    assert(mod);
    auto funcs = mod->getChildren("functions");
    assert(!funcs.empty());
    auto* fn = funcs[0];
    assert(fn->hasSpan());
    assert(fn->spanStartLine == 0);
    assert(fn->spanEndLine >= 1);
    std::cout << "Test 1 PASS: parser spans set" << std::endl;
    ++passed;

    auto j = toJson(fn);
    assert(j.contains("span"));
    assert(j["span"].contains("start"));
    auto* round = fromJson(j);
    assert(round->hasSpan());
    assert(round->spanStartLine == fn->spanStartLine);
    std::cout << "Test 2 PASS: serialization preserves spans" << std::endl;
    ++passed;

    std::cout << "\n=== Step 94a Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
