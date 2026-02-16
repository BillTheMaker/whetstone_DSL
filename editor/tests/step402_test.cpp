// Step 402: Parse AnnotationValidatorExtended.h (12 tests)
// Self-hosting: parse class-heavy validator header with static helpers and casts.

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "AnnotationInference.h"
#include "SelfHostHarness.h"
#include "ast/ClassDeclaration.h"
#include "ast/CppAdvancedNodes.h"
#include "ast/Function.h"

namespace {

std::string resolveHeaderPath() {
    const std::vector<std::string> candidates = {
        "src/AnnotationValidatorExtended.h",
        "editor/src/AnnotationValidatorExtended.h",
        "../src/AnnotationValidatorExtended.h",
        "../../editor/src/AnnotationValidatorExtended.h"
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) return candidate;
    }
    return "src/AnnotationValidatorExtended.h";
}

std::string headerPath() {
    static const std::string path = resolveHeaderPath();
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

int countConceptRecursive(const ASTNode* node, const std::string& conceptType) {
    if (!node) return 0;
    int total = (node->conceptType == conceptType) ? 1 : 0;
    for (auto* child : node->allChildren()) {
        total += countConceptRecursive(child, conceptType);
    }
    return total;
}

int countStaticMethods(const ClassDeclaration* cls) {
    if (!cls) return 0;
    int count = 0;
    for (auto* methodNode : cls->getChildren("methods")) {
        if (methodNode->conceptType == "MethodDeclaration") {
            auto* method = static_cast<MethodDeclaration*>(methodNode);
            if (method->isStatic) count++;
        }
    }
    return count;
}

}  // namespace

int main() {
    int passed = 0;

    // Test 1: Parse real validator header file from disk.
    {
        assert(std::filesystem::exists(headerPath()));
        auto result = SelfHostHarness::parseFile(headerPath());
        assert(result.parseSuccess);
        assert(result.ast != nullptr);
        std::cout << "PASS: test 1 — parse real AnnotationValidatorExtended.h file\n";
        passed++;
    }

    // Test 2: AnnotationValidatorExtended class is captured.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        assert(SelfHostHarness::hasParsedConstruct(
            result.ast.get(), "class", "AnnotationValidatorExtended"));
        std::cout << "PASS: test 2 — class AnnotationValidatorExtended captured\n";
        passed++;
    }

    // Test 3: Class has parsed method entries.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        const auto* cls = findClass(result.ast.get(), "AnnotationValidatorExtended");
        assert(cls != nullptr);
        assert(cls->getChildren("methods").size() >= 3);
        std::cout << "PASS: test 3 — class has method entries\n";
        passed++;
    }

    // Test 4: Public validate method signature is captured.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        const auto* cls = findClass(result.ast.get(), "AnnotationValidatorExtended");
        const auto* method = findMethod(cls, "validate");
        assert(method != nullptr);
        assert(method->getChildren("parameters").size() >= 1);
        std::cout << "PASS: test 4 — validate method signature captured\n";
        passed++;
    }

    // Test 5: Private helper methods are captured.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        const auto* cls = findClass(result.ast.get(), "AnnotationValidatorExtended");
        assert(findMethod(cls, "validateNode") != nullptr);
        assert(findMethod(cls, "validateAnnotation") != nullptr);
        std::cout << "PASS: test 5 — private helper methods captured\n";
        passed++;
    }

    // Test 6: Static helper methods are captured as static methods.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        const auto* cls = findClass(result.ast.get(), "AnnotationValidatorExtended");
        assert(findMethod(cls, "isPowerOf2") != nullptr);
        assert(findMethod(cls, "isOneOf") != nullptr);
        assert(countStaticMethods(cls) >= 2);
        std::cout << "PASS: test 6 — static helper methods parsed\n";
        passed++;
    }

    // Test 7: Self-host expected structure has full coverage.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        ExpectedStructure expected;
        expected.filePath = headerPath();
        expected.constructs = {{"class", "AnnotationValidatorExtended"}};
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 1);
        assert(result.totalParsed == 1);
        assert(result.totalOpaque == 0);
        assert(result.overallCoverage() == 1.0);
        std::cout << "PASS: test 7 — expected structure coverage 100%\n";
        passed++;
    }

    // Test 8: static_cast patterns from validator logic are recognized by cast helpers.
    {
        const std::string expr1 = "static_cast<const BitWidthAnnotation*>(anno)";
        const std::string expr2 = "static_cast<const LayoutAnnotation*>(anno)";
        assert(detectCastKind(expr1) == "static_cast");
        assert(detectCastKind(expr2) == "static_cast");
        assert(castRiskLevel(detectCastKind(expr1)) == "low");
        std::cout << "PASS: test 8 — static_cast detection helpers match validator patterns\n";
        passed++;
    }

    // Test 9: Inference produces complexity signals on parsed methods.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        AnnotationInference inf;
        auto inferred = inf.inferAll(result.ast.get());
        int complexityCount = 0;
        for (const auto& item : inferred) {
            if (item.annotationType == "ComplexityAnnotation") complexityCount++;
        }
        assert(complexityCount >= 1);
        std::cout << "PASS: test 9 — complexity inference runs on self-host AST\n";
        passed++;
    }

    // Test 10: Targeted class snippet with static helpers parses cleanly.
    {
        const std::string source = R"(
class ValidatorLite {
public:
    static bool ok(int x) { return x > 0; }
    static bool positive(int x) { return x >= 0; }
    int run(int value) { return value + 1; }
};
)";
        auto result = SelfHostHarness::parseSource(source, "validator_lite");
        assert(result.parseSuccess);
        assert(SelfHostHarness::hasParsedConstruct(result.ast.get(), "class", "ValidatorLite"));
        const auto* cls = findClass(result.ast.get(), "ValidatorLite");
        assert(cls != nullptr);
        assert(countStaticMethods(cls) >= 2);
        std::cout << "PASS: test 10 — static helper snippet parse stable\n";
        passed++;
    }

    // Test 11: Coverage report includes real file path and parse status.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        ExpectedStructure expected;
        expected.filePath = headerPath();
        expected.constructs = {{"class", "AnnotationValidatorExtended"}};
        SelfHostHarness::checkExpected(result, expected);
        const std::string report = SelfHostHarness::coverageReport(result);
        assert(report.find(headerPath()) != std::string::npos);
        assert(report.find("Parse success: yes") != std::string::npos);
        assert(report.find("100%") != std::string::npos);
        std::cout << "PASS: test 11 — coverage report content checks\n";
        passed++;
    }

    // Test 12: Missing expected constructs are counted as opaque, not fatal.
    {
        auto result = SelfHostHarness::parseFile(headerPath());
        ExpectedStructure expected;
        expected.filePath = headerPath();
        expected.constructs = {
            {"class", "AnnotationValidatorExtended"},
            {"function", "definitelyMissingFunction"},
            {"class", "DefinitelyMissingClass"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 3);
        assert(result.totalParsed == 1);
        assert(result.totalOpaque == 2);
        assert(result.overallCoverage() == (1.0 / 3.0));
        std::cout << "PASS: test 12 — graceful degradation via opaque tracking\n";
        passed++;
    }

    std::cout << "\nStep 402 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
