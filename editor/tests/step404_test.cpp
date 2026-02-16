// Step 404: Self-Hosting Progress Report + Phase Integration (8 tests)

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

namespace {

struct AggregatedCoverage {
    int expected = 0;
    int parsed = 0;
    int opaque = 0;

    double ratio() const {
        return expected > 0 ? static_cast<double>(parsed) / expected : 1.0;
    }
};

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

ExpectedStructure expectedConflict() {
    ExpectedStructure expected;
    expected.filePath = conflictHeaderPath();
    expected.constructs = {
        {"struct", "CrossTypeConflict"},
        {"function", "collectCrossTypeConflicts"}
    };
    return expected;
}

ExpectedStructure expectedValidator() {
    ExpectedStructure expected;
    expected.filePath = validatorHeaderPath();
    expected.constructs = {
        {"class", "AnnotationValidatorExtended"}
    };
    return expected;
}

AggregatedCoverage aggregate(const SelfHostResult& a, const SelfHostResult& b) {
    AggregatedCoverage out;
    out.expected = a.totalExpected + b.totalExpected;
    out.parsed = a.totalParsed + b.totalParsed;
    out.opaque = a.totalOpaque + b.totalOpaque;
    return out;
}

std::vector<std::string> collectGaps(const SelfHostResult& result) {
    std::vector<std::string> gaps;
    for (const auto& cov : result.coverage) {
        if (cov.opaque > 0) {
            gaps.push_back(cov.constructType + ":" + std::to_string(cov.opaque));
        }
    }
    return gaps;
}

}  // namespace

int main() {
    int passed = 0;

    // Test 1: Self-host coverage for both target headers.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        auto expectConflict = expectedConflict();
        auto expectValidator = expectedValidator();
        SelfHostHarness::checkExpected(conflict, expectConflict);
        SelfHostHarness::checkExpected(validator, expectValidator);
        assert(conflict.totalParsed == 2);
        assert(validator.totalParsed == 1);
        std::cout << "PASS: test 1 — per-file self-host coverage metrics\n";
        passed++;
    }

    // Test 2: Per-file coverage ratios are at full baseline.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        SelfHostHarness::checkExpected(conflict, expectedConflict());
        SelfHostHarness::checkExpected(validator, expectedValidator());
        assert(conflict.overallCoverage() == 1.0);
        assert(validator.overallCoverage() == 1.0);
        std::cout << "PASS: test 2 — baseline coverage ratios at 100%\n";
        passed++;
    }

    // Test 3: Combined phase coverage aggregates correctly.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        SelfHostHarness::checkExpected(conflict, expectedConflict());
        SelfHostHarness::checkExpected(validator, expectedValidator());
        auto totals = aggregate(conflict, validator);
        assert(totals.expected == 3);
        assert(totals.parsed == 3);
        assert(totals.opaque == 0);
        assert(totals.ratio() == 1.0);
        std::cout << "PASS: test 3 — aggregate coverage accounting\n";
        passed++;
    }

    // Test 4: Pipeline end-to-end succeeds on both self-host headers.
    {
        Pipeline pipeline;
        auto conflictSource = SelfHostHarness::readFile(conflictHeaderPath());
        auto validatorSource = SelfHostHarness::readFile(validatorHeaderPath());
        auto conflictRun = pipeline.run(conflictSource, "cpp", "cpp");
        auto validatorRun = pipeline.run(validatorSource, "cpp", "cpp");
        assert(conflictRun.success && conflictRun.ast != nullptr && !conflictRun.generatedCode.empty());
        assert(validatorRun.success && validatorRun.ast != nullptr && !validatorRun.generatedCode.empty());
        std::cout << "PASS: test 4 — pipeline runs for both self-host files\n";
        passed++;
    }

    // Test 5: Gap-analysis reporting captures opaque construct deficits.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        ExpectedStructure strictExpected;
        strictExpected.filePath = conflictHeaderPath();
        strictExpected.constructs = {
            {"struct", "CrossTypeConflict"},
            {"function", "collectCrossTypeConflicts"},
            {"class", "MissingClassForGapReport"},
            {"function", "missingHelperForGapReport"}
        };
        SelfHostHarness::checkExpected(conflict, strictExpected);
        auto gaps = collectGaps(conflict);
        assert(conflict.totalOpaque == 2);
        assert(gaps.size() >= 1);
        std::cout << "PASS: test 5 — gap-analysis report path\n";
        passed++;
    }

    // Test 6: Inferred complexity annotation emits valid Semanno.
    {
        auto validator = SelfHostHarness::parseFile(validatorHeaderPath());
        AnnotationInference inf;
        auto inferred = inf.inferAll(validator.ast.get());
        std::string complexity = "O(1)";
        for (const auto& item : inferred) {
            if (item.annotationType == "ComplexityAnnotation") {
                complexity = item.value;
                break;
            }
        }

        ComplexityAnnotation anno;
        anno.timeComplexity = complexity;
        const std::string semanno = SemannoEmitter::emit(&anno);
        assert(SemannoParser::isSemannoComment(semanno));
        auto parsed = SemannoParser::parse(semanno);
        assert(parsed.type == "complexity");
        std::cout << "PASS: test 6 — semanno output for inferred complexity\n";
        passed++;
    }

    // Test 7: Coverage reports include file identity and parse state.
    {
        auto conflict = SelfHostHarness::parseFile(conflictHeaderPath());
        SelfHostHarness::checkExpected(conflict, expectedConflict());
        auto report = SelfHostHarness::coverageReport(conflict);
        assert(report.find(conflictHeaderPath()) != std::string::npos);
        assert(report.find("Parse success: yes") != std::string::npos);
        std::cout << "PASS: test 7 — report formatting includes file + status\n";
        passed++;
    }

    // Test 8: Phase artifact sanity (steps 400-404 test files exist).
    {
        const std::vector<std::string> required = {
            "tests/step400_test.cpp",
            "tests/step401_test.cpp",
            "tests/step402_test.cpp",
            "tests/step403_test.cpp",
            "tests/step404_test.cpp"
        };
        int found = 0;
        for (const auto& rel : required) {
            if (std::filesystem::exists("editor/" + rel)) found++;
        }
        assert(found == 5);
        std::cout << "PASS: test 8 — phase 16b artifact set present\n";
        passed++;
    }

    std::cout << "\nStep 404 result: " << passed << "/8 tests passed\n";
    return (passed == 8) ? 0 : 1;
}
