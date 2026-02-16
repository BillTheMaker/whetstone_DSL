// Step 400: Self-Hosting Test Harness (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include "SelfHostHarness.h"

int main() {
    int passed = 0;

    // Test 1: Parse simple struct from source string
    {
        std::string src = R"(
struct Foo {
    int x;
    std::string name;
};
)";
        auto result = SelfHostHarness::parseSource(src, "test_struct");
        assert(result.parseSuccess);
        assert(result.ast != nullptr);
        std::cout << "PASS: test 1 — parse simple struct\n";
        passed++;
    }

    // Test 2: Parse class + free function from source string
    {
        std::string src = R"(
class Widget {
public:
    void draw() { }
};

void process(Widget* w) {
    w->draw();
}
)";
        auto result = SelfHostHarness::parseSource(src, "test_class_func");
        assert(result.parseSuccess);
        assert(SelfHostHarness::countClasses(result.ast.get()) >= 1);
        assert(SelfHostHarness::countFunctions(result.ast.get()) >= 1);
        std::cout << "PASS: test 2 — parse class + free function\n";
        passed++;
    }

    // Test 3: Coverage tracking — all constructs found
    {
        std::string src = R"(
struct Point { int x; int y; };
void doSomething() { }
)";
        auto result = SelfHostHarness::parseSource(src, "test_coverage");
        ExpectedStructure expected;
        expected.filePath = "test_coverage";
        expected.constructs = {
            {"struct", "Point"},
            {"function", "doSomething"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 2);
        assert(result.totalParsed == 2);
        assert(result.totalOpaque == 0);
        assert(result.overallCoverage() == 1.0);
        std::cout << "PASS: test 3 — coverage tracking, all found\n";
        passed++;
    }

    // Test 4: Coverage tracking — missing construct is opaque
    {
        std::string src = R"(
void foo() { }
)";
        auto result = SelfHostHarness::parseSource(src, "test_missing");
        ExpectedStructure expected;
        expected.filePath = "test_missing";
        expected.constructs = {
            {"function", "foo"},
            {"class", "MissingClass"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 2);
        assert(result.totalParsed == 1);
        assert(result.totalOpaque == 1);
        assert(result.overallCoverage() == 0.5);
        std::cout << "PASS: test 4 — missing construct is opaque\n";
        passed++;
    }

    // Test 5: Coverage report generation
    {
        std::string src = R"(
class Foo { };
void bar() { }
)";
        auto result = SelfHostHarness::parseSource(src, "report_test");
        ExpectedStructure expected;
        expected.filePath = "report_test";
        expected.constructs = {
            {"class", "Foo"},
            {"function", "bar"}
        };
        SelfHostHarness::checkExpected(result, expected);
        std::string report = SelfHostHarness::coverageReport(result);
        assert(!report.empty());
        assert(report.find("report_test") != std::string::npos);
        assert(report.find("100%") != std::string::npos);
        std::cout << "PASS: test 5 — coverage report generation\n";
        passed++;
    }

    // Test 6: Graceful degradation — empty source doesn't crash
    {
        auto result = SelfHostHarness::parseSource("", "empty");
        assert(!result.parseSuccess || result.ast == nullptr ||
               result.warnings.size() > 0);
        std::cout << "PASS: test 6 — graceful degradation on empty source\n";
        passed++;
    }

    // Test 7: hasParsedConstruct lookup
    {
        std::string src = R"(
class Pipeline { };
void run() { }
)";
        auto result = SelfHostHarness::parseSource(src, "lookup");
        assert(SelfHostHarness::hasParsedConstruct(result.ast.get(), "class", "Pipeline"));
        assert(SelfHostHarness::hasParsedConstruct(result.ast.get(), "function", "run"));
        assert(!SelfHostHarness::hasParsedConstruct(result.ast.get(), "class", "Missing"));
        std::cout << "PASS: test 7 — hasParsedConstruct lookup\n";
        passed++;
    }

    // Test 8: getClassNames and getFunctionNames
    {
        std::string src = R"(
class Alpha { };
class Beta { };
void one() { }
void two() { }
)";
        auto result = SelfHostHarness::parseSource(src, "names");
        auto classNames = SelfHostHarness::getClassNames(result.ast.get());
        auto funcNames = SelfHostHarness::getFunctionNames(result.ast.get());
        assert(classNames.size() >= 2);
        assert(funcNames.size() >= 2);
        std::cout << "PASS: test 8 — getClassNames and getFunctionNames\n";
        passed++;
    }

    // Test 9: Parse Whetstone-style header pattern (struct + inline function)
    {
        std::string src = R"(
#pragma once
#include <string>
#include <vector>

struct ConflictInfo {
    std::string type1;
    std::string type2;
    std::string message;
};

inline void checkConflicts(const std::vector<ConflictInfo>& items) {
    for (const auto& item : items) {
        // process
    }
}
)";
        auto result = SelfHostHarness::parseSource(src, "whetstone_pattern");
        assert(result.parseSuccess);
        // Should find at least the struct and the function
        ExpectedStructure expected;
        expected.constructs = {
            {"struct", "ConflictInfo"},
            {"function", "checkConflicts"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalParsed >= 1);  // At least struct or function found
        std::cout << "PASS: test 9 — Whetstone-style header pattern\n";
        passed++;
    }

    // Test 10: Type alias coverage tracking
    {
        std::string src = R"(
using json = nlohmann::json;
using StringVec = std::vector<std::string>;
)";
        auto result = SelfHostHarness::parseSource(src, "alias_test");
        assert(result.parseSuccess);
        int aliases = SelfHostHarness::countStatements(result.ast.get(), "TypeAlias");
        assert(aliases == 2);

        ExpectedStructure expected;
        expected.constructs = {
            {"type_alias", "json"},
            {"type_alias", "StringVec"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalParsed == 2);
        std::cout << "PASS: test 10 — type alias coverage tracking\n";
        passed++;
    }

    // Test 11: Namespace coverage tracking
    {
        std::string src = R"(
namespace detail {
    void helper() { }
}
)";
        auto result = SelfHostHarness::parseSource(src, "ns_test");
        assert(result.parseSuccess);

        ExpectedStructure expected;
        expected.constructs = {
            {"namespace", "detail"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalParsed == 1);
        std::cout << "PASS: test 11 — namespace coverage tracking\n";
        passed++;
    }

    // Test 12: Mixed coverage with partial success
    {
        std::string src = R"(
class Validator {
public:
    void validate() { }
};

void helperFunc() { }
)";
        auto result = SelfHostHarness::parseSource(src, "mixed_test");
        ExpectedStructure expected;
        expected.constructs = {
            {"class", "Validator"},
            {"function", "helperFunc"},
            {"class", "NonExistent"},
            {"function", "ghostFunc"}
        };
        SelfHostHarness::checkExpected(result, expected);
        assert(result.totalExpected == 4);
        assert(result.totalParsed == 2);
        assert(result.totalOpaque == 2);
        assert(result.overallCoverage() == 0.5);

        std::string report = SelfHostHarness::coverageReport(result);
        assert(report.find("50%") != std::string::npos);
        std::cout << "PASS: test 12 — mixed coverage with partial success\n";
        passed++;
    }

    std::cout << "\nStep 400 result: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
