// Step 454: Phase 21a Integration Tests (8 tests)

#include "AlgorithmEquivalence.h"
#include "ConcurrencyTranslator.h"
#include "IntentTranslator.h"
#include "MemoryModelTranslator.h"
#include "TypeSystemTranslator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_intent_driven_sort_python_to_rust() {
    TEST(intent_driven_sort_python_to_rust);
    std::vector<FunctionIntent> funcs = {{"sort_items", "sort list ascending", "items.sort()"}};
    auto intent = IntentTranslator::translate(funcs, "python", "rust");
    CHECK(intent.hasTranslationFor("sort_items"), "expected translated function");
    auto t = intent.getTranslation("sort_items");
    CHECK(t.isIdiomatic, "expected idiomatic translation");
    CHECK(t.confidence >= 0.90f, "expected high confidence");
    PASS();
}

void test_algorithm_recognition_sort_to_stdlib() {
    TEST(algorithm_recognition_sort_to_stdlib);
    std::string src = "for(i=0;i<n;i++) for(j=0;j<n-1;j++) if(a[j]>a[j+1]) swap(a[j],a[j+1]);";
    auto algo = AlgorithmRecognizer::analyze(src, "c", "rust");
    CHECK(algo.hasPattern("bubble_sort"), "expected bubble sort detection");
    auto eq = algo.equivalentFor("bubble_sort");
    CHECK(eq.usesStdLib, "expected stdlib use");
    CHECK(eq.targetCode.find("sort") != std::string::npos, "expected rust sort mapping");
    PASS();
}

void test_type_translation_generics_preserved_java_to_rust() {
    TEST(type_translation_generics_preserved_java_to_rust);
    auto tr = TypeSystemTranslator::translate({"List<T>", "Map<K,V>"}, "java", "rust");
    CHECK(tr.getTranslation("List<T>").targetType == "Vec<T>", "expected Vec<T>");
    CHECK(tr.getTranslation("Map<K,V>").targetType == "HashMap<K,V>", "expected HashMap");
    CHECK(tr.countLossy() == 0, "expected no lossy mappings");
    PASS();
}

void test_type_translation_to_c_is_lossy_when_needed() {
    TEST(type_translation_to_c_is_lossy_when_needed);
    auto tr = TypeSystemTranslator::translate({"Optional<T>", "List<T>"}, "java", "c");
    CHECK(tr.countLossy() >= 2, "expected lossy mappings for C target");
    CHECK(tr.getTranslation("List<T>").annotation == "@Risk(high)", "expected risk annotation");
    PASS();
}

void test_concurrency_async_python_to_rust_requires_review() {
    TEST(concurrency_async_python_to_rust_requires_review);
    auto conc = ConcurrencyTranslator::analyze("async def fetch(): await io()", "python", "rust");
    CHECK(conc.hasPattern("async/await"), "expected async detection");
    CHECK(conc.countNeedsReview() >= 1, "expected runtime review requirement");
    PASS();
}

void test_memory_manual_c_to_rust_gains_safety() {
    TEST(memory_manual_c_to_rust_gains_safety);
    auto mem = MemoryModelTranslator::analyze("p = malloc(8); free(p);", "c", "rust");
    CHECK(mem.hasPattern("@Owner(Manual)"), "expected manual ownership pattern");
    CHECK(mem.countSafetyGained() >= 1, "expected safety gain");
    CHECK(mem.countSafetyLost() == 0, "expected no safety loss");
    PASS();
}

void test_no_intent_falls_back_to_structural_and_review() {
    TEST(no_intent_falls_back_to_structural_and_review);
    std::vector<FunctionIntent> funcs = {{"mystery", "", "x = a + b * c"}};
    auto intent = IntentTranslator::translate(funcs, "python", "rust");
    auto t = intent.getTranslation("mystery");
    CHECK(!t.isIdiomatic, "no intent should not be idiomatic");
    CHECK(t.needsReview, "no intent should trigger review");
    PASS();
}

void test_phase_21a_combined_pipeline_signals() {
    TEST(phase_21a_combined_pipeline_signals);
    std::vector<FunctionIntent> funcs = {{
        "rank_users",
        "sort list ascending",
        "for(i=0;i<n;i++) for(j=0;j<n-1;j++) if(a[j]>a[j+1]) swap(a[j],a[j+1]);"
    }};
    auto intent = IntentTranslator::translate(funcs, "python", "rust");
    auto algo = AlgorithmRecognizer::analyze(funcs[0].source, "python", "rust");
    auto types = TypeSystemTranslator::translate({"List<T>"}, "python", "rust");
    auto conc = ConcurrencyTranslator::analyze("async def rank_users(): await fetch()", "python", "rust");
    auto mem = MemoryModelTranslator::analyze("buf = malloc(64); free(buf);", "c", "rust");

    CHECK(intent.countIdiomatic() == 1, "expected idiomatic intent mapping");
    CHECK(algo.hasPattern("bubble_sort"), "expected algorithm pattern");
    CHECK(types.getTranslation("List<T>").targetType == "Vec<T>", "expected rust vector mapping");
    CHECK(conc.hasPattern("async/await"), "expected concurrency pattern");
    CHECK(mem.countSafetyGained() >= 1, "expected safety gain in memory translation");
    PASS();
}

int main() {
    std::cout << "Step 454: Phase 21a Integration Tests\n";

    test_intent_driven_sort_python_to_rust();               // 1
    test_algorithm_recognition_sort_to_stdlib();            // 2
    test_type_translation_generics_preserved_java_to_rust();// 3
    test_type_translation_to_c_is_lossy_when_needed();      // 4
    test_concurrency_async_python_to_rust_requires_review();// 5
    test_memory_manual_c_to_rust_gains_safety();            // 6
    test_no_intent_falls_back_to_structural_and_review();   // 7
    test_phase_21a_combined_pipeline_signals();             // 8

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
