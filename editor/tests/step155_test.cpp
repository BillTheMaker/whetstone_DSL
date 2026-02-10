// Step 155 TDD Test: Agent library-aware code generation
#include "AgentCodeGen.h"
#include "ast/Module.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include "ast/Serialization.h"
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

static bool containsFunctionCall(ASTNode* node, const std::string& name) {
    if (!node) return false;
    if (node->conceptType == "FunctionCall") {
        auto* call = static_cast<FunctionCall*>(node);
        return call->functionName == name;
    }
    for (auto* child : node->allChildren()) {
        if (containsFunctionCall(child, name)) return true;
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    Module mod("m1", "Module", "python");
    auto* ext = new ExternalModule("ext_numpy", "numpy", "python");
    ext->addChild("signatures", new TypeSignature("sig_array", "numpy.array"));
    mod.addChild("externalModules", ext);

    PrimitivesRegistry reg;
    reg.setRoot(&mod);
    reg.setLanguage("python");

    AgentCodeGen gen;
    auto res = gen.generate("create array", reg, "python", true);
    expect(res.node != nullptr, "agent code generated", passed, failed);
    expect(containsFunctionCall(res.node, "numpy.array"),
           "imports preferred for generation", passed, failed);
    deleteTree(res.node);

    Module mod2("m2", "Module", "python");
    PrimitivesRegistry reg2;
    reg2.setRoot(&mod2);
    reg2.setLanguage("python");
    auto res2 = gen.generate("print", reg2, "python", true);
    expect(res2.node != nullptr, "fallback generation", passed, failed);
    deleteTree(res2.node);

    std::cout << "\n=== Step 155 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
