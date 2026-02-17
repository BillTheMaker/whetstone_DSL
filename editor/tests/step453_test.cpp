// Step 453: Memory Model Translation Tests (12 tests)

#include "MemoryModelTranslator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_manual_c_to_rust_gains_safety() {
    TEST(manual_c_to_rust_gains_safety);
    auto r = MemoryModelTranslator::analyze("p = malloc(8); free(p);", "c", "rust");
    CHECK(r.hasPattern("@Owner(Manual)"), "expected manual pattern");
    CHECK(r.translations[0].safetyChange == SafetyDelta::Gained, "manual->rust should gain safety");
    CHECK(r.translations[0].annotation.find("@Risk(lowered)") != std::string::npos,
          "expected lowered risk annotation");
    PASS();
}

void test_manual_c_to_java_gains_gc_safety() {
    TEST(manual_c_to_java_gains_gc_safety);
    auto r = MemoryModelTranslator::analyze("p = malloc(8); free(p);", "c", "java");
    CHECK(r.hasPattern("@Owner(Manual)"), "expected manual pattern");
    CHECK(r.translations[0].safetyChange == SafetyDelta::Gained, "manual->java should gain safety");
    CHECK(r.translations[0].annotation.find("@Owner(GC)") != std::string::npos,
          "expected GC owner annotation");
    PASS();
}

void test_unique_cpp_to_c_loses_safety() {
    TEST(unique_cpp_to_c_loses_safety);
    auto r = MemoryModelTranslator::analyze("std::unique_ptr<Foo> p;", "cpp", "c");
    CHECK(r.hasPattern("@Owner(Unique)"), "expected unique ownership pattern");
    CHECK(r.translations[0].safetyChange == SafetyDelta::Lost, "unique->C should lose safety");
    CHECK(r.translations[0].annotation.find("@Risk(high)") != std::string::npos,
          "expected high risk annotation");
    PASS();
}

void test_shared_cpp_to_rust_arc_mapping() {
    TEST(shared_cpp_to_rust_arc_mapping);
    auto r = MemoryModelTranslator::analyze("std::shared_ptr<Foo> p;", "cpp", "rust");
    CHECK(r.hasPattern("@Owner(Shared_ARC)"), "expected shared ownership pattern");
    CHECK(r.translations[0].targetCode.find("Arc::new") != std::string::npos,
          "expected Arc mapping");
    CHECK(r.translations[0].safetyChange == SafetyDelta::Neutral, "shared->shared should be neutral");
    PASS();
}

void test_shared_rust_to_c_loses_safety() {
    TEST(shared_rust_to_c_loses_safety);
    auto r = MemoryModelTranslator::analyze("let p: Arc<Foo> = Arc::new(v);", "rust", "c");
    CHECK(r.hasPattern("@Owner(Shared_ARC)"), "expected shared ownership pattern");
    CHECK(r.translations[0].safetyChange == SafetyDelta::Lost, "shared->C should lose safety");
    CHECK(r.translations[0].annotation.find("@Risk(high)") != std::string::npos,
          "expected high risk annotation");
    PASS();
}

void test_scope_lifetime_to_go_defer() {
    TEST(scope_lifetime_to_go_defer);
    auto r = MemoryModelTranslator::analyze("with open(path) as f: data = f.read()", "python", "go");
    CHECK(r.hasPattern("@Lifetime(Scope)"), "expected scope lifetime pattern");
    CHECK(r.translations[0].annotation.find("@Lifetime(deferred)") != std::string::npos,
          "expected deferred lifetime annotation");
    PASS();
}

void test_scope_to_c_loses_scope_guarantee() {
    TEST(scope_to_c_loses_scope_guarantee);
    auto r = MemoryModelTranslator::analyze("with resource_manager() as r: use(r)", "python", "c");
    CHECK(r.hasPattern("@Lifetime(Scope)"), "expected scope pattern");
    CHECK(r.translations[0].safetyChange == SafetyDelta::Lost, "scope->C should lose guarantee");
    CHECK(r.translations[0].annotation.find("@Risk(medium)") != std::string::npos,
          "expected medium risk annotation");
    PASS();
}

void test_detect_source_model_rust_borrowchecked() {
    TEST(detect_source_model_rust_borrowchecked);
    auto r = MemoryModelTranslator::analyze("let x = Box::new(1);", "rust", "cpp");
    CHECK(r.sourceModel == OwnershipModel::BorrowChecked, "rust source should be borrow-checked");
    PASS();
}

void test_detect_target_model_java_gc() {
    TEST(detect_target_model_java_gc);
    auto r = MemoryModelTranslator::analyze("int* p = malloc(8);", "c", "java");
    CHECK(r.targetModel == OwnershipModel::GC, "java target should use GC model");
    PASS();
}

void test_no_pattern_on_plain_code() {
    TEST(no_pattern_on_plain_code);
    auto r = MemoryModelTranslator::analyze("x = a + b;", "python", "rust");
    CHECK(r.translations.empty(), "plain code should not emit memory translations");
    PASS();
}

void test_safety_gain_count_for_manual_migration() {
    TEST(safety_gain_count_for_manual_migration);
    auto r = MemoryModelTranslator::analyze("p = malloc(8); free(p);", "c", "rust");
    CHECK(r.countSafetyGained() >= 1, "expected at least one safety gain");
    CHECK(r.countSafetyLost() == 0, "manual->rust should not lose safety");
    PASS();
}

void test_mixed_patterns_report_gain_and_loss() {
    TEST(mixed_patterns_report_gain_and_loss);
    auto r = MemoryModelTranslator::analyze(
        "std::unique_ptr<Foo> p; q = malloc(8); free(q);",
        "cpp",
        "c");
    CHECK(r.translations.size() >= 2, "expected unique + manual translations");
    CHECK(r.countSafetyLost() >= 1, "expected at least one safety loss");
    CHECK(r.countSafetyGained() == 0, "cpp->c should not gain safety here");
    PASS();
}

int main() {
    std::cout << "Step 453: Memory Model Translation Tests\n";

    test_manual_c_to_rust_gains_safety();      // 1
    test_manual_c_to_java_gains_gc_safety();   // 2
    test_unique_cpp_to_c_loses_safety();       // 3
    test_shared_cpp_to_rust_arc_mapping();     // 4
    test_shared_rust_to_c_loses_safety();      // 5
    test_scope_lifetime_to_go_defer();         // 6
    test_scope_to_c_loses_scope_guarantee();   // 7
    test_detect_source_model_rust_borrowchecked(); // 8
    test_detect_target_model_java_gc();        // 9
    test_no_pattern_on_plain_code();           // 10
    test_safety_gain_count_for_manual_migration(); // 11
    test_mixed_patterns_report_gain_and_loss();    // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
