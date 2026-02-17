// Step 525: Legal Operation Graph (12 tests)

#include "LegalOperationGraph.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_cpp_function_has_expected_ops() {
    TEST(cpp_function_has_expected_ops);
    LegalOperationGraph g;
    auto q = g.allowedOps("cpp", "Function");
    CHECK(q.supported, "cpp function should be supported");
    CHECK(g.opAllowed("cpp", "Function", "rename"), "rename should be allowed");
    CHECK(g.opAllowed("cpp", "Function", "inline"), "inline should be allowed");
    PASS();
}

void test_cpp_variable_disallows_extract() {
    TEST(cpp_variable_disallows_extract);
    LegalOperationGraph g;
    CHECK(!g.opAllowed("cpp", "Variable", "extract"), "extract should be disallowed");
    PASS();
}

void test_python_class_supports_rename_update_extract() {
    TEST(python_class_supports_rename_update_extract);
    LegalOperationGraph g;
    CHECK(g.opAllowed("python", "Class", "rename"), "rename missing");
    CHECK(g.opAllowed("python", "Class", "update"), "update missing");
    CHECK(g.opAllowed("python", "Class", "extract"), "extract missing");
    PASS();
}

void test_typescript_import_supports_reorder() {
    TEST(typescript_import_supports_reorder);
    LegalOperationGraph g;
    CHECK(g.opAllowed("typescript", "Import", "reorder"), "reorder should be allowed");
    PASS();
}

void test_rust_struct_disallows_delete() {
    TEST(rust_struct_disallows_delete);
    LegalOperationGraph g;
    CHECK(!g.opAllowed("rust", "Struct", "delete"), "delete should be disallowed");
    PASS();
}

void test_unknown_language_is_unsupported() {
    TEST(unknown_language_is_unsupported);
    LegalOperationGraph g;
    auto q = g.allowedOps("haskell", "Function");
    CHECK(!q.supported, "unknown language should be unsupported");
    PASS();
}

void test_unknown_node_kind_is_unsupported() {
    TEST(unknown_node_kind_is_unsupported);
    LegalOperationGraph g;
    auto q = g.allowedOps("cpp", "LambdaMagic");
    CHECK(!q.supported, "unknown node kind should be unsupported");
    PASS();
}

void test_register_rule_adds_new_language_mapping() {
    TEST(register_rule_adds_new_language_mapping);
    LegalOperationGraph g;
    g.registerRule("java", "Method", {"rename", "update"});
    CHECK(g.opAllowed("java", "Method", "rename"), "registered op missing");
    PASS();
}

void test_register_rule_overrides_existing_mapping() {
    TEST(register_rule_overrides_existing_mapping);
    LegalOperationGraph g;
    CHECK(g.opAllowed("cpp", "Function", "inline"), "baseline inline missing");
    g.registerRule("cpp", "Function", {"rename"});
    CHECK(g.opAllowed("cpp", "Function", "rename"), "rename should remain");
    CHECK(!g.opAllowed("cpp", "Function", "inline"), "inline should be removed by override");
    PASS();
}

void test_rule_deduplicates_ops() {
    TEST(rule_deduplicates_ops);
    LegalOperationGraph g;
    g.registerRule("go", "Function", {"rename", "rename", "update"});
    auto q = g.allowedOps("go", "Function");
    CHECK(q.supported, "go function should be supported");
    CHECK((int)q.operations.size() == 2, "ops should be deduplicated");
    PASS();
}

void test_c_function_supports_extract() {
    TEST(c_function_supports_extract);
    LegalOperationGraph g;
    CHECK(g.opAllowed("c", "Function", "extract"), "extract should be allowed for c function");
    PASS();
}

void test_go_import_supports_insert_delete_reorder() {
    TEST(go_import_supports_insert_delete_reorder);
    LegalOperationGraph g;
    CHECK(g.opAllowed("go", "Import", "insert"), "insert missing");
    CHECK(g.opAllowed("go", "Import", "delete"), "delete missing");
    CHECK(g.opAllowed("go", "Import", "reorder"), "reorder missing");
    PASS();
}

int main() {
    std::cout << "Step 525: Legal Operation Graph\n";

    test_cpp_function_has_expected_ops();                    // 1
    test_cpp_variable_disallows_extract();                   // 2
    test_python_class_supports_rename_update_extract();     // 3
    test_typescript_import_supports_reorder();              // 4
    test_rust_struct_disallows_delete();                    // 5
    test_unknown_language_is_unsupported();                 // 6
    test_unknown_node_kind_is_unsupported();                // 7
    test_register_rule_adds_new_language_mapping();         // 8
    test_register_rule_overrides_existing_mapping();        // 9
    test_rule_deduplicates_ops();                           // 10
    test_c_function_supports_extract();                     // 11
    test_go_import_supports_insert_delete_reorder();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
