// Step 536: Argument Shape Validator (12 tests)

#include "ArgumentShapeValidator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static TaskitemContract contract() {
    TaskitemContract c;
    c.id = "ti-536";
    c.nodeId = "n-1";
    c.language = "cpp";
    c.allowedTargets = {"Function"};
    c.allowedOps = {"rename", "update", "insert", "extract", "delete"};
    c.allowedSymbols = {"cursor", "count", "render"};
    c.forbiddenSymbols = {"unsafeGlobal"};
    c.expectedDiagnosticsAdd = {"none"};
    return c;
}

static bool hasError(const ArgumentShapeResult& r, const std::string& codePrefix) {
    for (const auto& e : r.errors) {
        if (e.find(codePrefix) == 0) return true;
    }
    return false;
}

void test_valid_rename_args_pass() {
    TEST(valid_rename_args_pass);
    auto r = ArgumentShapeValidator::validate(contract(), "rename", "cursor",
                                              json{{"newName", "cursor2"}});
    CHECK(r.valid, "valid rename should pass");
    PASS();
}

void test_rename_missing_new_name_fails() {
    TEST(rename_missing_new_name_fails);
    auto r = ArgumentShapeValidator::validate(contract(), "rename", "cursor", json::object());
    CHECK(hasError(r, "missing_or_invalid_string:newName"), "newName error expected");
    PASS();
}

void test_rename_noop_fails() {
    TEST(rename_noop_fails);
    auto r = ArgumentShapeValidator::validate(contract(), "rename", "cursor",
                                              json{{"newName", "cursor"}});
    CHECK(hasError(r, "rename_noop"), "rename noop should fail");
    PASS();
}

void test_update_requires_replacement() {
    TEST(update_requires_replacement);
    auto r = ArgumentShapeValidator::validate(contract(), "update", "cursor", json::object());
    CHECK(hasError(r, "missing_or_invalid_string:replacement"), "replacement error expected");
    PASS();
}

void test_insert_requires_position_and_insert_text() {
    TEST(insert_requires_position_and_insert_text);
    auto r = ArgumentShapeValidator::validate(contract(), "insert", "cursor",
                                              json{{"insertText", "x"}});
    CHECK(hasError(r, "missing_or_invalid_string:position"), "position required");
    PASS();
}

void test_insert_rejects_invalid_position_enum() {
    TEST(insert_rejects_invalid_position_enum);
    auto r = ArgumentShapeValidator::validate(contract(), "insert", "cursor",
                                              json{{"insertText", "x"}, {"position", "middle"}});
    CHECK(hasError(r, "invalid_enum:position"), "invalid position enum expected");
    PASS();
}

void test_extract_requires_target_and_mode() {
    TEST(extract_requires_target_and_mode);
    auto r = ArgumentShapeValidator::validate(contract(), "extract", "cursor", json::object());
    CHECK(hasError(r, "missing_or_invalid_string:target"), "target required");
    CHECK(hasError(r, "missing_or_invalid_string:mode"), "mode required");
    PASS();
}

void test_extract_rejects_invalid_mode() {
    TEST(extract_rejects_invalid_mode);
    auto r = ArgumentShapeValidator::validate(contract(), "extract", "cursor",
                                              json{{"target", "tmp"}, {"mode", "lambda"}});
    CHECK(hasError(r, "invalid_enum:mode"), "invalid mode expected");
    PASS();
}

void test_delete_requires_confirm_bool() {
    TEST(delete_requires_confirm_bool);
    auto r = ArgumentShapeValidator::validate(contract(), "delete", "cursor", json::object());
    CHECK(hasError(r, "missing_or_invalid_boolean:confirm"), "confirm required");
    PASS();
}

void test_disallowed_operation_fails() {
    TEST(disallowed_operation_fails);
    auto c = contract();
    c.allowedOps = {"rename"};
    auto r = ArgumentShapeValidator::validate(c, "delete", "cursor", json{{"confirm", true}});
    CHECK(hasError(r, "op_not_allowed:delete"), "disallowed op should fail");
    PASS();
}

void test_disallowed_symbol_fails() {
    TEST(disallowed_symbol_fails);
    auto r = ArgumentShapeValidator::validate(contract(), "rename", "ghost",
                                              json{{"newName", "ghost2"}});
    CHECK(hasError(r, "symbol_not_allowed:ghost"), "disallowed symbol should fail");
    PASS();
}

void test_args_must_be_object() {
    TEST(args_must_be_object);
    auto r = ArgumentShapeValidator::validate(contract(), "rename", "cursor", json::array());
    CHECK(hasError(r, "args_not_object"), "args object requirement expected");
    PASS();
}

int main() {
    std::cout << "Step 536: Argument Shape Validator\n";

    test_valid_rename_args_pass();                      // 1
    test_rename_missing_new_name_fails();               // 2
    test_rename_noop_fails();                           // 3
    test_update_requires_replacement();                 // 4
    test_insert_requires_position_and_insert_text();    // 5
    test_insert_rejects_invalid_position_enum();        // 6
    test_extract_requires_target_and_mode();            // 7
    test_extract_rejects_invalid_mode();                // 8
    test_delete_requires_confirm_bool();                // 9
    test_disallowed_operation_fails();                  // 10
    test_disallowed_symbol_fails();                     // 11
    test_args_must_be_object();                         // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
