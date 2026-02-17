// Step 468: Operator Overloading + Friend Tests (12 tests)

#include "ast/CppOperators.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_member_operator_less_const() {
    TEST(parse_member_operator_less_const);
    OperatorOverload o;
    bool ok = CppOperators::parseOperatorOverload("bool operator<(const Foo&) const;", o);
    CHECK(ok, "parse should succeed");
    CHECK(o.operatorSymbol == "<", "wrong operator symbol");
    CHECK(o.returnType == "bool", "wrong return type");
    CHECK(o.isConst, "expected const qualifier");
    CHECK(!o.isFriend, "member overload should not be friend");
    PASS();
}

void test_parse_friend_operator_stream_insertion() {
    TEST(parse_friend_operator_stream_insertion);
    OperatorOverload o;
    bool ok = CppOperators::parseOperatorOverload(
        "friend ostream& operator<<(ostream&, const Foo&);", o);
    CHECK(ok, "parse should succeed");
    CHECK(o.operatorSymbol == "<<", "wrong operator symbol");
    CHECK(o.isFriend, "expected friend");
    CHECK(o.parameters.size() == 2, "expected two parameters");
    PASS();
}

void test_parse_operator_equals() {
    TEST(parse_operator_equals);
    OperatorOverload o;
    bool ok = CppOperators::parseOperatorOverload(
        "bool operator==(const Foo& rhs) const;", o);
    CHECK(ok, "parse should succeed");
    CHECK(o.operatorSymbol == "==", "wrong operator symbol");
    CHECK(o.isConst, "expected const");
    PASS();
}

void test_parse_operator_plus_non_const() {
    TEST(parse_operator_plus_non_const);
    OperatorOverload o;
    bool ok = CppOperators::parseOperatorOverload(
        "Foo operator+(const Foo& rhs);", o);
    CHECK(ok, "parse should succeed");
    CHECK(o.operatorSymbol == "+", "wrong operator symbol");
    CHECK(!o.isConst, "unexpected const");
    PASS();
}

void test_parse_invalid_declaration_fails() {
    TEST(parse_invalid_declaration_fails);
    OperatorOverload o;
    bool ok = CppOperators::parseOperatorOverload("int notAnOperator(int x);", o);
    CHECK(!ok, "non-operator declaration should fail");
    PASS();
}

void test_operator_json_roundtrip() {
    TEST(operator_json_roundtrip);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("bool operator<(const Foo&) const;", o);
    auto j = CppOperators::toJson(o);
    auto out = CppOperators::fromJson(j);
    CHECK(out.operatorSymbol == o.operatorSymbol, "operator symbol mismatch");
    CHECK(out.returnType == o.returnType, "return type mismatch");
    CHECK(out.isConst == o.isConst, "const flag mismatch");
    PASS();
}

void test_generate_cpp_declaration_member() {
    TEST(generate_cpp_declaration_member);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("bool operator<(const Foo&) const;", o);
    auto s = CppOperators::generateCppDeclaration(o);
    CHECK(s == "bool operator<(const Foo&) const;", "wrong generated declaration");
    PASS();
}

void test_generate_cpp_declaration_friend() {
    TEST(generate_cpp_declaration_friend);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("friend ostream& operator<<(ostream&, const Foo&);", o);
    auto s = CppOperators::generateCppDeclaration(o);
    CHECK(s.find("friend ostream& operator<<") == 0, "missing friend declaration start");
    PASS();
}

void test_project_less_to_java_compare_to() {
    TEST(project_less_to_java_compare_to);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("bool operator<(const Foo&) const;", o);
    auto java = CppOperators::projectToJavaMethod(o);
    CHECK(java.find("compareTo") != std::string::npos, "expected compareTo projection");
    PASS();
}

void test_project_less_to_python_dunder_lt() {
    TEST(project_less_to_python_dunder_lt);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("bool operator<(const Foo&) const;", o);
    auto py = CppOperators::projectToPythonMethod(o);
    CHECK(py.find("__lt__") != std::string::npos, "expected __lt__ projection");
    PASS();
}

void test_project_equals_to_python_dunder_eq() {
    TEST(project_equals_to_python_dunder_eq);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("bool operator==(const Foo&) const;", o);
    auto py = CppOperators::projectToPythonMethod(o);
    CHECK(py.find("__eq__") != std::string::npos, "expected __eq__ projection");
    PASS();
}

void test_project_stream_operator_to_java_display_method() {
    TEST(project_stream_operator_to_java_display_method);
    OperatorOverload o;
    CppOperators::parseOperatorOverload("friend ostream& operator<<(ostream&, const Foo&);", o);
    auto java = CppOperators::projectToJavaMethod(o);
    CHECK(java.find("toDisplayString") != std::string::npos, "expected display projection");
    PASS();
}

int main() {
    std::cout << "Step 468: Operator Overloading + Friend Tests\n";

    test_parse_member_operator_less_const();            // 1
    test_parse_friend_operator_stream_insertion();      // 2
    test_parse_operator_equals();                       // 3
    test_parse_operator_plus_non_const();               // 4
    test_parse_invalid_declaration_fails();             // 5
    test_operator_json_roundtrip();                     // 6
    test_generate_cpp_declaration_member();             // 7
    test_generate_cpp_declaration_friend();             // 8
    test_project_less_to_java_compare_to();             // 9
    test_project_less_to_python_dunder_lt();            // 10
    test_project_equals_to_python_dunder_eq();          // 11
    test_project_stream_operator_to_java_display_method(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
