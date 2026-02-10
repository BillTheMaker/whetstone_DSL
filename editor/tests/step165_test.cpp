// Step 165 TDD Test: Sprint 5 integration coverage
#include "Pipeline.h"
#include "PrimitivesRegistry.h"
#include "AgentCodeGen.h"
#include "CrossLanguageProjector.h"
#include "CompositionBuilder.h"
#include "AgentLibraryPolicy.h"
#include "EmacsFunctionDiscovery.h"
#include "ast/Serialization.h"
#include "ast/Module.h"
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

int main() {
    int passed = 0;
    int failed = 0;

    // Library-aware primitives + agent codegen
    Module mod("m1", "Module", "python");
    auto* ext = new ExternalModule("ext_numpy", "numpy", "python");
    ext->addChild("signatures", new TypeSignature("sig_array", "numpy.array"));
    mod.addChild("externalModules", ext);

    PrimitivesRegistry registry;
    registry.setRoot(&mod);
    registry.setLanguage("python");
    auto funcs = registry.getAvailableFunctions();
    bool foundArray = false;
    for (const auto& f : funcs) {
        if (f.name == "numpy.array") {
            foundArray = true;
            break;
        }
    }
    expect(foundArray, "primitives registry includes import", passed, failed);

    AgentCodeGen gen;
    auto genRes = gen.generate("create array", registry, "python", true);
    expect(genRes.node != nullptr, "agent codegen returns node", passed, failed);
    deleteTree(genRes.node);

    // Composition builder
    std::vector<PrimitiveSymbol> prims = registry.getAvailableFunctions();
    auto suggestions = suggestCompositionSteps(prims, "", "");
    std::string code = buildCompositionFunction(suggestions, "compose_pipeline", "x");
    expect(!code.empty(), "composition builder output", passed, failed);

    // Cross-language projection
    CrossLanguageProjector projector;
    auto projected = projector.project(&mod, "rust");
    expect(projected != nullptr, "cross-language projection", passed, failed);

    // Pipeline parse + generate
    Pipeline pipeline;
    auto res = pipeline.run("def add(a,b):\n    return a+b\n", "python", "python");
    expect(res.success, "pipeline run", passed, failed);
    expect(!res.generatedCode.empty(), "pipeline generates code", passed, failed);

    // Emacs function index -> external modules
    EmacsFunctionIndex emacsIndex;
    emacsIndex.functionsByPackage["cl-lib"] = {
        {"cl-mapcar", "(cl-mapcar fn list)", "Mapcar doc"}
    };
    int sigId = 0;
    appendEmacsExternalModules(&mod, emacsIndex, sigId);
    bool hasEmacs = false;
    for (auto* node : mod.getChildren("externalModules")) {
        auto* em = static_cast<ExternalModule*>(node);
        if (em->name == "cl-lib") hasEmacs = true;
    }
    expect(hasEmacs, "emacs external module appended", passed, failed);

    // Agent library policy warning path
    LibraryPolicyResult policy = checkMutationLibraryPolicy(ext, &mod, true, false);
    expect(policy.ok, "library policy allows known import", passed, failed);

    std::cout << "\n=== Step 165 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
