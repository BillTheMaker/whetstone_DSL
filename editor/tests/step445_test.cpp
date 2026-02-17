// Step 445: API Boundary Preservation Tests (12 tests)

#include "APIBoundaryPreserver.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static MigrationUnit makeUnit(const std::string& path, const std::string& mod,
                               const std::vector<std::string>& exports,
                               const std::string& target = "rust") {
    MigrationUnit u;
    u.id = "unit-0";
    u.filePath = path;
    u.moduleName = mod;
    u.sourceLanguage = "c";
    u.targetLanguage = target;
    u.exports = exports;
    return u;
}

static const std::string UTILS_SRC =
    "int clamp(int x, int lo, int hi){ return x<lo?lo:x>hi?hi:x; }\n"
    "int max(int a, int b){ return a>b?a:b; }\n";

static const std::string PARSER_SRC =
    "typedef struct { int type; char* text; } Token;\n"
    "Token* parse(const char* input){ return 0; }\n"
    "void free_tokens(Token* t){ }\n";

void test_public_api_extracted() {
    TEST(public_api_extracted);
    auto unit = makeUnit("utils.c", "utils", {"clamp", "max"});
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(report.hasFunction("clamp"), "expected clamp in public API");
    CHECK(report.hasFunction("max"), "expected max in public API");
    CHECK(report.publicAPI.size() == 2, "expected 2 public functions");
    PASS();
}

void test_function_signatures_captured() {
    TEST(function_signatures_captured);
    auto unit = makeUnit("utils.c", "utils", {"clamp"});
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(!report.publicAPI.empty(), "expected public API");
    auto& sig = report.publicAPI[0];
    CHECK(sig.name == "clamp", "expected clamp");
    CHECK(!sig.returnType.empty(), "expected return type");
    CHECK(!sig.paramTypes.empty(), "expected param types");
    PASS();
}

void test_type_mappings_c_to_rust() {
    TEST(type_mappings_c_to_rust);
    auto unit = makeUnit("utils.c", "utils", {"clamp"}, "rust");
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(report.hasTypeMapping("int"), "expected int type mapping");
    PASS();
}

void test_type_mappings_c_to_java() {
    TEST(type_mappings_c_to_java);
    auto unit = makeUnit("utils.c", "utils", {"clamp"}, "java");
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(report.hasTypeMapping("int"), "expected int type mapping for java");
    PASS();
}

void test_pointer_type_mapping() {
    TEST(pointer_type_mapping);
    auto unit = makeUnit("parser.c", "parser", {"parse"}, "rust");
    auto report = APIBoundaryPreserver::analyze(PARSER_SRC, unit);
    CHECK(report.hasTypeMapping("char*"), "expected char* type mapping");
    PASS();
}

void test_contract_for_pointer_params() {
    TEST(contract_for_pointer_params);
    auto unit = makeUnit("parser.c", "parser", {"free_tokens"}, "rust");
    auto report = APIBoundaryPreserver::analyze(PARSER_SRC, unit);
    CHECK(report.hasContract("free_tokens"), "expected contract for free_tokens");
    // Should have non-null precondition for pointer param
    for (const auto& c : report.contracts) {
        if (c.functionName == "free_tokens") {
            CHECK(c.precondition.find("NULL") != std::string::npos,
                  "expected NULL check precondition");
        }
    }
    PASS();
}

void test_contract_for_pointer_return() {
    TEST(contract_for_pointer_return);
    auto unit = makeUnit("parser.c", "parser", {"parse"}, "rust");
    auto report = APIBoundaryPreserver::analyze(PARSER_SRC, unit);
    CHECK(report.hasContract("parse"), "expected contract for parse");
    for (const auto& c : report.contracts) {
        if (c.functionName == "parse") {
            CHECK(!c.postcondition.empty(), "expected postcondition for pointer return");
        }
    }
    PASS();
}

void test_ffi_boundary_rust() {
    TEST(ffi_boundary_rust);
    auto unit = makeUnit("utils.c", "utils", {"clamp", "max"}, "rust");
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(report.hasFFI("clamp"), "expected FFI for clamp");
    CHECK(report.hasFFI("max"), "expected FFI for max");
    for (const auto& f : report.ffiBoundaries) {
        CHECK(f.annotation.find("no_mangle") != std::string::npos,
              "Rust FFI should have #[no_mangle]");
    }
    PASS();
}

void test_ffi_boundary_java() {
    TEST(ffi_boundary_java);
    auto unit = makeUnit("utils.c", "utils", {"clamp"}, "java");
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(report.hasFFI("clamp"), "expected FFI for clamp");
    for (const auto& f : report.ffiBoundaries) {
        CHECK(f.annotation.find("JNI") != std::string::npos,
              "Java FFI should mention JNI");
    }
    PASS();
}

void test_api_preserved_flag() {
    TEST(api_preserved_flag);
    auto unit = makeUnit("utils.c", "utils", {"clamp", "max"});
    auto report = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    CHECK(report.apiPreserved, "API should be preserved");
    PASS();
}

void test_verify_preservation_pass() {
    TEST(verify_preservation_pass);
    auto unit = makeUnit("utils.c", "utils", {"clamp", "max"});
    auto original = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto migrated = original;  // same API = preserved
    CHECK(APIBoundaryPreserver::verifyPreservation(original, migrated),
          "same API should verify as preserved");
    PASS();
}

void test_verify_preservation_fail() {
    TEST(verify_preservation_fail);
    auto unit = makeUnit("utils.c", "utils", {"clamp", "max"});
    auto original = APIBoundaryPreserver::analyze(UTILS_SRC, unit);
    auto migrated = original;
    migrated.publicAPI.pop_back();  // remove one function
    CHECK(!APIBoundaryPreserver::verifyPreservation(original, migrated),
          "missing function should fail verification");
    PASS();
}

int main() {
    std::cout << "Step 445: API Boundary Preservation Tests\n";

    test_public_api_extracted();             // 1
    test_function_signatures_captured();     // 2
    test_type_mappings_c_to_rust();          // 3
    test_type_mappings_c_to_java();          // 4
    test_pointer_type_mapping();             // 5
    test_contract_for_pointer_params();      // 6
    test_contract_for_pointer_return();      // 7
    test_ffi_boundary_rust();                // 8
    test_ffi_boundary_java();               // 9
    test_api_preserved_flag();               // 10
    test_verify_preservation_pass();         // 11
    test_verify_preservation_fail();         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
