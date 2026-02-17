// Step 470: Phase 22b Integration + Sprint 22 Summary Tests (8 tests)

#include "ast/CppExceptions.h"
#include "ast/CppInitializerStl.h"
#include "ast/CppOperators.h"
#include "ast/CppRangeStructured.h"

#include <fstream>
#include <iostream>
#include <sstream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) return "";
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void test_self_host_file_contains_inheritance_and_virtual_features() {
    TEST(self_host_file_contains_inheritance_and_virtual_features);
    auto content = readFile("editor/src/TransformEngineExtended.h");
    CHECK(!content.empty(), "failed to read TransformEngineExtended.h");
    bool hasInheritance = content.find(": public") != std::string::npos;
    bool hasProtectedSection = content.find("protected:") != std::string::npos;
    CHECK(hasInheritance, "expected inheritance usage");
    CHECK(hasProtectedSection, "expected protected section usage");
    PASS();
}

void test_phase22b_combined_parse_range_structured_operator_initializer() {
    TEST(phase22b_combined_parse_range_structured_operator_initializer);
    RangeForStatement rf;
    StructuredBinding sb;
    OperatorOverload op;
    InitializerListExpression il;
    CHECK(CppRangeStructured::parseRangeFor("for (const auto& x : data) { use(x); }", rf),
          "range-for parse failed");
    CHECK(CppRangeStructured::parseStructuredBinding("auto [k, v] = pair;", sb),
          "structured binding parse failed");
    CHECK(CppOperators::parseOperatorOverload("bool operator<(const Foo&) const;", op),
          "operator parse failed");
    CHECK(CppInitializerStl::parseInitializerList("{1, 2, 3}", il),
          "initializer parse failed");
    CHECK(rf.iteratorName == "x", "wrong range iterator name");
    CHECK(sb.names.size() == 2, "wrong structured binding names");
    CHECK(op.operatorSymbol == "<", "wrong operator symbol");
    CHECK(il.elements.size() == 3, "wrong initializer size");
    PASS();
}

void test_exception_and_operator_projections_are_available() {
    TEST(exception_and_operator_projections_are_available);
    TryCatchStatement tc;
    CHECK(CppExceptions::parseTryCatch(
              "try { risky(); } catch (const std::exception& e) { log(e); }", tc),
          "try/catch parse failed");
    OperatorOverload op;
    CHECK(CppOperators::parseOperatorOverload("bool operator==(const Foo&) const;", op),
          "operator parse failed");

    auto rust = CppExceptions::projectTryCatchToRustResult(tc);
    auto pyEq = CppOperators::projectToPythonMethod(op);
    CHECK(rust.find("match") != std::string::npos, "missing rust projection");
    CHECK(pyEq.find("__eq__") != std::string::npos, "missing python eq projection");
    PASS();
}

void test_stl_iterator_detection_and_annotations() {
    TEST(stl_iterator_detection_and_annotations);
    auto stl = CppInitializerStl::recognizeContainer("std::vector<int> v = {1,2,3};");
    auto loop = CppInitializerStl::detectIteratorPattern(
        "for (auto it = v.begin(); it != v.end(); ++it) { use(*it); }");
    CHECK(stl.containerKind == "vector", "expected vector container");
    CHECK(loop.isIteratorLoop, "expected iterator loop");
    CHECK(!loop.annotations.empty(), "expected loop annotation");
    PASS();
}

void test_json_roundtrip_across_new_nodes() {
    TEST(json_roundtrip_across_new_nodes);
    RangeForStatement rf;
    CppRangeStructured::parseRangeFor("for (auto x : xs) { use(x); }", rf);
    auto rf2 = CppRangeStructured::rangeForFromJson(CppRangeStructured::toJson(rf));
    CHECK(rf2.iteratorName == "x", "range-for roundtrip failed");

    OperatorOverload op;
    CppOperators::parseOperatorOverload("friend ostream& operator<<(ostream&, const Foo&);", op);
    auto op2 = CppOperators::fromJson(CppOperators::toJson(op));
    CHECK(op2.operatorSymbol == "<<", "operator roundtrip failed");

    TryCatchStatement tc;
    CppExceptions::parseTryCatch("try { a(); } catch (...) { b(); }", tc);
    auto tc2 = CppExceptions::tryCatchFromJson(CppExceptions::toJson(tc));
    CHECK(tc2.catches.size() == 1, "try/catch roundtrip failed");
    PASS();
}

void test_feature_request_docs_present_for_coverage_tracking() {
    TEST(feature_request_docs_present_for_coverage_tracking);
    auto a = readFile("FEATURE_REQUESTS.md");
    auto b = readFile("feature-requests.md");
    CHECK(!a.empty() || !b.empty(), "expected at least one feature requests doc");
    PASS();
}

void test_phase22b_no_regression_on_invalid_parses() {
    TEST(phase22b_no_regression_on_invalid_parses);
    RangeForStatement rf;
    StructuredBinding sb;
    OperatorOverload op;
    TryCatchStatement tc;
    CHECK(!CppRangeStructured::parseRangeFor("if (x) {}", rf), "range parser should reject invalid");
    CHECK(!CppRangeStructured::parseStructuredBinding("auto x = pair;", sb),
          "structured binding parser should reject invalid");
    CHECK(!CppOperators::parseOperatorOverload("int normalMethod(int x);", op),
          "operator parser should reject invalid");
    CHECK(!CppExceptions::parseTryCatch("while(true){doWork();}", tc),
          "exception parser should reject invalid");
    PASS();
}

void test_sprint22b_combined_output_shapes() {
    TEST(sprint22b_combined_output_shapes);
    RangeForStatement rf;
    CppRangeStructured::parseRangeFor("for (const auto& x : items) { use(x); }", rf);
    auto py = CppRangeStructured::projectRangeForToPython(rf);
    auto rust = CppRangeStructured::projectRangeForToRust(rf);
    auto java = CppRangeStructured::projectRangeForToJava(rf);
    CHECK(py.find("for x in items:") != std::string::npos, "python projection missing");
    CHECK(rust.find("for x in items") != std::string::npos, "rust projection missing");
    CHECK(java.find("for (var x : items)") != std::string::npos, "java projection missing");
    PASS();
}

int main() {
    std::cout << "Step 470: Phase 22b Integration Tests\n";

    test_self_host_file_contains_inheritance_and_virtual_features();   // 1
    test_phase22b_combined_parse_range_structured_operator_initializer(); // 2
    test_exception_and_operator_projections_are_available();           // 3
    test_stl_iterator_detection_and_annotations();                     // 4
    test_json_roundtrip_across_new_nodes();                            // 5
    test_feature_request_docs_present_for_coverage_tracking();         // 6
    test_phase22b_no_regression_on_invalid_parses();                   // 7
    test_sprint22b_combined_output_shapes();                           // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
