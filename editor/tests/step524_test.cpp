// Step 524: Typed Taskitem Contract Schema (12 tests)

#include "TypedTaskitemContractSchema.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static json validContractJson() {
    return {
        {"id", "ti-1"},
        {"nodeId", "fn-1"},
        {"language", "cpp"},
        {"allowedTargets", json::array({"fn-1", "var-2"})},
        {"allowedOps", json::array({"update", "rename"})},
        {"allowedSymbols", json::array({"foo", "bar"})},
        {"forbiddenSymbols", json::array({"baz"})},
        {"expectedDiagnosticsAdd", json::array({"E0201"})},
        {"expectedDiagnosticsRemove", json::array()}
    };
}

void test_valid_contract_passes() {
    TEST(valid_contract_passes);
    auto r = TypedTaskitemContractSchema::parseAndValidate(validContractJson());
    CHECK(r.valid, "valid contract should pass");
    PASS();
}

void test_missing_id_rejected() {
    TEST(missing_id_rejected);
    auto j = validContractJson();
    j.erase("id");
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "missing id should fail");
    PASS();
}

void test_missing_allowed_targets_rejected() {
    TEST(missing_allowed_targets_rejected);
    auto j = validContractJson();
    j.erase("allowedTargets");
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "missing allowedTargets should fail");
    PASS();
}

void test_empty_allowed_ops_rejected() {
    TEST(empty_allowed_ops_rejected);
    auto j = validContractJson();
    j["allowedOps"] = json::array();
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "empty allowedOps should fail");
    PASS();
}

void test_duplicate_allowed_ops_rejected() {
    TEST(duplicate_allowed_ops_rejected);
    auto j = validContractJson();
    j["allowedOps"] = json::array({"update", "update"});
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "duplicate allowedOps should fail");
    PASS();
}

void test_symbol_conflict_rejected() {
    TEST(symbol_conflict_rejected);
    auto j = validContractJson();
    j["forbiddenSymbols"] = json::array({"foo"});
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "symbol overlap should fail");
    PASS();
}

void test_missing_diagnostics_delta_rejected() {
    TEST(missing_diagnostics_delta_rejected);
    auto j = validContractJson();
    j["expectedDiagnosticsAdd"] = json::array();
    j["expectedDiagnosticsRemove"] = json::array();
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "missing diagnostics delta should fail");
    PASS();
}

void test_invalid_array_item_rejected() {
    TEST(invalid_array_item_rejected);
    auto j = validContractJson();
    j["allowedSymbols"] = json::array({"foo", 1});
    auto r = TypedTaskitemContractSchema::parseAndValidate(j);
    CHECK(!r.valid, "non-string array item should fail");
    PASS();
}

void test_op_allowed_true_for_listed_op() {
    TEST(op_allowed_true_for_listed_op);
    auto r = TypedTaskitemContractSchema::parseAndValidate(validContractJson());
    CHECK(r.valid && TypedTaskitemContractSchema::opAllowed(r.contract, "rename"),
          "rename should be allowed");
    PASS();
}

void test_op_allowed_false_for_unlisted_op() {
    TEST(op_allowed_false_for_unlisted_op);
    auto r = TypedTaskitemContractSchema::parseAndValidate(validContractJson());
    CHECK(r.valid && !TypedTaskitemContractSchema::opAllowed(r.contract, "delete"),
          "delete should not be allowed");
    PASS();
}

void test_symbol_allowed_false_for_forbidden() {
    TEST(symbol_allowed_false_for_forbidden);
    auto r = TypedTaskitemContractSchema::parseAndValidate(validContractJson());
    CHECK(r.valid && !TypedTaskitemContractSchema::symbolAllowed(r.contract, "baz"),
          "forbidden symbol should be blocked");
    PASS();
}

void test_symbol_allowed_false_for_unknown() {
    TEST(symbol_allowed_false_for_unknown);
    auto r = TypedTaskitemContractSchema::parseAndValidate(validContractJson());
    CHECK(r.valid && !TypedTaskitemContractSchema::symbolAllowed(r.contract, "unknown"),
          "unknown symbol should be blocked");
    PASS();
}

int main() {
    std::cout << "Step 524: Typed Taskitem Contract Schema\n";

    test_valid_contract_passes();                   // 1
    test_missing_id_rejected();                     // 2
    test_missing_allowed_targets_rejected();        // 3
    test_empty_allowed_ops_rejected();              // 4
    test_duplicate_allowed_ops_rejected();          // 5
    test_symbol_conflict_rejected();                // 6
    test_missing_diagnostics_delta_rejected();      // 7
    test_invalid_array_item_rejected();             // 8
    test_op_allowed_true_for_listed_op();           // 9
    test_op_allowed_false_for_unlisted_op();        // 10
    test_symbol_allowed_false_for_forbidden();      // 11
    test_symbol_allowed_false_for_unknown();        // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
