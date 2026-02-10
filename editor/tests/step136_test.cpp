// Step 136 TDD Test: Agent library-aware mode policy
#include "AgentLibraryPolicy.h"
#include "ast/Function.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
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

    Module mod("m1", "Module", "python");
    auto* fn = new Function("fn1", "user_fn");
    mod.addChild("functions", fn);
    auto* ext = new ExternalModule("ext_numpy", "numpy", "python", "1.0");
    ext->addChild("signatures", new TypeSignature("sig_1", "numpy.array"));
    mod.addChild("externalModules", ext);

    auto* callKnown = new FunctionCall();
    callKnown->functionName = "numpy.array";
    auto resKnown = checkMutationLibraryPolicy(callKnown, &mod, true, false);
    expect(resKnown.warning.empty(), "known function no warning", passed, failed);

    auto* callUnknown = new FunctionCall();
    callUnknown->functionName = "mystery";
    auto resWarn = checkMutationLibraryPolicy(callUnknown, &mod, true, false);
    expect(!resWarn.warning.empty(), "unknown function warning", passed, failed);
    expect(!resWarn.unknownFunctions.empty(), "unknown list set", passed, failed);

    auto resStrict = checkMutationLibraryPolicy(callUnknown, &mod, true, true);
    expect(!resStrict.ok, "strict mode blocks unknown", passed, failed);

    delete callKnown;
    delete callUnknown;

    std::cout << "\n=== Step 136 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
