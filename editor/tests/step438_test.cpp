// Step 438: Code Age + Idiom Detection Tests (12 tests)

#include "LegacyIdiomDetector.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_detect_knr_function_declaration() {
    TEST(detect_knr_function_declaration);
    const std::string src = "int add(a,b)\nint a;\nint b;\n{\n  return a+b;\n}\n";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "legacy.c");
    CHECK(report.hasPattern("knr-declaration"), "expected K&R declaration pattern");
    PASS();
}

void test_detect_goto_heavy_flow() {
    TEST(detect_goto_heavy_flow);
    const std::string src = "int f(){\nstart: if(1) goto end; goto start; end: return 0;\n}\n";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "flow.c");
    CHECK(report.hasPattern("goto-control-flow"), "expected goto pattern");
    PASS();
}

void test_detect_deprecated_gets() {
    TEST(detect_deprecated_gets);
    const std::string src = "int main(){ char b[32]; gets(b); }";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "deprecated.c");
    CHECK(report.hasPattern("deprecated-api-gets"), "expected gets pattern");
    PASS();
}

void test_detect_deprecated_sprintf() {
    TEST(detect_deprecated_sprintf);
    const std::string src = "void f(){ char b[32]; sprintf(b, \"%d\", 7); }";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "deprecated2.c");
    CHECK(report.hasPattern("deprecated-api-sprintf"), "expected sprintf pattern");
    PASS();
}

void test_detect_deprecated_strcpy() {
    TEST(detect_deprecated_strcpy);
    const std::string src = "void f(char* d,const char* s){ strcpy(d,s); }";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "deprecated3.c");
    CHECK(report.hasPattern("deprecated-api-strcpy"), "expected strcpy pattern");
    PASS();
}

void test_detect_pointer_arithmetic() {
    TEST(detect_pointer_arithmetic);
    const std::string src = "void f(int* p){ *(p+1)=3; p = p + 2; }";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "ptr.c");
    CHECK(report.hasPattern("pointer-arithmetic"), "expected pointer arithmetic pattern");
    PASS();
}

void test_detect_manual_memory_without_raii() {
    TEST(detect_manual_memory_without_raii);
    const std::string src = "void f(){ int* p=(int*)malloc(sizeof(int)); free(p); }";
    auto report = LegacyIdiomDetector::analyzeFile(src, "cpp", "mem.cpp");
    CHECK(report.hasPattern("manual-memory-management"), "expected manual memory pattern");
    PASS();
}

void test_language_version_detect_c89_from_knr() {
    TEST(language_version_detect_c89_from_knr);
    const std::string src = "int f(a) int a; { return a; }";
    std::string v = LegacyIdiomDetector::detectLanguageVersion(src, "c");
    CHECK(v == "c89", "expected c89");
    PASS();
}

void test_language_version_detect_c99_from_declaration_style() {
    TEST(language_version_detect_c99_from_declaration_style);
    const std::string src = "int f(){ int x=0; for (int i=0;i<3;i++) x+=i; return x; }";
    std::string v = LegacyIdiomDetector::detectLanguageVersion(src, "c");
    CHECK(v == "c99", "expected c99");
    PASS();
}

void test_language_version_detect_cpp11() {
    TEST(language_version_detect_cpp11);
    const std::string src = "auto x = 1; std::unique_ptr<int> p(new int(3));";
    std::string v = LegacyIdiomDetector::detectLanguageVersion(src, "cpp");
    CHECK(v == "cpp11", "expected cpp11");
    PASS();
}

void test_legacy_score_clamped_to_10() {
    TEST(legacy_score_clamped_to_10);
    const std::string src = "gets(a); sprintf(b,\"x\"); strcpy(c,d); goto x; x: ; malloc(1);";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "score.c");
    CHECK(report.legacyScore >= 0 && report.legacyScore <= 10, "score out of range");
    PASS();
}

void test_per_function_findings_include_function_name() {
    TEST(per_function_findings_include_function_name);
    const std::string src = "int risky(){ gets(buf); return 0; }";
    auto report = LegacyIdiomDetector::analyzeFile(src, "c", "fn.c");
    CHECK(!report.functionFindings.empty(), "expected function findings");
    CHECK(report.functionFindings[0].functionName == "risky", "expected function name risky");
    PASS();
}

int main() {
    std::cout << "Step 438: Code Age + Idiom Detection Tests\n";

    test_detect_knr_function_declaration();                 // 1
    test_detect_goto_heavy_flow();                          // 2
    test_detect_deprecated_gets();                          // 3
    test_detect_deprecated_sprintf();                       // 4
    test_detect_deprecated_strcpy();                        // 5
    test_detect_pointer_arithmetic();                       // 6
    test_detect_manual_memory_without_raii();               // 7
    test_language_version_detect_c89_from_knr();            // 8
    test_language_version_detect_c99_from_declaration_style(); // 9
    test_language_version_detect_cpp11();                   // 10
    test_legacy_score_clamped_to_10();                      // 11
    test_per_function_findings_include_function_name();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
