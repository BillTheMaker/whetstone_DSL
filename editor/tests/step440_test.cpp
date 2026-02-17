// Step 440: Modernization Suggestions Tests (12 tests)

#include "ModernizationSuggester.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: run full pipeline on source
static ModernizationReport analyze(const std::string& src,
                                   const std::string& lang,
                                   const std::string& file,
                                   const std::string& target = "") {
    auto legacy = LegacyIdiomDetector::analyzeFile(src, lang, file);
    auto safety = SafetyAuditor::audit(src, lang, file);
    return ModernizationSuggester::suggest(legacy, safety, target);
}

void test_malloc_to_smart_pointer() {
    TEST(malloc_to_smart_pointer);
    auto r = analyze("void f(){ int* p=(int*)malloc(4); free(p); }", "cpp", "mem.cpp");
    CHECK(r.hasSuggestionFrom("malloc/free"), "expected malloc/free suggestion");
    CHECK(r.hasSuggestionTo("std::unique_ptr / std::shared_ptr"),
          "expected smart pointer target");
    PASS();
}

void test_sprintf_to_format() {
    TEST(sprintf_to_format);
    auto r = analyze("void f(){ char b[32]; sprintf(b, \"%d\", 7); }", "c", "fmt.c");
    CHECK(r.hasSuggestionFrom("sprintf"), "expected sprintf suggestion");
    CHECK(r.hasSuggestionTo("std::format / snprintf"), "expected format target");
    PASS();
}

void test_strcpy_to_string() {
    TEST(strcpy_to_string);
    auto r = analyze("void f(char* d, const char* s){ strcpy(d, s); }", "c", "cpy.c");
    CHECK(r.hasSuggestionFrom("strcpy"), "expected strcpy suggestion");
    PASS();
}

void test_gets_to_fgets() {
    TEST(gets_to_fgets);
    auto r = analyze("void f(){ char b[32]; gets(b); }", "c", "gets.c");
    CHECK(r.hasSuggestionFrom("gets"), "expected gets suggestion");
    CHECK(r.hasSuggestionTo("fgets / std::getline"), "expected fgets target");
    PASS();
}

void test_goto_to_structured() {
    TEST(goto_to_structured);
    auto r = analyze("void f(){ start: goto start; }", "c", "goto.c");
    CHECK(r.hasSuggestionFrom("goto"), "expected goto suggestion");
    CHECK(r.hasSuggestionTo("structured control flow"), "expected structured control");
    PASS();
}

void test_global_mutable_to_mutex() {
    TEST(global_mutable_to_mutex);
    const std::string src =
        "static int counter = 0;\n"
        "void worker(){ counter++; }\n"
        "void start(){ pthread_create(&t, NULL, worker, NULL); }\n";
    auto r = analyze(src, "c", "race.c");
    CHECK(r.hasSuggestionFrom("unprotected shared state"),
          "expected shared state suggestion");
    CHECK(r.hasSuggestionTo("std::mutex / std::atomic"), "expected mutex target");
    PASS();
}

void test_quick_wins_grouped() {
    TEST(quick_wins_grouped);
    auto r = analyze(
        "void f(){ char b[32]; gets(b); sprintf(b, \"x\"); strcpy(b, \"y\"); }",
        "c", "multi.c");
    auto qw = r.quickWins();
    CHECK(qw.size() >= 3, "expected at least 3 quick wins");
    for (const auto& s : qw)
        CHECK(s.risk == "low", "quick wins should be low risk");
    PASS();
}

void test_deep_refactors_grouped() {
    TEST(deep_refactors_grouped);
    const std::string src =
        "static int g = 0;\n"
        "void f(){ start: goto start; }\n"
        "void g2(){ pthread_create(&t, NULL, f, NULL); }\n";
    auto r = analyze(src, "c", "deep.c");
    auto dr = r.deepRefactors();
    CHECK(dr.size() >= 1, "expected at least 1 deep refactor");
    for (const auto& s : dr)
        CHECK(s.risk == "high", "deep refactors should be high risk");
    PASS();
}

void test_annotation_format() {
    TEST(annotation_format);
    auto r = analyze("void f(){ char b[32]; gets(b); }", "c", "ann.c");
    CHECK(!r.suggestions.empty(), "expected suggestions");
    bool foundAnnotation = false;
    for (const auto& s : r.suggestions) {
        if (s.annotation.find("@Modernize(") == 0) {
            foundAnnotation = true;
            CHECK(s.annotation.find("from=") != std::string::npos, "annotation missing from=");
            CHECK(s.annotation.find("to=") != std::string::npos, "annotation missing to=");
            CHECK(s.annotation.find("risk=") != std::string::npos, "annotation missing risk=");
        }
    }
    CHECK(foundAnnotation, "expected @Modernize annotation");
    PASS();
}

void test_cross_language_c_to_rust() {
    TEST(cross_language_c_to_rust);
    auto r = analyze("void f(){ int* p=(int*)malloc(4); free(p); }", "c", "migrate.c", "rust");
    CHECK(r.targetLanguage == "rust", "expected rust target");
    CHECK(r.hasSuggestionTo("Rust ownership + borrow checker"),
          "expected Rust ownership suggestion");
    PASS();
}

void test_cross_language_c_to_java() {
    TEST(cross_language_c_to_java);
    auto r = analyze("void f(){ int* p=(int*)malloc(4); free(p); }", "c", "migrate.c", "java");
    CHECK(r.targetLanguage == "java", "expected java target");
    bool foundGc = false;
    for (const auto& s : r.suggestions)
        if (s.toReplacement.find("garbage collection") != std::string::npos) foundGc = true;
    CHECK(foundGc, "expected GC suggestion for Java target");
    PASS();
}

void test_effort_classification_counts() {
    TEST(effort_classification_counts);
    const std::string src =
        "static int g = 0;\n"
        "void f(int* p){\n"
        "  char b[32]; gets(b); sprintf(b,\"x\"); strcpy(b,\"y\");\n"
        "  int* q=(int*)malloc(4); free(q);\n"
        "  goto end; end:;\n"
        "  unsigned int x = g + 1;\n"
        "  pthread_create(&t, NULL, f, NULL);\n"
        "}\n";
    auto r = analyze(src, "c", "effort.c");
    int qw = r.countByEffort(ModernizeEffort::QuickWin);
    int mod = r.countByEffort(ModernizeEffort::Moderate);
    int deep = r.countByEffort(ModernizeEffort::DeepRefactor);
    CHECK(qw >= 2, "expected at least 2 quick wins");
    CHECK(mod >= 1, "expected at least 1 moderate");
    CHECK(deep >= 1, "expected at least 1 deep refactor");
    CHECK(qw + mod + deep == (int)r.suggestions.size(), "effort counts should sum to total");
    PASS();
}

int main() {
    std::cout << "Step 440: Modernization Suggestions Tests\n";

    test_malloc_to_smart_pointer();          // 1
    test_sprintf_to_format();                // 2
    test_strcpy_to_string();                 // 3
    test_gets_to_fgets();                    // 4
    test_goto_to_structured();               // 5
    test_global_mutable_to_mutex();          // 6
    test_quick_wins_grouped();               // 7
    test_deep_refactors_grouped();           // 8
    test_annotation_format();                // 9
    test_cross_language_c_to_rust();         // 10
    test_cross_language_c_to_java();         // 11
    test_effort_classification_counts();     // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
