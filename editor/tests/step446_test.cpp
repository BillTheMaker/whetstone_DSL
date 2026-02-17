// Step 446: Test Generation for Migration Validation Tests (12 tests)

#include "MigrationTestGenerator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static MigrationUnit makeUnit(const std::string& mod,
                               const std::vector<std::string>& exports,
                               const std::string& target = "rust") {
    MigrationUnit u;
    u.id = "unit-0";
    u.filePath = mod + ".c";
    u.moduleName = mod;
    u.sourceLanguage = "c";
    u.targetLanguage = target;
    u.exports = exports;
    return u;
}

static const std::string UTILS_SRC =
    "int clamp(int x, int lo, int hi){ return x<lo?lo:x>hi?hi:x; }\n"
    "int max(int a, int b){ return a>b?a:b; }\n";

static const std::string PARSER_SRC =
    "typedef struct { int type; char* text; } Token;\n"
    "Token* parse(const char* input){ return 0; }\n"
    "void free_tokens(Token* t){ }\n";

void test_equivalence_tests_generated() {
    TEST(equivalence_tests_generated);
    auto unit = makeUnit("utils", {"clamp", "max"});
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    int eq = suite.countByKind(MigrationTestKind::Equivalence);
    CHECK(eq >= 2, "expected equivalence test for each public function");
    PASS();
}

void test_edge_case_tests_from_contracts() {
    TEST(edge_case_tests_from_contracts);
    auto unit = makeUnit("parser", {"parse", "free_tokens"});
    auto api = APIBoundaryPreserver::analyze(PARSER_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    int edge = suite.countByKind(MigrationTestKind::EdgeCase);
    CHECK(edge >= 1, "expected at least 1 edge case test from contracts");
    PASS();
}

void test_performance_tests_generated() {
    TEST(performance_tests_generated);
    auto unit = makeUnit("utils", {"clamp", "max"});
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    int perf = suite.countByKind(MigrationTestKind::Performance);
    CHECK(perf >= 2, "expected performance test for each function");
    PASS();
}

void test_tests_generated_for_each_function() {
    TEST(tests_generated_for_each_function);
    auto unit = makeUnit("utils", {"clamp", "max"});
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    CHECK(suite.hasTestFor("clamp"), "expected tests for clamp");
    CHECK(suite.hasTestFor("max"), "expected tests for max");
    PASS();
}

void test_test_language_matches_target_rust() {
    TEST(test_language_matches_target_rust);
    auto unit = makeUnit("utils", {"clamp"}, "rust");
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    CHECK(suite.targetLanguage == "rust", "suite should target rust");
    for (const auto& t : suite.tests)
        CHECK(t.language == "rust", "all tests should be in rust");
    PASS();
}

void test_test_language_matches_target_java() {
    TEST(test_language_matches_target_java);
    auto unit = makeUnit("utils", {"clamp"}, "java");
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    CHECK(suite.targetLanguage == "java", "suite should target java");
    for (const auto& t : suite.tests)
        CHECK(t.language == "java", "all tests should be in java");
    PASS();
}

void test_rust_test_body_format() {
    TEST(rust_test_body_format);
    auto unit = makeUnit("utils", {"clamp"}, "rust");
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    auto tests = suite.testsForFunction("clamp");
    bool foundRustTest = false;
    for (const auto& t : tests) {
        if (t.kind == MigrationTestKind::Equivalence) {
            CHECK(t.testBody.find("#[test]") != std::string::npos,
                  "Rust test should have #[test] attribute");
            CHECK(t.testBody.find("fn test_clamp") != std::string::npos,
                  "Rust test should have test function name");
            foundRustTest = true;
        }
    }
    CHECK(foundRustTest, "expected Rust equivalence test");
    PASS();
}

void test_java_test_body_format() {
    TEST(java_test_body_format);
    auto unit = makeUnit("utils", {"clamp"}, "java");
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    auto tests = suite.testsForFunction("clamp");
    bool foundJavaTest = false;
    for (const auto& t : tests) {
        if (t.kind == MigrationTestKind::Equivalence) {
            CHECK(t.testBody.find("@Test") != std::string::npos,
                  "Java test should have @Test annotation");
            foundJavaTest = true;
        }
    }
    CHECK(foundJavaTest, "expected Java equivalence test");
    PASS();
}

void test_null_edge_case_for_pointer_params() {
    TEST(null_edge_case_for_pointer_params);
    auto unit = makeUnit("parser", {"free_tokens"}, "rust");
    auto api = APIBoundaryPreserver::analyze(PARSER_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    auto tests = suite.testsForFunction("free_tokens");
    bool foundNullTest = false;
    for (const auto& t : tests) {
        if (t.kind == MigrationTestKind::EdgeCase &&
            t.title.find("null") != std::string::npos) {
            foundNullTest = true;
        }
    }
    CHECK(foundNullTest, "expected null edge case test for pointer param");
    PASS();
}

void test_skeleton_work_items_routed() {
    TEST(skeleton_work_items_routed);
    auto unit = makeUnit("utils", {"clamp"});
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    bool hasSLM = false, hasLLM = false;
    for (const auto& t : suite.tests) {
        if (t.routing == TestRouting::SLM) hasSLM = true;
        if (t.routing == TestRouting::LLM) hasLLM = true;
    }
    CHECK(hasSLM, "expected SLM-routed tests (equivalence)");
    CHECK(hasLLM, "expected LLM-routed tests (edge/perf)");
    PASS();
}

void test_unique_test_ids() {
    TEST(unique_test_ids);
    auto unit = makeUnit("parser", {"parse", "free_tokens"});
    auto api = APIBoundaryPreserver::analyze(PARSER_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    for (size_t i = 0; i < suite.tests.size(); ++i)
        for (size_t j = i + 1; j < suite.tests.size(); ++j)
            CHECK(suite.tests[i].id != suite.tests[j].id, "test IDs must be unique");
    PASS();
}

void test_python_target_language() {
    TEST(python_target_language);
    auto unit = makeUnit("utils", {"clamp"}, "python");
    auto api = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto suite = MigrationTestGenerator::generate(api);
    auto tests = suite.testsForFunction("clamp");
    bool foundPython = false;
    for (const auto& t : tests) {
        if (t.kind == MigrationTestKind::Equivalence) {
            CHECK(t.testBody.find("def test_") != std::string::npos,
                  "Python test should use def test_ format");
            foundPython = true;
        }
    }
    CHECK(foundPython, "expected Python test");
    PASS();
}

int main() {
    std::cout << "Step 446: Test Generation for Migration Validation Tests\n";

    test_equivalence_tests_generated();          // 1
    test_edge_case_tests_from_contracts();        // 2
    test_performance_tests_generated();           // 3
    test_tests_generated_for_each_function();     // 4
    test_test_language_matches_target_rust();     // 5
    test_test_language_matches_target_java();     // 6
    test_rust_test_body_format();                 // 7
    test_java_test_body_format();                 // 8
    test_null_edge_case_for_pointer_params();     // 9
    test_skeleton_work_items_routed();            // 10
    test_unique_test_ids();                       // 11
    test_python_target_language();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
