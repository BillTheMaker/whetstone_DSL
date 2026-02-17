// Step 541: Rust/Go Constructive Edit Adapter (12 tests)

#include "RustGoConstructiveEditAdapter.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

static bool hasDiag(const RustGoConstructiveEditResult& r, const std::string& d) {
    for (const auto& x : r.diagnostics) if (x == d) return true;
    return false;
}

void test_rust_function_rename_allowed() {
    TEST(rust_function_rename_allowed);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "function", "rename", false, false});
    CHECK(r.supported && r.allowed, "rust function rename should be allowed");
    PASS();
}

void test_rust_borrow_region_extract_blocked_on_risk() {
    TEST(rust_borrow_region_extract_blocked_on_risk);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "borrow_region", "extract", true, false});
    CHECK(!r.allowed, "rust borrow extract should be blocked on risk");
    CHECK(hasDiag(r, "rust_borrow_lifetime_extract_blocked"), "borrow/lifetime guard expected");
    PASS();
}

void test_rust_struct_rename_blocked_on_lifetime_risk() {
    TEST(rust_struct_rename_blocked_on_lifetime_risk);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "struct", "rename", true, false});
    CHECK(!r.allowed, "rust struct rename should be blocked on risk");
    CHECK(hasDiag(r, "rust_lifetime_sensitive_rename_blocked"), "lifetime guard expected");
    PASS();
}

void test_rust_impl_extract_allowed_without_risk() {
    TEST(rust_impl_extract_allowed_without_risk);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "impl", "extract", false, false});
    CHECK(r.allowed, "rust impl extract should be allowed without risk");
    PASS();
}

void test_go_channel_extract_blocked_on_concurrency_risk() {
    TEST(go_channel_extract_blocked_on_concurrency_risk);
    auto r = RustGoConstructiveEditAdapter::evaluate({"go", "channel", "extract", false, true});
    CHECK(!r.allowed, "go channel extract should be blocked on concurrency risk");
    CHECK(hasDiag(r, "go_concurrency_extract_blocked"), "go concurrency guard expected");
    PASS();
}

void test_go_function_update_blocked_on_concurrency_risk() {
    TEST(go_function_update_blocked_on_concurrency_risk);
    auto r = RustGoConstructiveEditAdapter::evaluate({"go", "function", "update", false, true});
    CHECK(!r.allowed, "go function update should be blocked on risk");
    CHECK(hasDiag(r, "go_concurrency_update_blocked"), "go update guard expected");
    PASS();
}

void test_go_function_update_allowed_without_risk() {
    TEST(go_function_update_allowed_without_risk);
    auto r = RustGoConstructiveEditAdapter::evaluate({"go", "function", "update", false, false});
    CHECK(r.allowed, "go function update should be allowed without risk");
    PASS();
}

void test_go_import_reorder_allowed() {
    TEST(go_import_reorder_allowed);
    auto r = RustGoConstructiveEditAdapter::evaluate({"go", "import", "reorder", false, false});
    CHECK(r.allowed, "go import reorder should be allowed");
    PASS();
}

void test_illegal_operation_for_construct_rejected() {
    TEST(illegal_operation_for_construct_rejected);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "struct", "extract", false, false});
    CHECK(!r.allowed, "illegal op should be rejected");
    CHECK(hasDiag(r, "operation_not_legal_for_construct"), "illegal op diag expected");
    PASS();
}

void test_unsupported_construct_rejected() {
    TEST(unsupported_construct_rejected);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "trait_object_magic", "update", false, false});
    CHECK(!r.supported, "unsupported construct should fail");
    CHECK(hasDiag(r, "unsupported_construct"), "unsupported construct diag expected");
    PASS();
}

void test_unsupported_language_rejected() {
    TEST(unsupported_language_rejected);
    auto r = RustGoConstructiveEditAdapter::evaluate({"python", "function", "rename", false, false});
    CHECK(!r.supported, "unsupported language should fail");
    CHECK(hasDiag(r, "unsupported_language"), "unsupported language diag expected");
    PASS();
}

void test_rust_import_insert_allowed() {
    TEST(rust_import_insert_allowed);
    auto r = RustGoConstructiveEditAdapter::evaluate({"rust", "import", "insert", false, false});
    CHECK(r.allowed, "rust import insert should be allowed");
    PASS();
}

int main() {
    std::cout << "Step 541: Rust/Go Constructive Edit Adapter\n";

    test_rust_function_rename_allowed();                     // 1
    test_rust_borrow_region_extract_blocked_on_risk();      // 2
    test_rust_struct_rename_blocked_on_lifetime_risk();     // 3
    test_rust_impl_extract_allowed_without_risk();          // 4
    test_go_channel_extract_blocked_on_concurrency_risk();  // 5
    test_go_function_update_blocked_on_concurrency_risk();  // 6
    test_go_function_update_allowed_without_risk();         // 7
    test_go_import_reorder_allowed();                       // 8
    test_illegal_operation_for_construct_rejected();        // 9
    test_unsupported_construct_rejected();                  // 10
    test_unsupported_language_rejected();                   // 11
    test_rust_import_insert_allowed();                      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
