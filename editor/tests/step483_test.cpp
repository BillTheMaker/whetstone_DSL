// Step 483: Convention Extractor + Validator Tests (unit, negative, regression)

#include "ProjectConventionAnalyzer.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_extract_reads_size_limits_from_architecture_doc() {
    TEST(extract_reads_size_limits_from_architecture_doc);
    auto c = ProjectConventionAnalyzer::extract(".");
    CHECK(c.sizeLimits["header"] == 600, "header limit mismatch");
    CHECK(c.sizeLimits["main.cpp"] == 1500, "main limit mismatch");
    CHECK(c.sizeLimits["function"] == 80, "function limit mismatch");
    PASS();
}

void test_extract_detects_cmake_target_pattern() {
    TEST(extract_detects_cmake_target_pattern);
    auto c = ProjectConventionAnalyzer::extract(".");
    CHECK(c.buildSystem == "cmake", "expected cmake build system");
    CHECK(c.buildTargetPattern.find("add_executable(step") != std::string::npos,
          "missing build target pattern");
    PASS();
}

void test_extract_detects_test_harness_macros() {
    TEST(extract_detects_test_harness_macros);
    auto c = ProjectConventionAnalyzer::extract(".");
    CHECK(c.testHarnessPattern.find("TEST") != std::string::npos, "missing TEST harness detection");
    PASS();
}

void test_validate_flags_header_line_limit_violation_negative() {
    TEST(validate_flags_header_line_limit_violation_negative);
    ProjectConventions c;
    c.sizeLimits["header"] = 3;
    SourceFileDraft bigHeader{
        "editor/src/TooBig.h",
        "#pragma once\nline1\nline2\nline3\nline4\n"
    };
    auto r = ProjectConventionAnalyzer::validate(c, {bigHeader});
    CHECK(r.hasViolation("HEADER_TOO_LARGE"), "expected HEADER_TOO_LARGE violation");
    PASS();
}

void test_validate_flags_missing_test_assertions_negative() {
    TEST(validate_flags_missing_test_assertions_negative);
    ProjectConventions c;
    SourceFileDraft testFile{
        "editor/tests/step999_test.cpp",
        "int main(){ return 0; }\n"
    };
    auto r = ProjectConventionAnalyzer::validate(c, {testFile});
    CHECK(r.hasViolation("TEST_ASSERTION_MISSING"), "expected missing assertion violation");
    PASS();
}

void test_validate_flags_class_naming_mismatch_negative() {
    TEST(validate_flags_class_naming_mismatch_negative);
    ProjectConventions c;
    SourceFileDraft h{
        "editor/src/bad_name.h",
        "struct bad_name { int x; };"
    };
    auto r = ProjectConventionAnalyzer::validate(c, {h});
    CHECK(r.hasViolation("CLASS_NAMING_MISMATCH"), "expected class naming violation");
    PASS();
}

void test_validate_flags_missing_cmake_step_target_negative() {
    TEST(validate_flags_missing_cmake_step_target_negative);
    ProjectConventions c;
    SourceFileDraft cm{
        "editor/CMakeLists.txt",
        "add_executable(other tests/other.cpp)\n"
    };
    auto r = ProjectConventionAnalyzer::validate(c, {cm});
    CHECK(r.hasViolation("BUILD_TARGET_MISSING"), "expected build target violation");
    PASS();
}

void test_validate_accepts_well_formed_step_files() {
    TEST(validate_accepts_well_formed_step_files);
    ProjectConventions c;
    c.sizeLimits["header"] = 600;
    SourceFileDraft header{
        "editor/src/GoodName.h",
        "struct GoodName { int x; };"
    };
    SourceFileDraft testFile{
        "editor/tests/step999_test.cpp",
        "#define CHECK(a,b) do{}while(0)\nint main(){ CHECK(true, \"ok\"); return 0; }\n"
    };
    SourceFileDraft cm{
        "editor/CMakeLists.txt",
        "add_executable(step999_test tests/step999_test.cpp)\n"
    };
    auto r = ProjectConventionAnalyzer::validate(c, {header, testFile, cm});
    CHECK(r.violations.empty(), "expected no violations");
    PASS();
}

void test_regression_step482_files_validate_without_warnings() {
    TEST(regression_step482_files_validate_without_warnings);
    auto c = ProjectConventionAnalyzer::extract(".");
    std::ifstream hIn("editor/src/StepSpecExpander.h");
    std::ifstream tIn("editor/tests/step482_test.cpp");
    std::ostringstream hs, ts;
    hs << hIn.rdbuf();
    ts << tIn.rdbuf();
    std::vector<SourceFileDraft> files{
        {"editor/src/StepSpecExpander.h", hs.str()},
        {"editor/tests/step482_test.cpp", ts.str()}
    };
    auto r = ProjectConventionAnalyzer::validate(c, files);
    CHECK(!r.hasViolation("HEADER_TOO_LARGE"), "unexpected header limit violation");
    CHECK(!r.hasViolation("TEST_ASSERTION_MISSING"), "unexpected test assertion violation");
    PASS();
}

int main() {
    std::cout << "Step 483: Convention Extractor + Validator Tests\n";

    test_extract_reads_size_limits_from_architecture_doc();          // 1
    test_extract_detects_cmake_target_pattern();                     // 2
    test_extract_detects_test_harness_macros();                      // 3
    test_validate_flags_header_line_limit_violation_negative();      // 4
    test_validate_flags_missing_test_assertions_negative();          // 5
    test_validate_flags_class_naming_mismatch_negative();            // 6
    test_validate_flags_missing_cmake_step_target_negative();        // 7
    test_validate_accepts_well_formed_step_files();                  // 8
    test_regression_step482_files_validate_without_warnings();       // 9

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
