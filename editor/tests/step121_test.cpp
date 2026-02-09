// Step 121 TDD Test: Import/ExternalModule/TypeSignature serialization
#include "ast/Serialization.h"
#include "ast/Import.h"
#include "ast/ExternalModule.h"
#include "ast/TypeSignature.h"
#include "ast/Type.h"
#include "ast/Module.h"
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

    // Test 1: Import round-trip
    Module root("m1", "Root", "python");
    auto* imp = new Import("imp1", "numpy", "module", "np");
    root.addChild("imports", imp);

    json j = toJson(&root);
    ASTNode* round = fromJson(j);
    expect(round && round->conceptType == "Module", "module round-trip", passed, failed);
    auto* roundMod = static_cast<Module*>(round);
    auto imports = roundMod->getChildren("imports");
    expect(imports.size() == 1 && imports[0]->conceptType == "Import", "import child preserved", passed, failed);
    if (!imports.empty()) {
        auto* ri = static_cast<Import*>(imports[0]);
        expect(ri->moduleName == "numpy" && ri->alias == "np" && ri->importKind == "module",
               "import properties", passed, failed);
    }

    deleteTree(round);

    // Test 2: ExternalModule + TypeSignature + Type children round-trip
    Module root2("m2", "Root2", "python");
    auto* ext = new ExternalModule("ext1", "numpy", "python", "1.26");
    auto* sig = new TypeSignature("sig1", "array", true);
    auto* retType = new CustomType("t1", "ndarray");
    auto* paramType = new PrimitiveType("t2", "float");
    sig->setChild("returnType", retType);
    sig->addChild("paramTypes", paramType);
    ext->addChild("signatures", sig);
    root2.addChild("externalModules", ext);

    json j2 = toJson(&root2);
    ASTNode* round2 = fromJson(j2);
    expect(round2 && round2->conceptType == "Module", "module round-trip (external)", passed, failed);
    auto* roundMod2 = static_cast<Module*>(round2);
    auto externals = roundMod2->getChildren("externalModules");
    expect(externals.size() == 1 && externals[0]->conceptType == "ExternalModule",
           "external module child", passed, failed);
    if (!externals.empty()) {
        auto* re = static_cast<ExternalModule*>(externals[0]);
        expect(re->name == "numpy" && re->version == "1.26" && re->language == "python",
               "external module properties", passed, failed);
        auto sigs = re->getChildren("signatures");
        expect(sigs.size() == 1 && sigs[0]->conceptType == "TypeSignature",
               "type signature child", passed, failed);
        if (!sigs.empty()) {
            auto* rs = static_cast<TypeSignature*>(sigs[0]);
            expect(rs->name == "array" && rs->variadic == true,
                   "type signature properties", passed, failed);
            auto* rret = rs->getChild("returnType");
            auto params = rs->getChildren("paramTypes");
            expect(rret && rret->conceptType == "CustomType", "return type child", passed, failed);
            expect(params.size() == 1 && params[0]->conceptType == "PrimitiveType",
                   "param type child", passed, failed);
        }
    }

    deleteTree(round2);

    std::cout << "\n=== Step 121 Results: " << passed << " passed, " << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
