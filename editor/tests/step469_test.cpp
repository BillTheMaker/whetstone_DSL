// Step 469: Initializer Lists + STL Patterns Tests (12 tests)

#include "ast/CppInitializerStl.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_initializer_list_simple_ints() {
    TEST(parse_initializer_list_simple_ints);
    InitializerListExpression e;
    bool ok = CppInitializerStl::parseInitializerList("{1, 2, 3}", e);
    CHECK(ok, "parse should succeed");
    CHECK(e.elements.size() == 3, "expected 3 elements");
    CHECK(e.elements[0] == "1" && e.elements[2] == "3", "wrong elements");
    PASS();
}

void test_parse_initializer_list_strings() {
    TEST(parse_initializer_list_strings);
    InitializerListExpression e;
    bool ok = CppInitializerStl::parseInitializerList("{\"a\", \"b\"}", e);
    CHECK(ok, "parse should succeed");
    CHECK(e.elements.size() == 2, "expected 2 elements");
    PASS();
}

void test_initializer_list_json_roundtrip() {
    TEST(initializer_list_json_roundtrip);
    InitializerListExpression e;
    e.elements = {"1", "2", "3"};
    auto j = CppInitializerStl::toJson(e);
    auto out = CppInitializerStl::fromJson(j);
    CHECK(out.elements.size() == 3, "roundtrip element count mismatch");
    CHECK(out.elements[1] == "2", "roundtrip element mismatch");
    PASS();
}

void test_recognize_std_vector_container() {
    TEST(recognize_std_vector_container);
    auto info = CppInitializerStl::recognizeContainer("std::vector<int> v = {1,2,3};");
    CHECK(info.containerKind == "vector", "expected vector kind");
    CHECK(info.elementType == "int", "expected int element type");
    PASS();
}

void test_recognize_std_map_container() {
    TEST(recognize_std_map_container);
    auto info = CppInitializerStl::recognizeContainer("std::map<std::string,int> m;");
    CHECK(info.containerKind == "map", "expected map kind");
    CHECK(info.elementType.find("std::string") != std::string::npos, "expected map template args");
    PASS();
}

void test_recognize_std_set_container() {
    TEST(recognize_std_set_container);
    auto info = CppInitializerStl::recognizeContainer("std::set<long> s;");
    CHECK(info.containerKind == "set", "expected set kind");
    PASS();
}

void test_recognize_std_string_container() {
    TEST(recognize_std_string_container);
    auto info = CppInitializerStl::recognizeContainer("std::string name;");
    CHECK(info.containerKind == "string", "expected string kind");
    PASS();
}

void test_unknown_container_fallback() {
    TEST(unknown_container_fallback);
    auto info = CppInitializerStl::recognizeContainer("CustomList<int> c;");
    CHECK(info.containerKind == "unknown", "expected unknown kind");
    PASS();
}

void test_container_annotations_include_type_info() {
    TEST(container_annotations_include_type_info);
    auto info = CppInitializerStl::recognizeContainer("std::vector<int> v;");
    bool hasContainer = false, hasElement = false;
    for (const auto& a : info.annotations) {
        if (a.find("@Container(") != std::string::npos) hasContainer = true;
        if (a.find("@ElementType(") != std::string::npos) hasElement = true;
    }
    CHECK(hasContainer, "missing container annotation");
    CHECK(hasElement, "missing element type annotation");
    PASS();
}

void test_detect_iterator_pattern_begin_end_loop() {
    TEST(detect_iterator_pattern_begin_end_loop);
    std::string src = "for (std::vector<int>::iterator it = v.begin(); it != v.end(); ++it) { use(*it); }";
    auto info = CppInitializerStl::detectIteratorPattern(src);
    CHECK(info.isIteratorLoop, "expected iterator loop detection");
    CHECK(!info.annotations.empty(), "expected loop annotation");
    PASS();
}

void test_detect_iterator_pattern_auto_it_loop() {
    TEST(detect_iterator_pattern_auto_it_loop);
    std::string src = "for (auto it = values.begin(); it != values.end(); it++) { sum += *it; }";
    auto info = CppInitializerStl::detectIteratorPattern(src);
    CHECK(info.isIteratorLoop, "expected iterator loop detection");
    PASS();
}

void test_non_iterator_loop_not_annotated() {
    TEST(non_iterator_loop_not_annotated);
    std::string src = "for (int i = 0; i < n; ++i) { sum += i; }";
    auto info = CppInitializerStl::detectIteratorPattern(src);
    CHECK(!info.isIteratorLoop, "should not detect iterator loop");
    CHECK(info.annotations.empty(), "should not add loop annotations");
    PASS();
}

int main() {
    std::cout << "Step 469: Initializer Lists + STL Patterns Tests\n";

    test_parse_initializer_list_simple_ints();          // 1
    test_parse_initializer_list_strings();              // 2
    test_initializer_list_json_roundtrip();             // 3
    test_recognize_std_vector_container();              // 4
    test_recognize_std_map_container();                 // 5
    test_recognize_std_set_container();                 // 6
    test_recognize_std_string_container();              // 7
    test_unknown_container_fallback();                  // 8
    test_container_annotations_include_type_info();     // 9
    test_detect_iterator_pattern_begin_end_loop();      // 10
    test_detect_iterator_pattern_auto_it_loop();        // 11
    test_non_iterator_loop_not_annotated();             // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
