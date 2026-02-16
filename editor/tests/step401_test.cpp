// Step 401: Parse AnnotationConflictExtended.h (12 tests)
// Self-hosting: parse Whetstone's own header and validate core structure.

#include <cassert>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "SelfHostHarness.h"
#include "ast/ClassDeclaration.h"
#include "ast/Function.h"
#include "ast/Variable.h"

namespace {

std::string resolveHeaderPath() {
    const std::vector<std::string> candidates = {
        "src/AnnotationConflictExtended.h",
        "editor/src/AnnotationConflictExtended.h",
        "../src/AnnotationConflictExtended.h",
        "../../editor/src/AnnotationConflictExtended.h"
    };
    for (const auto& candidate : candidates) {
        if (std::filesystem::exists(candidate)) return candidate;
    }
    return "src/AnnotationConflictExtended.h";
}

std::string kHeaderPath() {
    static const std::string path = resolveHeaderPath();
    return path;
}

ExpectedStructure expectedAnnotationConflictStructure() {
    ExpectedStructure expected;
    expected.filePath = kHeaderPath();
    expected.constructs = {
        {"struct", "CrossTypeConflict"},
        {"function", "collectCrossTypeConflicts"}
    };
    return expected;
}

const ClassDeclaration* findClass(const Module* module, const std::string& name) {
    if (!module) return nullptr;
    for (auto* clsNode : module->getChildren("classes")) {
        auto* cls = static_cast<ClassDeclaration*>(clsNode);
        if (cls->name == name) return cls;
    }
    return nullptr;
}

const Function* findFunction(const Module* module, const std::string& name) {
    if (!module) return nullptr;
    for (auto* fnNode : module->getChildren("functions")) {
        auto* fn = static_cast<Function*>(fnNode);
        if (fn->name == name) return fn;
    }
    return nullptr;
}

bool hasFieldName(const ClassDeclaration* cls, const std::string& fieldName) {
    if (!cls) return false;
    for (auto* fieldNode : cls->getChildren("fields")) {
        if (fieldNode->conceptType == "Variable") {
            auto* var = static_cast<Variable*>(fieldNode);
            if (var->name == fieldName) return true;
        }
        if (fieldNode->id.find(fieldName) != std::string::npos) return true;
    }
    return false;
}

}  // namespace

int main() {
    int passed = 0;

    // Test 1: Parse real header file from disk.
    {
        assert(std::filesystem::exists(kHeaderPath()));
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        assert(result.parseSuccess);
        assert(result.ast != nullptr);
        std::cout << "PASS: test 1 — parse real AnnotationConflictExtended.h file\n";
        passed++;
    }

    // Test 2: Struct definition is captured.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        assert(SelfHostHarness::hasParsedConstruct(result.ast.get(), "struct", "CrossTypeConflict"));
        std::cout << "PASS: test 2 — CrossTypeConflict struct captured\n";
        passed++;
    }

    // Test 3: Free function signature is captured.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        assert(SelfHostHarness::hasParsedConstruct(
            result.ast.get(), "function", "collectCrossTypeConflicts"));
        std::cout << "PASS: test 3 — collectCrossTypeConflicts signature captured\n";
        passed++;
    }

    // Test 4: Expected self-hosting structure has full coverage.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        auto expected = expectedAnnotationConflictStructure();
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 2);
        assert(result.totalParsed == 2);
        assert(result.totalOpaque == 0);
        assert(result.overallCoverage() == 1.0);
        std::cout << "PASS: test 4 — expected structure coverage 100%\n";
        passed++;
    }

    // Test 5: Struct fields are present for the self-hosting type.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        const auto* ctc = findClass(result.ast.get(), "CrossTypeConflict");
        assert(ctc != nullptr);
        assert(ctc->getChildren("fields").size() >= 4);
        std::cout << "PASS: test 5 — CrossTypeConflict fields parsed\n";
        passed++;
    }

    // Test 6: Struct field names are accessible.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        const auto* ctc = findClass(result.ast.get(), "CrossTypeConflict");
        assert(ctc != nullptr);
        assert(hasFieldName(ctc, "type1"));
        assert(hasFieldName(ctc, "type2"));
        assert(hasFieldName(ctc, "message"));
        assert(hasFieldName(ctc, "nodeId"));
        std::cout << "PASS: test 6 — CrossTypeConflict field names preserved\n";
        passed++;
    }

    // Test 7: Free function has expected parameters.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        const auto* fn = findFunction(result.ast.get(), "collectCrossTypeConflicts");
        assert(fn != nullptr);
        assert(fn->getChildren("parameters").size() >= 2);
        std::cout << "PASS: test 7 — collectCrossTypeConflicts parameters parsed\n";
        passed++;
    }

    // Test 8: Free function has a body node (parsed or opaque block).
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        const auto* fn = findFunction(result.ast.get(), "collectCrossTypeConflicts");
        assert(fn != nullptr);
        assert(fn->getChild("body") != nullptr);
        std::cout << "PASS: test 8 — collectCrossTypeConflicts body attached\n";
        passed++;
    }

    // Test 9: Coverage report includes file path and successful parse status.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        auto expected = expectedAnnotationConflictStructure();
        SelfHostHarness::checkExpected(result, expected);
        const std::string report = SelfHostHarness::coverageReport(result);
        assert(report.find(kHeaderPath()) != std::string::npos);
        assert(report.find("Parse success: yes") != std::string::npos);
        assert(report.find("100%") != std::string::npos);
        std::cout << "PASS: test 9 — coverage report reflects self-host parse\n";
        passed++;
    }

    // Test 10: Parser tolerates lambda-heavy sections from this header style.
    {
        const std::string source = R"(
inline void lambda_case() {
    auto checkPair = [&](const std::string& a, const std::string& b) {
        return a == b;
    };
    (void)checkPair("x", "y");
}
)";
        auto result = SelfHostHarness::parseSource(source, "lambda_case");
        assert(result.parseSuccess);
        assert(SelfHostHarness::countFunctions(result.ast.get()) == 1);
        std::cout << "PASS: test 10 — lambda-heavy function parses without crash\n";
        passed++;
    }

    // Test 11: Parser tolerates std::map/std::function patterns in header bodies.
    {
        const std::string source = R"(
inline void complex_types_case() {
    std::map<std::string, int> m;
    std::function<bool()> fn = []() { return true; };
    std::vector<std::pair<std::string, std::string>> pairs;
}
)";
        auto result = SelfHostHarness::parseSource(source, "complex_types_case");
        assert(result.parseSuccess);
        assert(SelfHostHarness::countFunctions(result.ast.get()) == 1);
        std::cout << "PASS: test 11 — STL-heavy function parses without crash\n";
        passed++;
    }

    // Test 12: Missing expected constructs become opaque instead of hard failure.
    {
        auto result = SelfHostHarness::parseFile(kHeaderPath());
        ExpectedStructure expected;
        expected.filePath = kHeaderPath();
        expected.constructs = {
            {"struct", "CrossTypeConflict"},
            {"function", "collectCrossTypeConflicts"},
            {"class", "DefinitelyMissingClass"},
            {"function", "definitelyMissingFunction"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 4);
        assert(result.totalParsed == 2);
        assert(result.totalOpaque == 2);
        assert(result.overallCoverage() == 0.5);
        std::cout << "PASS: test 12 — graceful degradation via opaque tracking\n";
        passed++;
    }

    std::cout << "\nStep 401 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
