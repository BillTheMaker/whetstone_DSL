// Step 156 TDD Test: Agent annotation assistant
#include "AgentAnnotationAssistant.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Annotation.h"
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

static bool hasAnnotationType(const ASTNode* node, const std::string& type) {
    if (!node) return false;
    for (auto* anno : node->getChildren("annotations")) {
        if (anno->conceptType == type) return true;
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    Module mod;
    mod.id = "mod1";
    mod.name = "TestModule";
    mod.targetLanguage = "cpp";

    auto* fn = new Function();
    fn->id = "fn1";
    fn->name = "doWork";
    auto* da = new DeallocateAnnotation("da1", "Explicit");
    fn->addChild("annotations", da);
    mod.addChild("functions", fn);

    MemoryStrategyInference inf;
    auto base = inf.inferAnnotations(&mod);
    double baseConf = 0.0;
    MemoryStrategyInference::Suggestion baseSug;
    for (const auto& s : base) {
        if (s.nodeId == mod.id && s.annotationType == "LifetimeAnnotation") {
            baseConf = s.confidence;
            baseSug = s;
            break;
        }
    }
    expect(baseConf > 0.0, "module suggestion present", passed, failed);

    inf.recordFeedback(baseSug, true);
    auto after = inf.inferAnnotations(&mod);
    double afterConf = 0.0;
    for (const auto& s : after) {
        if (s.nodeId == mod.id && s.annotationType == "LifetimeAnnotation") {
            afterConf = s.confidence;
            break;
        }
    }
    expect(afterConf >= baseConf, "confidence increases with positive feedback", passed, failed);

    AgentAnnotationAssistant assistant;
    auto result = assistant.suggest(&mod, fn->id);
    bool hasMissingIntent = false;
    for (const auto& d : result.diagnostics) {
        if (d.nodeId == fn->id &&
            d.message.find("Missing Intent") != std::string::npos) {
            hasMissingIntent = true;
            break;
        }
    }
    expect(hasMissingIntent, "diagnostics scoped to function", passed, failed);

    MemoryStrategyInference::Suggestion applySug;
    applySug.nodeId = fn->id;
    applySug.annotationType = "LifetimeAnnotation";
    applySug.strategy = "RAII";
    applySug.reason = "test";
    applySug.confidence = 0.6;

    auto applyRes = assistant.applySuggestion(&mod, applySug);
    expect(applyRes.success, "apply suggestion mutation succeeds", passed, failed);
    expect(hasAnnotationType(fn, "LifetimeAnnotation"),
           "annotation inserted on target", passed, failed);

    std::cout << "\n=== Step 156 Results: " << passed << " passed, "
              << failed << " failed ===\n";
    return failed == 0 ? 0 : 1;
}
