// Step 586: Capability Discovery Panels (12 tests)

#include "CapabilityDiscoveryPanels.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_register_category_success() {
    TEST(register_category_success);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "register should succeed");
    CHECK(p.operationCountFor("mcp") == 0, "initial count should be zero");
    PASS();
}

void test_register_category_rejects_missing_id() {
    TEST(register_category_rejects_missing_id);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(!p.registerCategory("", "MCP Tools", &error), "register should fail");
    CHECK(error == "category_id_missing", "wrong error");
    PASS();
}

void test_register_category_rejects_duplicate() {
    TEST(register_category_rejects_duplicate);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "first register failed");
    CHECK(!p.registerCategory("mcp", "Dup", &error), "duplicate should fail");
    CHECK(error == "category_duplicate", "wrong error");
    PASS();
}

void test_record_operation_increments_count() {
    TEST(record_operation_increments_count);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "register failed");
    CHECK(p.recordOperation("mcp", &error), "record failed");
    CHECK(p.recordOperation("mcp", &error), "record failed");
    CHECK(p.operationCountFor("mcp") == 2, "count mismatch");
    PASS();
}

void test_record_operation_fails_for_unknown_category() {
    TEST(record_operation_fails_for_unknown_category);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(!p.recordOperation("missing", &error), "record should fail");
    CHECK(error == "category_missing", "wrong error");
    PASS();
}

void test_add_recommendation_success() {
    TEST(add_recommendation_success);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "register failed");
    CHECK(p.addRecommendation("mcp", "Try tree-sitter panel", &error), "recommendation add failed");
    CHECK(p.recommendationsFor("mcp").size() == 1, "recommendation count mismatch");
    PASS();
}

void test_add_recommendation_rejects_duplicate() {
    TEST(add_recommendation_rejects_duplicate);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "register failed");
    CHECK(p.addRecommendation("mcp", "Try tree-sitter panel", &error), "add failed");
    CHECK(!p.addRecommendation("mcp", "Try tree-sitter panel", &error), "duplicate should fail");
    CHECK(error == "recommendation_duplicate", "wrong error");
    PASS();
}

void test_add_recommendation_rejects_missing_text() {
    TEST(add_recommendation_rejects_missing_text);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "register failed");
    CHECK(!p.addRecommendation("mcp", "", &error), "add should fail");
    CHECK(error == "recommendation_missing", "wrong error");
    PASS();
}

void test_sorted_by_operation_count_desc() {
    TEST(sorted_by_operation_count_desc);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("mcp", "MCP Tools", &error), "register mcp failed");
    CHECK(p.registerCategory("verify", "Verification", &error), "register verify failed");
    CHECK(p.recordOperation("verify", &error), "record verify failed");
    CHECK(p.recordOperation("verify", &error), "record verify failed");
    CHECK(p.recordOperation("mcp", &error), "record mcp failed");
    const auto sorted = p.sortedByOperationCountDesc();
    CHECK(sorted[0].categoryId == "verify", "verify should rank first");
    CHECK(sorted[1].categoryId == "mcp", "mcp should rank second");
    PASS();
}

void test_sorted_tie_breaks_by_category_id() {
    TEST(sorted_tie_breaks_by_category_id);
    CapabilityDiscoveryPanels p;
    std::string error;
    CHECK(p.registerCategory("a", "A", &error), "register a failed");
    CHECK(p.registerCategory("b", "B", &error), "register b failed");
    const auto sorted = p.sortedByOperationCountDesc();
    CHECK(sorted[0].categoryId == "a", "a should come first on tie");
    PASS();
}

void test_recommendations_for_unknown_category_empty() {
    TEST(recommendations_for_unknown_category_empty);
    CapabilityDiscoveryPanels p;
    CHECK(p.recommendationsFor("missing").empty(), "unknown category should return empty recommendations");
    PASS();
}

void test_operation_count_for_unknown_category_zero() {
    TEST(operation_count_for_unknown_category_zero);
    CapabilityDiscoveryPanels p;
    CHECK(p.operationCountFor("missing") == 0, "unknown category count should be zero");
    PASS();
}

int main() {
    std::cout << "Step 586: Capability Discovery Panels\n";

    test_register_category_success();                 // 1
    test_register_category_rejects_missing_id();      // 2
    test_register_category_rejects_duplicate();       // 3
    test_record_operation_increments_count();         // 4
    test_record_operation_fails_for_unknown_category();// 5
    test_add_recommendation_success();                // 6
    test_add_recommendation_rejects_duplicate();      // 7
    test_add_recommendation_rejects_missing_text();   // 8
    test_sorted_by_operation_count_desc();            // 9
    test_sorted_tie_breaks_by_category_id();          // 10
    test_recommendations_for_unknown_category_empty();// 11
    test_operation_count_for_unknown_category_zero(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed == 0 ? 0 : 1;
}
