// Step 449: Intent-Driven Translation Tests (12 tests)

#include "IntentTranslator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_sort_intent_python_to_rust() {
    TEST(sort_intent_python_to_rust);
    std::vector<FunctionIntent> funcs = {
        {"sort_items", "sort list ascending", "items.sort()"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "rust");
    CHECK(r.hasTranslationFor("sort_items"), "expected translation");
    auto t = r.getTranslation("sort_items");
    CHECK(t.isIdiomatic, "sort should be idiomatic");
    CHECK(t.confidence >= 0.90f, "sort should be high confidence");
    CHECK(t.targetCode.find("sort") != std::string::npos, "target should use sort");
    PASS();
}

void test_sort_intent_to_sql() {
    TEST(sort_intent_to_sql);
    auto mapping = IntentTranslator::findIdiomaticMapping("sort results", "python", "sql");
    CHECK(mapping.confidence > 0.0f, "should find SQL sort mapping");
    CHECK(mapping.targetPattern.find("ORDER BY") != std::string::npos, "SQL should use ORDER BY");
    CHECK(mapping.isIdiomatic, "should be idiomatic");
    PASS();
}

void test_find_intent_python_to_java() {
    TEST(find_intent_python_to_java);
    std::vector<FunctionIntent> funcs = {
        {"find_user", "find element by key", "users.get(key)"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "java");
    auto t = r.getTranslation("find_user");
    CHECK(t.isIdiomatic, "find should be idiomatic");
    CHECK(t.targetCode.find("stream") != std::string::npos ||
          t.targetCode.find("find") != std::string::npos, "Java should use streams or find");
    PASS();
}

void test_find_intent_to_sql() {
    TEST(find_intent_to_sql);
    auto mapping = IntentTranslator::findIdiomaticMapping("find element by key", "java", "sql");
    CHECK(mapping.targetPattern.find("WHERE") != std::string::npos, "SQL should use WHERE");
    PASS();
}

void test_filter_intent_to_rust() {
    TEST(filter_intent_to_rust);
    std::vector<FunctionIntent> funcs = {
        {"get_active", "filter active items", "[x for x in items if x.active]"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "rust");
    auto t = r.getTranslation("get_active");
    CHECK(t.isIdiomatic, "filter should be idiomatic");
    CHECK(t.targetCode.find("filter") != std::string::npos, "Rust should use .filter()");
    PASS();
}

void test_map_intent_to_python() {
    TEST(map_intent_to_python);
    auto mapping = IntentTranslator::findIdiomaticMapping("transform items", "java", "python");
    CHECK(mapping.confidence >= 0.85f, "map should be high confidence");
    CHECK(mapping.isIdiomatic, "should be idiomatic");
    PASS();
}

void test_reduce_intent_to_cpp() {
    TEST(reduce_intent_to_cpp);
    auto mapping = IntentTranslator::findIdiomaticMapping("sum all values", "python", "cpp");
    CHECK(mapping.confidence >= 0.85f, "reduce should be high confidence");
    CHECK(mapping.targetPattern.find("accumulate") != std::string::npos,
          "C++ should use std::accumulate");
    PASS();
}

void test_no_intent_structural_translation() {
    TEST(no_intent_structural_translation);
    std::vector<FunctionIntent> funcs = {
        {"mystery", "", "x = a + b * c"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "rust");
    auto t = r.getTranslation("mystery");
    CHECK(!t.isIdiomatic, "no intent should not be idiomatic");
    CHECK(t.confidence <= 0.55f, "no intent should be low confidence");
    CHECK(t.needsReview, "no intent should need review");
    PASS();
}

void test_ambiguous_intent_flags_review() {
    TEST(ambiguous_intent_flags_review);
    std::vector<FunctionIntent> funcs = {
        {"process", "", "do_complex_thing(data)"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "rust");
    auto t = r.getTranslation("process");
    CHECK(t.needsReview, "ambiguous should need review");
    CHECK(!t.reviewReason.empty(), "should have review reason");
    PASS();
}

void test_intent_with_no_mapping_uses_structural() {
    TEST(intent_with_no_mapping_uses_structural);
    std::vector<FunctionIntent> funcs = {
        {"custom", "perform quantum entanglement", "quantum_op(q1, q2)"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "rust");
    auto t = r.getTranslation("custom");
    CHECK(!t.isIdiomatic, "unknown intent should not be idiomatic");
    CHECK(t.confidence >= 0.65f, "intent present but no mapping should be moderate confidence");
    CHECK(!t.needsReview, "intent present should not auto-flag review");
    PASS();
}

void test_multiple_functions_translated() {
    TEST(multiple_functions_translated);
    std::vector<FunctionIntent> funcs = {
        {"sort_data", "sort list ascending", "data.sort()"},
        {"find_item", "find element by key", "data[key]"},
        {"no_intent", "", "x = 1"}
    };
    auto r = IntentTranslator::translate(funcs, "python", "rust");
    CHECK(r.translations.size() == 3, "expected 3 translations");
    CHECK(r.countIdiomatic() == 2, "expected 2 idiomatic translations");
    CHECK(r.countNeedsReview() == 1, "expected 1 needing review");
    PASS();
}

void test_read_file_intent() {
    TEST(read_file_intent);
    auto mapping = IntentTranslator::findIdiomaticMapping("read file contents", "python", "rust");
    CHECK(mapping.confidence >= 0.80f, "read file should be high confidence");
    CHECK(mapping.targetPattern.find("read_to_string") != std::string::npos,
          "Rust should use read_to_string");
    CHECK(mapping.isIdiomatic, "should be idiomatic");
    PASS();
}

int main() {
    std::cout << "Step 449: Intent-Driven Translation Tests\n";

    test_sort_intent_python_to_rust();           // 1
    test_sort_intent_to_sql();                   // 2
    test_find_intent_python_to_java();           // 3
    test_find_intent_to_sql();                   // 4
    test_filter_intent_to_rust();                // 5
    test_map_intent_to_python();                 // 6
    test_reduce_intent_to_cpp();                 // 7
    test_no_intent_structural_translation();     // 8
    test_ambiguous_intent_flags_review();        // 9
    test_intent_with_no_mapping_uses_structural(); // 10
    test_multiple_functions_translated();         // 11
    test_read_file_intent();                      // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
