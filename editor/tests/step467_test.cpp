// Step 467: Exception Handling Tests (12 tests)

#include "ast/CppExceptions.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_parse_try_with_single_typed_catch() {
    TEST(parse_try_with_single_typed_catch);
    TryCatchStatement t;
    bool ok = CppExceptions::parseTryCatch(
        "try { risky(); } catch (const std::exception& e) { log(e); }", t);
    CHECK(ok, "parse should succeed");
    CHECK(t.catches.size() == 1, "expected one catch");
    CHECK(t.catches[0].exceptionType == "const std::exception&", "wrong exception type");
    CHECK(t.catches[0].variableName == "e", "wrong catch variable");
    PASS();
}

void test_parse_try_with_multiple_catches() {
    TEST(parse_try_with_multiple_catches);
    TryCatchStatement t;
    bool ok = CppExceptions::parseTryCatch(
        "try { risky(); } catch (const std::runtime_error& e) { a(); } catch (...) { b(); }", t);
    CHECK(ok, "parse should succeed");
    CHECK(t.catches.size() == 2, "expected two catches");
    PASS();
}

void test_parse_catch_all_clause() {
    TEST(parse_catch_all_clause);
    TryCatchStatement t;
    bool ok = CppExceptions::parseTryCatch(
        "try { x(); } catch (...) { recover(); }", t);
    CHECK(ok, "parse should succeed");
    CHECK(t.catches[0].catchAll, "expected catch-all flag");
    CHECK(t.catches[0].exceptionType == "...", "wrong catch-all type marker");
    PASS();
}

void test_parse_throw_expression() {
    TEST(parse_throw_expression);
    ThrowExpression th;
    bool ok = CppExceptions::parseThrowExpression(
        "if (bad) throw std::runtime_error(\"oops\");", th);
    CHECK(ok, "throw parse should succeed");
    CHECK(th.expression.find("std::runtime_error") != std::string::npos, "wrong throw expression");
    PASS();
}

void test_detect_noexcept_true() {
    TEST(detect_noexcept_true);
    CHECK(CppExceptions::hasNoexcept("int f() noexcept"), "expected noexcept detection");
    PASS();
}

void test_detect_noexcept_false() {
    TEST(detect_noexcept_false);
    CHECK(!CppExceptions::hasNoexcept("int f()"), "expected non-noexcept");
    PASS();
}

void test_try_catch_json_roundtrip() {
    TEST(try_catch_json_roundtrip);
    TryCatchStatement t;
    CppExceptions::parseTryCatch(
        "try { risky(); } catch (const std::exception& e) { log(e); }", t);
    auto j = CppExceptions::toJson(t);
    auto out = CppExceptions::tryCatchFromJson(j);
    CHECK(out.catches.size() == 1, "roundtrip catch size mismatch");
    CHECK(out.tryBody == t.tryBody, "roundtrip try body mismatch");
    PASS();
}

void test_throw_json_roundtrip() {
    TEST(throw_json_roundtrip);
    ThrowExpression th;
    th.expression = "std::runtime_error(\"err\")";
    auto j = CppExceptions::toJson(th);
    auto out = CppExceptions::throwFromJson(j);
    CHECK(out.expression == th.expression, "throw roundtrip mismatch");
    PASS();
}

void test_project_to_rust_result_match() {
    TEST(project_to_rust_result_match);
    TryCatchStatement t;
    CppExceptions::parseTryCatch(
        "try { risky(); } catch (const std::exception& e) { log(e); }", t);
    auto rust = CppExceptions::projectTryCatchToRustResult(t);
    CHECK(rust.find("match") != std::string::npos, "expected rust match projection");
    CHECK(rust.find("Err") != std::string::npos, "expected Err branch");
    PASS();
}

void test_project_to_python_try_except() {
    TEST(project_to_python_try_except);
    TryCatchStatement t;
    CppExceptions::parseTryCatch(
        "try { risky(); } catch (const std::exception& e) { log(e); }", t);
    auto py = CppExceptions::projectTryCatchToPython(t);
    CHECK(py.find("try:") != std::string::npos, "expected try block");
    CHECK(py.find("except") != std::string::npos, "expected except block");
    PASS();
}

void test_project_to_go_err_check() {
    TEST(project_to_go_err_check);
    TryCatchStatement t;
    CppExceptions::parseTryCatch("try { x(); } catch (...) { y(); }", t);
    auto go = CppExceptions::projectTryCatchToGo(t);
    CHECK(go.find("if err != nil") != std::string::npos, "expected go error check");
    PASS();
}

void test_invalid_try_catch_parse_fails() {
    TEST(invalid_try_catch_parse_fails);
    TryCatchStatement t;
    bool ok = CppExceptions::parseTryCatch("if (x) { doWork(); }", t);
    CHECK(!ok, "parse should fail without try/catch");
    PASS();
}

int main() {
    std::cout << "Step 467: Exception Handling Tests\n";

    test_parse_try_with_single_typed_catch();   // 1
    test_parse_try_with_multiple_catches();     // 2
    test_parse_catch_all_clause();              // 3
    test_parse_throw_expression();              // 4
    test_detect_noexcept_true();                // 5
    test_detect_noexcept_false();               // 6
    test_try_catch_json_roundtrip();            // 7
    test_throw_json_roundtrip();                // 8
    test_project_to_rust_result_match();        // 9
    test_project_to_python_try_except();        // 10
    test_project_to_go_err_check();             // 11
    test_invalid_try_catch_parse_fails();       // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
