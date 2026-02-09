// Step 103 TDD Test: Side-by-side generated code view
//
// Tests:
// 1. Generated code for python includes function name

#include <cassert>
#include <iostream>
#include "ast/Generator.h"
#include "ast/Module.h"
#include "ast/Function.h"

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "mod", "python");
    auto* fn = new Function("f1", "foo");
    mod.addChild("functions", fn);

    PythonGenerator gen;
    auto out = gen.generate(&mod);
    assert(out.find("def foo") != std::string::npos);
    std::cout << "Test 1 PASS: generated code includes function" << std::endl;
    ++passed;

    std::cout << "\n=== Step 103 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
