// Step 439: Safety Audit via Annotations Tests (12 tests)

#include "SafetyAuditor.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_buffer_overflow_gets() {
    TEST(buffer_overflow_gets);
    const std::string src = "void f(){ char b[32]; gets(b); }";
    auto r = SafetyAuditor::audit(src, "c", "unsafe.c");
    CHECK(r.hasCategory("buffer-overflow"), "expected buffer-overflow category");
    CHECK(r.hasCwe("CWE-120"), "expected CWE-120");
    CHECK(r.overallRisk >= 3, "expected high risk for gets()");
    PASS();
}

void test_buffer_overflow_strcpy() {
    TEST(buffer_overflow_strcpy);
    const std::string src = "void copy(char* d, const char* s){ strcpy(d, s); }";
    auto r = SafetyAuditor::audit(src, "c", "cpy.c");
    CHECK(r.hasCategory("buffer-overflow"), "expected buffer-overflow");
    CHECK(r.hasCwe("CWE-120"), "expected CWE-120");
    PASS();
}

void test_use_after_free_manual_malloc() {
    TEST(use_after_free_manual_malloc);
    const std::string src = "void f(){ int* p=(int*)malloc(sizeof(int)); free(p); }";
    auto r = SafetyAuditor::audit(src, "c", "uaf.c");
    CHECK(r.hasCategory("use-after-free"), "expected use-after-free");
    CHECK(r.hasCwe("CWE-416"), "expected CWE-416");
    PASS();
}

void test_use_after_free_dangling_deref() {
    TEST(use_after_free_dangling_deref);
    const std::string src = "void f(){ int* p=(int*)malloc(4); free(p); int x = *p; }";
    auto r = SafetyAuditor::audit(src, "c", "dangling.c");
    CHECK(r.hasCategory("use-after-free"), "expected use-after-free");
    // Should find the dangling dereference
    bool foundDangling = false;
    for (const auto& f : r.findings)
        if (f.annotation == "@Lifetime(dangling)") foundDangling = true;
    CHECK(foundDangling, "expected @Lifetime(dangling) annotation");
    PASS();
}

void test_null_deref_unchecked_pointer() {
    TEST(null_deref_unchecked_pointer);
    const std::string src = "void f(int* p){ *p = 42; p = NULL; *p = 0; }";
    auto r = SafetyAuditor::audit(src, "c", "null.c");
    CHECK(r.hasCategory("null-dereference"), "expected null-dereference");
    CHECK(r.hasCwe("CWE-476"), "expected CWE-476");
    PASS();
}

void test_null_deref_unchecked_malloc() {
    TEST(null_deref_unchecked_malloc);
    const std::string src = "void f(){ int* p = (int*)malloc(4); *p = 7; free(p); }";
    auto r = SafetyAuditor::audit(src, "c", "nullalloc.c");
    CHECK(r.hasCategory("null-dereference"), "expected null-dereference for unchecked malloc");
    bool foundUncheckedAlloc = false;
    for (const auto& f : r.findings)
        if (f.annotation == "@Nullability(unchecked-alloc)") foundUncheckedAlloc = true;
    CHECK(foundUncheckedAlloc, "expected @Nullability(unchecked-alloc)");
    PASS();
}

void test_race_condition_thread_without_sync() {
    TEST(race_condition_thread_without_sync);
    const std::string src =
        "static int counter = 0;\n"
        "void worker(){ counter++; }\n"
        "void start(){ pthread_create(&t, NULL, worker, NULL); }\n";
    auto r = SafetyAuditor::audit(src, "c", "race.c");
    CHECK(r.hasCategory("race-condition"), "expected race-condition");
    CHECK(r.hasCwe("CWE-362"), "expected CWE-362");
    PASS();
}

void test_race_condition_with_mutex_not_flagged() {
    TEST(race_condition_with_mutex_not_flagged);
    const std::string src =
        "static int counter = 0;\n"
        "pthread_mutex_t mutex;\n"
        "void worker(){ pthread_mutex_lock(&mutex); counter++; pthread_mutex_unlock(&mutex); }\n"
        "void start(){ pthread_create(&t, NULL, worker, NULL); }\n";
    auto r = SafetyAuditor::audit(src, "c", "safe_race.c");
    // With mutex present, the thread+global finding should NOT appear
    bool hasSyncNone = false;
    for (const auto& f : r.findings)
        if (f.annotation == "@Sync(none)") hasSyncNone = true;
    CHECK(!hasSyncNone, "should not flag @Sync(none) when mutex is present");
    PASS();
}

void test_integer_overflow_unchecked() {
    TEST(integer_overflow_unchecked);
    const std::string src = "int f(int a, int b){ unsigned int result = a + b; return result; }";
    auto r = SafetyAuditor::audit(src, "c", "overflow.c");
    CHECK(r.hasCategory("integer-overflow"), "expected integer-overflow");
    CHECK(r.hasCwe("CWE-190"), "expected CWE-190");
    PASS();
}

void test_integer_overflow_checked_not_flagged() {
    TEST(integer_overflow_checked_not_flagged);
    const std::string src =
        "int safe_add(int a, int b){\n"
        "  if (a > INT_MAX - b) return -1;\n"
        "  int result = a + b;\n"
        "  return result;\n"
        "}\n";
    auto r = SafetyAuditor::audit(src, "c", "safe_overflow.c");
    CHECK(!r.hasCategory("integer-overflow"),
          "should not flag integer-overflow when INT_MAX check is present");
    PASS();
}

void test_per_function_safety_findings() {
    TEST(per_function_safety_findings);
    const std::string src = "void vulnerable(){ char b[32]; gets(b); }";
    auto r = SafetyAuditor::audit(src, "c", "fn.c");
    CHECK(!r.functionFindings.empty(), "expected function findings");
    CHECK(r.functionFindings[0].functionName == "vulnerable",
          "expected function name 'vulnerable'");
    CHECK(r.functionFindings[0].maxRisk >= 3, "expected high max risk");
    PASS();
}

void test_cwe_mapping_comprehensive() {
    TEST(cwe_mapping_comprehensive);
    const std::string src =
        "static int g_count = 0;\n"
        "void f(){\n"
        "  char buf[32];\n"
        "  int* p = (int*)malloc(4);\n"
        "  gets(buf);\n"
        "  free(p);\n"
        "  *p = 1;\n"  // use-after-free + dangling
        "  unsigned int x = g_count + 1;\n"
        "  pthread_create(&t, NULL, f, NULL);\n"
        "}\n";
    auto r = SafetyAuditor::audit(src, "c", "all.c");
    CHECK(r.hasCwe("CWE-120"), "expected CWE-120 (buffer overflow)");
    CHECK(r.hasCwe("CWE-416"), "expected CWE-416 (use-after-free)");
    CHECK(r.hasCwe("CWE-190"), "expected CWE-190 (integer overflow)");
    CHECK(r.hasCwe("CWE-362"), "expected CWE-362 (race condition)");
    CHECK(r.findings.size() >= 4, "expected at least 4 distinct findings");
    PASS();
}

int main() {
    std::cout << "Step 439: Safety Audit via Annotations Tests\n";

    test_buffer_overflow_gets();                    // 1
    test_buffer_overflow_strcpy();                  // 2
    test_use_after_free_manual_malloc();             // 3
    test_use_after_free_dangling_deref();            // 4
    test_null_deref_unchecked_pointer();             // 5
    test_null_deref_unchecked_malloc();              // 6
    test_race_condition_thread_without_sync();       // 7
    test_race_condition_with_mutex_not_flagged();    // 8
    test_integer_overflow_unchecked();               // 9
    test_integer_overflow_checked_not_flagged();     // 10
    test_per_function_safety_findings();             // 11
    test_cwe_mapping_comprehensive();                // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
