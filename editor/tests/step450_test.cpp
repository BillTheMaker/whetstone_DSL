// Step 450: Algorithm-Level Equivalence Tests (12 tests)

#include "AlgorithmEquivalence.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_detect_bubble_sort() {
    TEST(detect_bubble_sort);
    std::string src = "for(i=0;i<n;i++) for(j=0;j<n-1;j++) if(a[j]>a[j+1]) swap(a[j],a[j+1]);";
    auto r = AlgorithmRecognizer::analyze(src, "c", "rust");
    CHECK(r.hasPattern("bubble_sort"), "expected bubble_sort pattern");
    PASS();
}

void test_sort_maps_to_stdlib() {
    TEST(sort_maps_to_stdlib);
    std::string src = "for(i=0;i<n;i++) for(j=0;j<n-1;j++) if(a[j]>a[j+1]) swap(a[j],a[j+1]);";
    auto r = AlgorithmRecognizer::analyze(src, "c", "rust");
    auto eq = r.equivalentFor("bubble_sort");
    CHECK(eq.usesStdLib, "sort should map to stdlib");
    CHECK(eq.targetCode.find("sort") != std::string::npos, "should use .sort()");
    PASS();
}

void test_detect_linear_search() {
    TEST(detect_linear_search);
    std::string src = "for(int i=0;i<n;i++) if(items[i]==key) return i; return -1;";
    auto r = AlgorithmRecognizer::analyze(src, "c", "rust");
    CHECK(r.hasPattern("linear_search"), "expected linear_search");
    PASS();
}

void test_detect_binary_search() {
    TEST(detect_binary_search);
    std::string src = "int low=0,high=n; while(low<high){ int mid=(low+high)/2; if(a[mid]<key) low=mid+1; else high=mid; }";
    auto r = AlgorithmRecognizer::analyze(src, "c", "rust");
    CHECK(r.hasPattern("binary_search"), "expected binary_search");
    auto eq = r.equivalentFor("binary_search");
    CHECK(eq.targetCode.find("binary_search") != std::string::npos, "Rust should use binary_search");
    PASS();
}

void test_detect_key_lookup() {
    TEST(detect_key_lookup);
    std::string src = "value = data.get(key)";
    auto r = AlgorithmRecognizer::analyze(src, "python", "rust");
    CHECK(r.hasPattern("key_lookup"), "expected key_lookup");
    auto eq = r.equivalentFor("key_lookup");
    CHECK(eq.targetCode.find("get") != std::string::npos, "Rust should use map.get");
    PASS();
}

void test_detect_manual_map() {
    TEST(detect_manual_map);
    std::string src = "result = []\nfor x in items:\n    result.append(f(x))";
    auto r = AlgorithmRecognizer::analyze(src, "python", "rust");
    CHECK(r.hasPattern("manual_map"), "expected manual_map");
    auto eq = r.equivalentFor("manual_map");
    CHECK(eq.targetCode.find("map") != std::string::npos, "Rust should use .map()");
    PASS();
}

void test_detect_manual_filter() {
    TEST(detect_manual_filter);
    std::string src = "result = []\nfor x in items:\n    if pred(x):\n        result.append(x)";
    auto r = AlgorithmRecognizer::analyze(src, "python", "rust");
    CHECK(r.hasPattern("manual_filter"), "expected manual_filter");
    auto eq = r.equivalentFor("manual_filter");
    CHECK(eq.targetCode.find("filter") != std::string::npos, "Rust should use .filter()");
    PASS();
}

void test_detect_manual_reduce() {
    TEST(detect_manual_reduce);
    std::string src = "total = 0\nfor x in items:\n    total += x";
    auto r = AlgorithmRecognizer::analyze(src, "python", "rust");
    CHECK(r.hasPattern("manual_reduce"), "expected manual_reduce");
    auto eq = r.equivalentFor("manual_reduce");
    CHECK(eq.targetCode.find("fold") != std::string::npos, "Rust should use .fold()");
    PASS();
}

void test_detect_builder_pattern() {
    TEST(detect_builder_pattern);
    std::string src = "obj = Builder().set_name('x').set_value(1).build()";
    auto r = AlgorithmRecognizer::analyze(src, "python", "rust");
    CHECK(r.hasPattern("builder_pattern"), "expected builder_pattern");
    PASS();
}

void test_stdlib_count() {
    TEST(stdlib_count);
    std::string src = "for x in items: total += x\nresult = [f(x) for x in items if pred(x)]";
    // append not present but push is checked — use a source with append
    std::string src2 = "total = 0\nfor x in items:\n    total += x\nresult=[]\nfor x in items:\n    if pred(x): result.append(x)";
    auto r = AlgorithmRecognizer::analyze(src2, "python", "java");
    CHECK(r.countStdLib() >= 2, "expected at least 2 stdlib equivalents");
    PASS();
}

void test_supported_patterns_list() {
    TEST(supported_patterns_list);
    auto patterns = AlgorithmRecognizer::supportedPatterns();
    CHECK(patterns.size() >= 15, "expected at least 15 supported patterns");
    PASS();
}

void test_cross_language_sort_targets() {
    TEST(cross_language_sort_targets);
    std::string src = "for(i=0;i<n;i++) for(j=0;j<n-1;j++) if(a[j]>a[j+1]) swap(a[j],a[j+1]);";
    auto r1 = AlgorithmRecognizer::analyze(src, "c", "python");
    auto r2 = AlgorithmRecognizer::analyze(src, "c", "java");
    auto eq1 = r1.equivalentFor("bubble_sort");
    auto eq2 = r2.equivalentFor("bubble_sort");
    CHECK(eq1.targetCode.find("sorted") != std::string::npos, "Python should use sorted()");
    CHECK(eq2.targetCode.find("Collections.sort") != std::string::npos, "Java should use Collections.sort");
    PASS();
}

int main() {
    std::cout << "Step 450: Algorithm-Level Equivalence Tests\n";
    test_detect_bubble_sort();            // 1
    test_sort_maps_to_stdlib();           // 2
    test_detect_linear_search();          // 3
    test_detect_binary_search();          // 4
    test_detect_key_lookup();             // 5
    test_detect_manual_map();             // 6
    test_detect_manual_filter();          // 7
    test_detect_manual_reduce();          // 8
    test_detect_builder_pattern();        // 9
    test_stdlib_count();                  // 10
    test_supported_patterns_list();       // 11
    test_cross_language_sort_targets();   // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
