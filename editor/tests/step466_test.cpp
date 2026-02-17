// Step 466: Range-Based For + Structured Bindings Tests (12 tests)

#include "ast/CppRangeStructured.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_range_for_const_auto_ref() {
    TEST(parse_range_for_const_auto_ref);
    RangeForStatement r;
    bool ok = CppRangeStructured::parseRangeFor(
        "for (const auto& x : items) { total += x; }", r);
    CHECK(ok, "parse should succeed");
    CHECK(r.iteratorDecl == "const auto& x", "wrong iterator decl");
    CHECK(r.iteratorName == "x", "wrong iterator name");
    CHECK(r.containerExpr == "items", "wrong container expr");
    PASS();
}

void test_parse_range_for_simple_auto() {
    TEST(parse_range_for_simple_auto);
    RangeForStatement r;
    bool ok = CppRangeStructured::parseRangeFor(
        "for (auto value : vec) { use(value); }", r);
    CHECK(ok, "parse should succeed");
    CHECK(r.iteratorName == "value", "wrong iterator name");
    PASS();
}

void test_parse_structured_binding_pair() {
    TEST(parse_structured_binding_pair);
    StructuredBinding b;
    bool ok = CppRangeStructured::parseStructuredBinding("auto [key, value] = pair;", b);
    CHECK(ok, "parse should succeed");
    CHECK(b.names.size() == 2, "expected two names");
    CHECK(b.names[0] == "key" && b.names[1] == "value", "wrong binding names");
    CHECK(b.initializerExpr == "pair", "wrong initializer");
    PASS();
}

void test_parse_structured_binding_three_names() {
    TEST(parse_structured_binding_three_names);
    StructuredBinding b;
    bool ok = CppRangeStructured::parseStructuredBinding("auto [a, b, c] = tupleVal;", b);
    CHECK(ok, "parse should succeed");
    CHECK(b.names.size() == 3, "expected three names");
    PASS();
}

void test_range_for_json_roundtrip() {
    TEST(range_for_json_roundtrip);
    RangeForStatement r;
    r.iteratorDecl = "const auto& item";
    r.iteratorName = "item";
    r.containerExpr = "items";
    r.bodyCode = "sum += item;";
    auto j = CppRangeStructured::toJson(r);
    auto out = CppRangeStructured::rangeForFromJson(j);
    CHECK(out.iteratorDecl == r.iteratorDecl, "iteratorDecl mismatch");
    CHECK(out.containerExpr == r.containerExpr, "containerExpr mismatch");
    PASS();
}

void test_structured_binding_json_roundtrip() {
    TEST(structured_binding_json_roundtrip);
    StructuredBinding b;
    b.names = {"k", "v"};
    b.initializerExpr = "entry";
    auto j = CppRangeStructured::toJson(b);
    auto out = CppRangeStructured::structuredBindingFromJson(j);
    CHECK(out.names.size() == 2, "name count mismatch");
    CHECK(out.initializerExpr == "entry", "initializer mismatch");
    PASS();
}

void test_generate_cpp_range_for() {
    TEST(generate_cpp_range_for);
    RangeForStatement r;
    r.iteratorDecl = "const auto& x";
    r.containerExpr = "items";
    r.bodyCode = "use(x);";
    auto out = CppRangeStructured::generateCpp(r);
    CHECK(out.find("for (const auto& x : items)") != std::string::npos, "missing for header");
    CHECK(out.find("use(x);") != std::string::npos, "missing body code");
    PASS();
}

void test_generate_cpp_structured_binding() {
    TEST(generate_cpp_structured_binding);
    StructuredBinding b;
    b.names = {"key", "value"};
    b.initializerExpr = "pair";
    auto out = CppRangeStructured::generateCpp(b);
    CHECK(out == "auto [key, value] = pair;", "wrong generated binding");
    PASS();
}

void test_project_range_for_to_python() {
    TEST(project_range_for_to_python);
    RangeForStatement r;
    r.iteratorName = "x";
    r.containerExpr = "items";
    auto out = CppRangeStructured::projectRangeForToPython(r);
    CHECK(out.find("for x in items:") != std::string::npos, "missing python projection");
    PASS();
}

void test_project_range_for_to_rust() {
    TEST(project_range_for_to_rust);
    RangeForStatement r;
    r.iteratorName = "x";
    r.containerExpr = "items";
    auto out = CppRangeStructured::projectRangeForToRust(r);
    CHECK(out.find("for x in items") != std::string::npos, "missing rust projection");
    PASS();
}

void test_project_range_for_to_java() {
    TEST(project_range_for_to_java);
    RangeForStatement r;
    r.iteratorName = "x";
    r.containerExpr = "items";
    auto out = CppRangeStructured::projectRangeForToJava(r);
    CHECK(out.find("for (var x : items)") != std::string::npos, "missing java projection");
    PASS();
}

void test_parse_invalid_inputs_fail_gracefully() {
    TEST(parse_invalid_inputs_fail_gracefully);
    RangeForStatement r;
    StructuredBinding b;
    bool ok1 = CppRangeStructured::parseRangeFor("while (x < y) {}", r);
    bool ok2 = CppRangeStructured::parseStructuredBinding("auto key = pair.first;", b);
    CHECK(!ok1, "range-for parser should fail on non-range-for");
    CHECK(!ok2, "structured-binding parser should fail on non-binding");
    PASS();
}

int main() {
    std::cout << "Step 466: Range-Based For + Structured Bindings Tests\n";

    test_parse_range_for_const_auto_ref();      // 1
    test_parse_range_for_simple_auto();         // 2
    test_parse_structured_binding_pair();       // 3
    test_parse_structured_binding_three_names();// 4
    test_range_for_json_roundtrip();            // 5
    test_structured_binding_json_roundtrip();   // 6
    test_generate_cpp_range_for();              // 7
    test_generate_cpp_structured_binding();     // 8
    test_project_range_for_to_python();         // 9
    test_project_range_for_to_rust();           // 10
    test_project_range_for_to_java();           // 11
    test_parse_invalid_inputs_fail_gracefully();// 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
