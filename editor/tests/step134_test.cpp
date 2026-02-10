// Step 134 TDD Test: PrimitivesRegistry aggregation
#include "PrimitivesRegistry.h"
#include "ast/Function.h"
#include "ast/Variable.h"
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

static bool containsName(const std::vector<PrimitiveSymbol>& list, const std::string& name) {
    for (const auto& s : list) {
        if (s.name == name) return true;
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "Module", "python");
    auto* fn = new Function("fn1", "user_fn");
    auto* var = new Variable("var1", "USER_CONST");
    mod.addChild("functions", fn);
    mod.addChild("variables", var);

    auto* ext = new ExternalModule("ext_numpy", "numpy", "python", "1.0");
    ext->addChild("signatures", new TypeSignature("sig_1", "numpy.array"));
    ext->addChild("signatures", new TypeSignature("sig_2", "Matrix"));
    mod.addChild("externalModules", ext);

    PrimitivesRegistry reg;
    reg.setRoot(&mod);
    reg.setLanguage("python");

    auto funcs = reg.getAvailableFunctions("fn1");
    expect(containsName(funcs, "print"), "builtin function present", passed, failed);
    expect(containsName(funcs, "numpy.array"), "import function present", passed, failed);
    expect(containsName(funcs, "user_fn"), "user function present", passed, failed);

    auto types = reg.getAvailableTypes("fn1");
    expect(containsName(types, "Matrix"), "import type present", passed, failed);
    expect(containsName(types, "list"), "builtin type present", passed, failed);

    auto constants = reg.getAvailableConstants("fn1");
    expect(containsName(constants, "USER_CONST"), "user constant present", passed, failed);

    std::cout << "\n=== Step 134 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
