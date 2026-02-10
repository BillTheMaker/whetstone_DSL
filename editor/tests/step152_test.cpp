// Step 152 TDD Test: Cross-language projection for new languages
#include "CrossLanguageProjector.h"
#include "ast/Annotation.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Statement.h"
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

static bool hasReclaimAnnotation(const ASTNode* node, std::string* strategyOut = nullptr) {
    if (!node) return false;
    for (auto* anno : node->getChildren("annotations")) {
        if (anno->conceptType == "ReclaimAnnotation") {
            if (strategyOut) {
                auto* ra = static_cast<const ReclaimAnnotation*>(anno);
                *strategyOut = ra->strategy;
            }
            return true;
        }
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    Module src("mod_src", "Demo", "python");
    auto* fn = new Function("fn_add", "add");
    auto* reclaim = new ReclaimAnnotation("anno_gc", "Tracing");
    fn->addChild("annotations", reclaim);

    auto* ret = new Return();
    ret->id = "ret";
    ret->setChild("value", new IntegerLiteral("int_one", 1));
    fn->addChild("body", ret);
    src.addChild("functions", fn);

    CrossLanguageProjector projector;

    auto rust = projector.project(&src, "rust");
    expect(rust != nullptr, "rust projection created", passed, failed);
    expect(rust && rust->targetLanguage == "rust", "rust target language set", passed, failed);

    auto go = projector.project(&src, "go");
    expect(go != nullptr, "go projection created", passed, failed);
    expect(go && go->targetLanguage == "go", "go target language set", passed, failed);

    bool reclaimFound = false;
    std::string strategy;
    if (go) {
        auto fns = go->getChildren("functions");
        if (!fns.empty() && fns[0]->conceptType == "Function") {
            reclaimFound = hasReclaimAnnotation(fns[0], &strategy);
        }
    }
    expect(reclaimFound, "reclaim annotation preserved", passed, failed);
    expect(strategy == "Escape", "reclaim adapted for go", passed, failed);

    std::cout << "\n=== Step 152 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
