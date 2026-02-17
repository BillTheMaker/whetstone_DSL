#pragma once

// Step 446: Test Generation for Migration Validation — auto-generates
// equivalence, edge case, and performance regression tests for migrated modules.

#include "APIBoundaryPreserver.h"

#include <string>
#include <vector>

enum class MigrationTestKind { Equivalence, EdgeCase, Performance };
enum class TestRouting { SLM, LLM };

struct MigrationTest {
    std::string id;
    std::string functionName;
    std::string title;
    std::string testBody;       // skeleton test code
    std::string language;       // target language
    MigrationTestKind kind = MigrationTestKind::Equivalence;
    TestRouting routing = TestRouting::SLM;
};

struct MigrationTestSuite {
    std::string moduleName;
    std::string targetLanguage;
    std::vector<MigrationTest> tests;

    int countByKind(MigrationTestKind k) const {
        int n = 0;
        for (const auto& t : tests)
            if (t.kind == k) ++n;
        return n;
    }

    bool hasTestFor(const std::string& funcName) const {
        for (const auto& t : tests)
            if (t.functionName == funcName) return true;
        return false;
    }

    std::vector<MigrationTest> testsForFunction(const std::string& funcName) const {
        std::vector<MigrationTest> out;
        for (const auto& t : tests)
            if (t.functionName == funcName) out.push_back(t);
        return out;
    }
};

class MigrationTestGenerator {
public:
    static MigrationTestSuite generate(const APIBoundaryReport& api) {
        MigrationTestSuite suite;
        suite.moduleName = api.moduleName;
        suite.targetLanguage = api.targetLanguage;

        int idx = 0;
        for (const auto& func : api.publicAPI) {
            // Equivalence test for each public function
            generateEquivalenceTest(func, api, suite, idx);
            ++idx;

            // Edge case tests from contracts
            generateEdgeCaseTests(func, api, suite, idx);

            // Performance regression test
            generatePerformanceTest(func, api, suite, idx);
            ++idx;
        }

        return suite;
    }

private:
    static void generateEquivalenceTest(const FunctionSignature& func,
                                         const APIBoundaryReport& api,
                                         MigrationTestSuite& suite, int& idx) {
        MigrationTest test;
        test.id = "test-eq-" + std::to_string(idx);
        test.functionName = func.name;
        test.title = "equivalence: " + func.name + " produces same output";
        test.language = api.targetLanguage;
        test.kind = MigrationTestKind::Equivalence;
        test.routing = TestRouting::SLM;
        test.testBody = generateEquivalenceBody(func, api.targetLanguage);
        suite.tests.push_back(std::move(test));
    }

    static void generateEdgeCaseTests(const FunctionSignature& func,
                                       const APIBoundaryReport& api,
                                       MigrationTestSuite& suite, int& idx) {
        // Generate edge case tests from contract annotations
        for (const auto& contract : api.contracts) {
            if (contract.functionName != func.name) continue;

            // Null pointer edge case
            if (contract.precondition.find("NULL") != std::string::npos) {
                MigrationTest test;
                test.id = "test-edge-" + std::to_string(idx++);
                test.functionName = func.name;
                test.title = "edge case: " + func.name + " null input handling";
                test.language = api.targetLanguage;
                test.kind = MigrationTestKind::EdgeCase;
                test.routing = TestRouting::LLM;
                test.testBody = generateNullEdgeCaseBody(func, api.targetLanguage);
                suite.tests.push_back(std::move(test));
            }

            // Return value edge case
            if (!contract.postcondition.empty()) {
                MigrationTest test;
                test.id = "test-edge-" + std::to_string(idx++);
                test.functionName = func.name;
                test.title = "edge case: " + func.name + " return value contract";
                test.language = api.targetLanguage;
                test.kind = MigrationTestKind::EdgeCase;
                test.routing = TestRouting::LLM;
                test.testBody = generateReturnEdgeCaseBody(func, contract, api.targetLanguage);
                suite.tests.push_back(std::move(test));
            }
        }
    }

    static void generatePerformanceTest(const FunctionSignature& func,
                                         const APIBoundaryReport& api,
                                         MigrationTestSuite& suite, int& idx) {
        MigrationTest test;
        test.id = "test-perf-" + std::to_string(idx);
        test.functionName = func.name;
        test.title = "performance: " + func.name + " not slower than original";
        test.language = api.targetLanguage;
        test.kind = MigrationTestKind::Performance;
        test.routing = TestRouting::LLM;
        test.testBody = generatePerformanceBody(func, api.targetLanguage);
        suite.tests.push_back(std::move(test));
    }

    // --- Test body generators by target language ---

    static std::string generateEquivalenceBody(const FunctionSignature& func,
                                                const std::string& lang) {
        if (lang == "rust") {
            return "#[test]\nfn test_" + func.name + "_equivalence() {\n"
                   "    // STUB: call " + func.name + " with known inputs\n"
                   "    // assert_eq!(result, expected_from_c_implementation);\n"
                   "}\n";
        }
        if (lang == "java") {
            return "@Test\npublic void test" + capitalize(func.name) + "Equivalence() {\n"
                   "    // STUB: call " + func.name + " with known inputs\n"
                   "    // assertEquals(expected, result);\n"
                   "}\n";
        }
        if (lang == "python") {
            return "def test_" + func.name + "_equivalence():\n"
                   "    # STUB: call " + func.name + " with known inputs\n"
                   "    # assert result == expected_from_c\n";
        }
        return "// STUB: " + func.name + " equivalence test\n";
    }

    static std::string generateNullEdgeCaseBody(const FunctionSignature& func,
                                                 const std::string& lang) {
        if (lang == "rust") {
            return "#[test]\nfn test_" + func.name + "_null_input() {\n"
                   "    // STUB: pass None/null equivalent, verify behavior\n"
                   "}\n";
        }
        if (lang == "java") {
            return "@Test(expected = NullPointerException.class)\n"
                   "public void test" + capitalize(func.name) + "NullInput() {\n"
                   "    // STUB: pass null, verify exception\n"
                   "}\n";
        }
        return "// STUB: " + func.name + " null edge case\n";
    }

    static std::string generateReturnEdgeCaseBody(const FunctionSignature& func,
                                                    const APIMigrationContract& contract,
                                                    const std::string& lang) {
        if (lang == "rust") {
            return "#[test]\nfn test_" + func.name + "_return_contract() {\n"
                   "    // Contract: " + contract.postcondition + "\n"
                   "    // STUB: verify return value matches contract\n"
                   "}\n";
        }
        return "// Contract: " + contract.postcondition + "\n"
               "// STUB: verify " + func.name + " return contract\n";
    }

    static std::string generatePerformanceBody(const FunctionSignature& func,
                                                const std::string& lang) {
        if (lang == "rust") {
            return "#[test]\nfn test_" + func.name + "_performance() {\n"
                   "    // STUB: benchmark " + func.name + " and compare with C baseline\n"
                   "    // assert!(elapsed < c_baseline * 1.1); // allow 10% regression\n"
                   "}\n";
        }
        if (lang == "java") {
            return "@Test\npublic void test" + capitalize(func.name) + "Performance() {\n"
                   "    // STUB: benchmark and compare with C baseline\n"
                   "}\n";
        }
        return "// STUB: " + func.name + " performance regression test\n";
    }

    static std::string capitalize(const std::string& s) {
        if (s.empty()) return s;
        std::string result = s;
        result[0] = (char)std::toupper((unsigned char)result[0]);
        return result;
    }
};
