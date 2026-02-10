// Step 138 TDD Test: Type-aware code generation
#include "ast/CppGenerator.h"
#include "ast/Module.h"
#include "ast/Variable.h"
#include "ast/Expression.h"
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

    Module mod("m1", "Module", "cpp");
    auto* var = new Variable("v1", "arr");
    auto* call = new FunctionCall();
    call->functionName = "numpy.array";
    var->setChild("initializer", call);
    mod.addChild("variables", var);

    CppGenerator gen;
    std::string out = gen.generate(&mod);
    expect(out.find("std::vector<double> arr") != std::string::npos,
           "mapped numpy.array to std::vector<double>", passed, failed);

    Module mod2("m2", "Module", "cpp");
    auto* var2 = new Variable("v2", "val");
    auto* call2 = new FunctionCall();
    call2->functionName = "custom.fn";
    var2->setChild("initializer", call2);
    mod2.addChild("variables", var2);
    std::string out2 = gen.generate(&mod2);
    expect(out2.find("auto val") != std::string::npos,
           "unknown call keeps auto", passed, failed);

    std::cout << "\n=== Step 138 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
