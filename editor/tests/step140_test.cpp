// Step 140 TDD Test: Constructive coding tests (integration-style)
#include "CompletionUtils.h"
#include "PrimitivesRegistry.h"
#include "AgentLibraryPolicy.h"
#include "CompositionBuilder.h"
#include "LibraryCompatibility.h"
#include "ast/CppGenerator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Expression.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
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

static bool containsLabel(const std::vector<LSPClient::CompletionItem>& items,
                          const std::string& label) {
    for (const auto& item : items) {
        if (item.label == label) return true;
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    // 1. Import numpy -> completions prioritize numpy
    Module mod("m1", "Module", "python");
    auto* ext = new ExternalModule("ext_numpy", "numpy", "python", "1.0");
    ext->addChild("signatures", new TypeSignature("sig_1", "numpy.array"));
    mod.addChild("externalModules", ext);

    PrimitivesRegistry reg;
    reg.setRoot(&mod);
    reg.setLanguage("python");
    auto funcs = reg.getAvailableFunctions("m1");
    std::vector<LSPClient::CompletionItem> baseItems = {{"foo", 0, "", "foo", ""}};
    auto built = buildLibraryAwareCompletions(baseItems, funcs, "");
    expect(containsLabel(built.items, "numpy.array"), "imported numpy in completions", passed, failed);

    // 2. Remove import -> primitives shrink
    Module mod2("m2", "Module", "python");
    PrimitivesRegistry reg2;
    reg2.setRoot(&mod2);
    reg2.setLanguage("python");
    auto funcs2 = reg2.getAvailableFunctions("m2");
    auto built2 = buildLibraryAwareCompletions(baseItems, funcs2, "");
    expect(!containsLabel(built2.items, "numpy.array"), "numpy removed from completions", passed, failed);

    // 3. preferImports warns on out-of-library usage but doesn't block
    auto* callUnknown = new FunctionCall();
    callUnknown->functionName = "mystery";
    auto policyWarn = checkMutationLibraryPolicy(callUnknown, &mod, true, false);
    expect(policyWarn.ok && !policyWarn.warning.empty(),
           "preferImports warns but allows", passed, failed);

    // 4. strictMode blocks out-of-library usage
    auto policyStrict = checkMutationLibraryPolicy(callUnknown, &mod, true, true);
    expect(!policyStrict.ok && !policyStrict.error.empty(),
           "strictMode blocks unknown", passed, failed);
    delete callUnknown;

    // 5. Cross-language projection mapping (library compatibility)
    LibraryCompatibility compat;
    compat.loadDefaultMappings();
    auto map = compat.getMappings("numpy", "python");
    bool hasEigen = false;
    for (const auto& entry : map) {
        if (entry.first == "Eigen" && entry.second == "cpp") hasEigen = true;
    }
    expect(hasEigen, "numpy->Eigen mapping exists", passed, failed);

    // 6. Composition builder generates correct code
    std::vector<CompositionStep> pipeline = {
        {"normalize", "string", "string"},
        {"tokenize", "string", "list"}
    };
    std::string comp = buildCompositionFunction(pipeline, "pipe", "input");
    expect(comp.find("return tokenize(normalize(input))") != std::string::npos,
           "composition output", passed, failed);

    // 7. Type-aware generation produces mapped type
    Module mod3("m3", "Module", "cpp");
    auto* var = new Variable("v1", "arr");
    auto* call = new FunctionCall();
    call->functionName = "numpy.array";
    var->setChild("initializer", call);
    mod3.addChild("variables", var);
    CppGenerator gen;
    std::string out = gen.generate(&mod3);
    expect(out.find("std::vector<double> arr") != std::string::npos,
           "type-aware mapping in C++", passed, failed);

    std::cout << "\n=== Step 140 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
