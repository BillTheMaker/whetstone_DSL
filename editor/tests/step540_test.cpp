// Step 540: Python/TypeScript Constructive Edit Adapter (12 tests)

#include "PythonTypeScriptConstructiveEditAdapter.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasDiag(const PyTsConstructiveEditResult& r, const std::string& d) {
    for (const auto& x : r.diagnostics) if (x == d) return true;
    return false;
}

void test_python_function_rename_allowed() {
    TEST(python_function_rename_allowed);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"python", "function", "rename", false});
    CHECK(r.supported && r.allowed, "python function rename should be allowed");
    PASS();
}

void test_python_class_extract_allowed() {
    TEST(python_class_extract_allowed);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"python", "class", "extract", false});
    CHECK(r.allowed, "python class extract should be allowed");
    PASS();
}

void test_typescript_import_reorder_allowed_without_side_effect_risk() {
    TEST(typescript_import_reorder_allowed_without_side_effect_risk);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"typescript", "import", "reorder", false});
    CHECK(r.allowed, "ts import reorder should be allowed when risk false");
    PASS();
}

void test_typescript_import_reorder_blocked_with_side_effect_risk() {
    TEST(typescript_import_reorder_blocked_with_side_effect_risk);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"typescript", "import", "reorder", true});
    CHECK(!r.allowed, "ts import reorder should be blocked when risk true");
    CHECK(hasDiag(r, "module_side_effect_reorder_blocked"), "side-effect diag expected");
    PASS();
}

void test_module_stmt_delete_blocked_with_side_effect_risk() {
    TEST(module_stmt_delete_blocked_with_side_effect_risk);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"python", "module_stmt", "delete", true});
    CHECK(!r.allowed, "module stmt delete should be blocked with risk");
    CHECK(hasDiag(r, "module_side_effect_delete_blocked"), "side-effect delete diag expected");
    PASS();
}

void test_function_inline_blocked_with_side_effect_risk() {
    TEST(function_inline_blocked_with_side_effect_risk);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"python", "function", "inline", true});
    CHECK(!r.allowed, "inline should be blocked with module risk");
    CHECK(hasDiag(r, "module_side_effect_inline_blocked"), "side-effect inline diag expected");
    PASS();
}

void test_signature_update_allowed() {
    TEST(signature_update_allowed);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"typescript", "signature", "update", false});
    CHECK(r.allowed, "signature update should be allowed");
    PASS();
}

void test_signature_rename_not_legal() {
    TEST(signature_rename_not_legal);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"typescript", "signature", "rename", false});
    CHECK(!r.allowed, "signature rename should be illegal");
    CHECK(hasDiag(r, "operation_not_legal_for_construct"), "illegal op diag expected");
    PASS();
}

void test_import_delete_allowed_without_side_effect_risk() {
    TEST(import_delete_allowed_without_side_effect_risk);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"python", "import", "delete", false});
    CHECK(r.allowed, "import delete should be allowed when risk false");
    PASS();
}

void test_unsupported_construct_rejected() {
    TEST(unsupported_construct_rejected);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"python", "decorator_stack", "update", false});
    CHECK(!r.supported, "unsupported construct should not be supported");
    CHECK(hasDiag(r, "unsupported_construct"), "unsupported construct diag expected");
    PASS();
}

void test_unsupported_language_rejected() {
    TEST(unsupported_language_rejected);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"cpp", "function", "rename", false});
    CHECK(!r.supported, "unsupported language should fail");
    CHECK(hasDiag(r, "unsupported_language"), "unsupported language diag expected");
    PASS();
}

void test_typescript_function_extract_allowed() {
    TEST(typescript_function_extract_allowed);
    auto r = PythonTypeScriptConstructiveEditAdapter::evaluate({"typescript", "function", "extract", false});
    CHECK(r.allowed, "ts function extract should be allowed");
    PASS();
}

int main() {
    std::cout << "Step 540: Python/TypeScript Constructive Edit Adapter\n";

    test_python_function_rename_allowed();                          // 1
    test_python_class_extract_allowed();                            // 2
    test_typescript_import_reorder_allowed_without_side_effect_risk(); // 3
    test_typescript_import_reorder_blocked_with_side_effect_risk();// 4
    test_module_stmt_delete_blocked_with_side_effect_risk();       // 5
    test_function_inline_blocked_with_side_effect_risk();          // 6
    test_signature_update_allowed();                               // 7
    test_signature_rename_not_legal();                             // 8
    test_import_delete_allowed_without_side_effect_risk();         // 9
    test_unsupported_construct_rejected();                         // 10
    test_unsupported_language_rejected();                          // 11
    test_typescript_function_extract_allowed();                    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
