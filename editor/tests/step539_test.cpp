// Step 539: C/C++ Constructive Edit Adapter (12 tests)

#include "CppConstructiveEditAdapter.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasDiag(const CppConstructiveEditResult& r, const std::string& d) {
    for (const auto& x : r.diagnostics) if (x == d) return true;
    return false;
}

void test_cpp_declaration_rename_allowed() {
    TEST(cpp_declaration_rename_allowed);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "declaration", "header", "rename"});
    CHECK(r.supported && r.allowed, "declaration rename should be allowed");
    PASS();
}

void test_cpp_definition_extract_allowed_in_source() {
    TEST(cpp_definition_extract_allowed_in_source);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "definition", "source", "extract"});
    CHECK(r.allowed, "source definition extract should be allowed");
    PASS();
}

void test_include_insert_allowed() {
    TEST(include_insert_allowed);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "include", "header", "insert"});
    CHECK(r.allowed, "include insert should be allowed");
    PASS();
}

void test_include_reorder_blocked_in_source() {
    TEST(include_reorder_blocked_in_source);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "include", "source", "reorder"});
    CHECK(!r.allowed, "source include reorder should be blocked");
    CHECK(hasDiag(r, "source_include_reorder_blocked"), "boundary diagnostic expected");
    PASS();
}

void test_macro_update_allowed() {
    TEST(macro_update_allowed);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "macro", "header", "update"});
    CHECK(r.allowed, "macro update should be allowed");
    PASS();
}

void test_macro_delete_blocked_in_source() {
    TEST(macro_delete_blocked_in_source);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "macro", "source", "delete"});
    CHECK(!r.allowed, "macro delete in source should be blocked");
    CHECK(hasDiag(r, "macro_delete_requires_header_review"), "macro boundary diagnostic expected");
    PASS();
}

void test_member_extract_allowed_in_header() {
    TEST(member_extract_allowed_in_header);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "member", "header", "extract"});
    CHECK(r.allowed, "header member extract should be allowed");
    PASS();
}

void test_member_extract_blocked_in_source() {
    TEST(member_extract_blocked_in_source);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "member", "source", "extract"});
    CHECK(!r.allowed, "source member extract should be illegal");
    CHECK(hasDiag(r, "operation_not_legal_for_construct"), "illegal-op diagnostic expected");
    PASS();
}

void test_definition_inline_blocked_in_header() {
    TEST(definition_inline_blocked_in_header);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "definition", "header", "inline"});
    CHECK(!r.allowed, "header definition inline should be blocked");
    CHECK(hasDiag(r, "header_source_boundary_violation"), "boundary diagnostic expected");
    PASS();
}

void test_unsupported_construct_rejected() {
    TEST(unsupported_construct_rejected);
    auto r = CppConstructiveEditAdapter::evaluate({"cpp", "template_magic", "header", "rename"});
    CHECK(!r.supported, "unsupported construct should not be supported");
    CHECK(hasDiag(r, "unsupported_construct"), "unsupported construct diagnostic expected");
    PASS();
}

void test_unsupported_language_rejected() {
    TEST(unsupported_language_rejected);
    auto r = CppConstructiveEditAdapter::evaluate({"typescript", "definition", "source", "update"});
    CHECK(!r.supported, "unsupported language should not be supported");
    CHECK(hasDiag(r, "unsupported_language"), "unsupported language diagnostic expected");
    PASS();
}

void test_c_language_paths_supported() {
    TEST(c_language_paths_supported);
    auto r = CppConstructiveEditAdapter::evaluate({"c", "definition", "source", "update"});
    CHECK(r.supported && r.allowed, "c definition update should be supported");
    PASS();
}

int main() {
    std::cout << "Step 539: C/C++ Constructive Edit Adapter\n";

    test_cpp_declaration_rename_allowed();          // 1
    test_cpp_definition_extract_allowed_in_source();// 2
    test_include_insert_allowed();                  // 3
    test_include_reorder_blocked_in_source();       // 4
    test_macro_update_allowed();                    // 5
    test_macro_delete_blocked_in_source();          // 6
    test_member_extract_allowed_in_header();        // 7
    test_member_extract_blocked_in_source();        // 8
    test_definition_inline_blocked_in_header();     // 9
    test_unsupported_construct_rejected();          // 10
    test_unsupported_language_rejected();           // 11
    test_c_language_paths_supported();              // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
