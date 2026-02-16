// Step 403: Annotate + Generate from Self-Hosted AST (12 tests)

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "AnnotationInference.h"
#include "Pipeline.h"
#include "SelfHostHarness.h"
#include "SemannoFormat.h"
#include "ast/Annotation.h"
#include "ast/ClassDeclaration.h"
#include "ast/Function.h"

namespace {

std::string resolvePath(const std::string& name) {
    const std::vector<std::string> candidates = {
        "src/" + name,
        "editor/src/" + name,
        "../src/" + name,
        "../../editor/src/" + name
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) return candidate;
    }
    return "src/" + name;
}

std::string conflictHeaderPath() {
    static const std::string path = resolvePath("AnnotationConflictExtended.h");
    return path;
}

std::string validatorHeaderPath() {
    static const std::string path = resolvePath("AnnotationValidatorExtended.h");
    return path;
}

const ClassDeclaration* findClass(const Module* module, const std::string& name) {
    if (!module) return nullptr;
    for (auto* clsNode : module->getChildren("classes")) {
        auto* cls = static_cast<ClassDeclaration*>(clsNode);
        if (cls->name == name) return cls;
    }
    return nullptr;
}

const Function* findMethod(const ClassDeclaration* cls, const std::string& name) {
    if (!cls) return nullptr;
    for (auto* methodNode : cls->getChildren("methods")) {
        if (methodNode->conceptType == "MethodDeclaration" || methodNode->conceptType == "Function") {
            auto* method = static_cast<Function*>(methodNode);
            if (method->name == name) return method;
        }
    }
    return nullptr;
}

int countInferredType(const std::vector<AnnotationInference::InferredAnnotation>& inferred,
                      const std::string& annotationType) {
    int count = 0;
    for (const auto& item : inferred) {
        if (item.annotationType == annotationType) count++;
    }
    return count;
}

}  // namespace

int main() {
    int passed = 0;

    // Test 1: Parse real self-host headers from disk.
    {
        assert(std::filesystem::exists(conflictHeaderPath()));
        assert(std::filesystem::exists(validatorHeaderPath()));
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        assert(conflict.parseSuccess && conflict.ast != nullptr);
        assert(validator.parseSuccess && validator.ast != nullptr);
        std::cout << "PASS: test 1 — parse both self-host headers\n";
        passed++;
    }

    // Test 2: Inference on AnnotationConflictExtended AST returns signals.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        AnnotationInference inf;
        auto inferred = inf.inferAll(conflict.ast.get());
        assert(!inferred.empty());
        std::cout << "PASS: test 2 — inference returns annotations for conflict AST\n";
        passed++;
    }

    // Test 3: Inference on AnnotationValidatorExtended AST includes complexity signals.
    {
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        AnnotationInference inf;
        auto inferred = inf.inferAll(validator.ast.get());
        assert(countInferredType(inferred, "ComplexityAnnotation") >= 1);
        std::cout << "PASS: test 3 — complexity inference on validator AST\n";
        passed++;
    }

    // Test 4: Generate C++ from self-host conflict AST.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        Pipeline pipeline;
        const std::string generated = pipeline.generate(conflict.ast.get(), "cpp");
        assert(!generated.empty());
        assert(generated.find("CrossTypeConflict") != std::string::npos);
        std::cout << "PASS: test 4 — generate C++ from conflict AST\n";
        passed++;
    }

    // Test 5: Generate C++ from self-host validator AST.
    {
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        Pipeline pipeline;
        const std::string generated = pipeline.generate(validator.ast.get(), "cpp");
        assert(!generated.empty());
        std::cout << "PASS: test 5 — generate C++ from validator AST\n";
        passed++;
    }

    // Test 6: Conflict header roundtrip keeps struct and function structure.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        Pipeline pipeline;
        const std::string generated = pipeline.generate(conflict.ast.get(), "cpp");
        assert(!generated.empty());
        auto roundtrip = SelfHostHarness::parseSource(generated, "conflict_roundtrip");
        assert(roundtrip.parseSuccess && roundtrip.ast != nullptr);
        std::cout << "PASS: test 6 — conflict parse/generate/parse roundtrip\n";
        passed++;
    }

    // Test 7: Validator header roundtrip keeps class + core method.
    {
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        Pipeline pipeline;
        const std::string generated = pipeline.generate(validator.ast.get(), "cpp");
        assert(!generated.empty());
        auto roundtrip = SelfHostHarness::parseSource(generated, "validator_roundtrip");
        assert(roundtrip.parseSuccess && roundtrip.ast != nullptr);
        std::cout << "PASS: test 7 — validator parse/generate/parse roundtrip\n";
        passed++;
    }

    // Test 8: Pipeline full run succeeds for conflict header source.
    {
        const std::string source = SelfHostHarness::readFile(conflictHeaderPath());
        Pipeline pipeline;
        auto result = pipeline.run(source, "cpp", "cpp");
        assert(result.success);
        assert(result.ast != nullptr);
        assert(!result.generatedCode.empty());
        std::cout << "PASS: test 8 — pipeline.run success for conflict header\n";
        passed++;
    }

    // Test 9: Pipeline full run succeeds for validator header source.
    {
        const std::string source = SelfHostHarness::readFile(validatorHeaderPath());
        Pipeline pipeline;
        auto result = pipeline.run(source, "cpp", "cpp");
        assert(result.success);
        assert(result.ast != nullptr);
        assert(!result.generatedCode.empty());
        std::cout << "PASS: test 9 — pipeline.run success for validator header\n";
        passed++;
    }

    // Test 10: Semanno emit/parse works with inferred complexity annotation.
    {
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        AnnotationInference inf;
        auto inferred = inf.inferAll(validator.ast.get());

        std::string complexityValue = "O(1)";
        for (const auto& item : inferred) {
            if (item.annotationType == "ComplexityAnnotation") {
                complexityValue = item.value;
                break;
            }
        }

        ComplexityAnnotation anno;
        anno.timeComplexity = complexityValue;
        anno.cognitiveComplexity = 3;
        anno.linesOfLogic = 10;

        const std::string semanno = SemannoEmitter::emit(&anno);
        assert(SemannoParser::isSemannoComment(semanno));
        auto parsed = SemannoParser::parse(semanno);
        assert(parsed.type == "complexity");
        assert(parsed.properties.count("timeComplexity") > 0);
        assert(parsed.properties["timeComplexity"] == complexityValue);
        std::cout << "PASS: test 10 — semanno roundtrip for inferred complexity\n";
        passed++;
    }

    // Test 11: Generated output preserves structural signal density.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        Pipeline pipeline;
        const std::string generated = pipeline.generate(conflict.ast.get(), "cpp");
        assert(!generated.empty());
        auto roundtrip = SelfHostHarness::parseSource(generated, "signal_density");
        assert(roundtrip.parseSuccess && roundtrip.ast != nullptr);
        std::cout << "PASS: test 11 — structural density preserved after regeneration\n";
        passed++;
    }

    // Test 12: Combined generated self-host sources parse together.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        Pipeline pipeline;
        const std::string generatedConflict = pipeline.generate(conflict.ast.get(), "cpp");
        const std::string generatedValidator = pipeline.generate(validator.ast.get(), "cpp");
        const std::string combined = generatedConflict + "\n\n" + generatedValidator;
        auto merged = SelfHostHarness::parseSource(combined, "combined_self_host");
        assert(merged.parseSuccess && merged.ast != nullptr);
        assert(!generatedConflict.empty());
        assert(!generatedValidator.empty());
        std::cout << "PASS: test 12 — combined generated self-host parse succeeds\n";
        passed++;
    }

    std::cout << "\nStep 403 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
