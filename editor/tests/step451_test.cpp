// Step 451: Type System Translation Tests (12 tests)

#include "TypeSystemTranslator.h"

#include <iostream>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

void test_nullable_to_rust_option() {
    TEST(nullable_to_rust_option);
    auto r = TypeSystemTranslator::translate({"Optional<T>"}, "java", "rust");
    auto t = r.getTranslation("Optional<T>");
    CHECK(t.targetType == "Option<T>", "expected Option<T>");
    CHECK(t.safety == TypeSafety::Preserved, "nullable should be preserved");
    PASS();
}

void test_nullable_to_c_is_lossy() {
    TEST(nullable_to_c_is_lossy);
    auto r = TypeSystemTranslator::translate({"Optional<T>"}, "java", "c");
    auto t = r.getTranslation("Optional<T>");
    CHECK(t.targetType == "T*", "expected pointer representation");
    CHECK(t.safety == TypeSafety::Lossy, "nullable->C should be lossy");
    CHECK(t.annotation == "@Risk(medium)", "expected risk annotation");
    PASS();
}

void test_list_to_rust_vec() {
    TEST(list_to_rust_vec);
    auto r = TypeSystemTranslator::translate({"List<T>"}, "java", "rust");
    auto t = r.getTranslation("List<T>");
    CHECK(t.targetType == "Vec<T>", "expected Vec<T>");
    CHECK(t.safety == TypeSafety::Preserved, "generic list should preserve");
    PASS();
}

void test_list_to_c_void_ptr_lossy() {
    TEST(list_to_c_void_ptr_lossy);
    auto r = TypeSystemTranslator::translate({"List<T>"}, "java", "c");
    auto t = r.getTranslation("List<T>");
    CHECK(t.targetType == "void*", "expected void* in C");
    CHECK(t.safety == TypeSafety::Lossy, "generics->C should be lossy");
    CHECK(t.annotation == "@Risk(high)", "expected high risk annotation");
    PASS();
}

void test_map_to_go_map_type() {
    TEST(map_to_go_map_type);
    auto r = TypeSystemTranslator::translate({"Map<K,V>"}, "java", "go");
    auto t = r.getTranslation("Map<K,V>");
    CHECK(t.targetType == "map[K]V", "expected Go map");
    CHECK(t.safety == TypeSafety::Preserved, "map should preserve semantics");
    PASS();
}

void test_enum_to_c_narrowed() {
    TEST(enum_to_c_narrowed);
    auto r = TypeSystemTranslator::translate({"Enum"}, "java", "c");
    auto t = r.getTranslation("Enum");
    CHECK(t.targetType == "enum/constants", "expected C enum/constants");
    CHECK(t.safety == TypeSafety::Narrowed, "C enum should be narrowed");
    PASS();
}

void test_class_to_rust_trait_struct() {
    TEST(class_to_rust_trait_struct);
    auto r = TypeSystemTranslator::translate({"abstract class"}, "java", "rust");
    auto t = r.getTranslation("abstract class");
    CHECK(t.targetType == "trait + struct", "expected trait + struct");
    CHECK(t.safety == TypeSafety::Preserved, "should preserve behavior intent");
    PASS();
}

void test_class_to_c_vtable_lossy() {
    TEST(class_to_c_vtable_lossy);
    auto r = TypeSystemTranslator::translate({"class"}, "java", "c");
    auto t = r.getTranslation("class");
    CHECK(t.targetType == "struct + vtable", "expected struct + vtable");
    CHECK(t.safety == TypeSafety::Lossy, "class->C should be lossy");
    CHECK(t.annotation == "@Risk(medium)", "expected risk annotation");
    PASS();
}

void test_string_to_c_char_ptr_lossy() {
    TEST(string_to_c_char_ptr_lossy);
    auto r = TypeSystemTranslator::translate({"String"}, "java", "c");
    auto t = r.getTranslation("String");
    CHECK(t.targetType == "char*", "expected char*");
    CHECK(t.safety == TypeSafety::Lossy, "String->char* should be lossy");
    PASS();
}

void test_int_to_python_widened() {
    TEST(int_to_python_widened);
    auto r = TypeSystemTranslator::translate({"int"}, "c", "python");
    auto t = r.getTranslation("int");
    CHECK(t.targetType == "int", "expected python int");
    CHECK(t.safety == TypeSafety::Widened, "expected widened numeric range");
    PASS();
}

void test_unknown_type_preserved() {
    TEST(unknown_type_preserved);
    auto r = TypeSystemTranslator::translate({"CustomBlob"}, "java", "rust");
    auto t = r.getTranslation("CustomBlob");
    CHECK(t.targetType == "CustomBlob", "unknown type should stay unchanged");
    CHECK(t.safety == TypeSafety::Preserved, "unknown type should preserve");
    PASS();
}

void test_mixed_batch_translation_counts_lossy() {
    TEST(mixed_batch_translation_counts_lossy);
    auto r = TypeSystemTranslator::translate(
        {"Optional<T>", "List<T>", "Map<K,V>", "int"},
        "java",
        "c");
    CHECK(r.translations.size() == 4, "expected 4 translated types");
    CHECK(r.countLossy() >= 3, "expected at least 3 lossy mappings");
    CHECK(r.hasTranslation("Map<K,V>"), "expected map translation");
    PASS();
}

int main() {
    std::cout << "Step 451: Type System Translation Tests\n";

    test_nullable_to_rust_option();            // 1
    test_nullable_to_c_is_lossy();             // 2
    test_list_to_rust_vec();                   // 3
    test_list_to_c_void_ptr_lossy();           // 4
    test_map_to_go_map_type();                 // 5
    test_enum_to_c_narrowed();                 // 6
    test_class_to_rust_trait_struct();         // 7
    test_class_to_c_vtable_lossy();            // 8
    test_string_to_c_char_ptr_lossy();         // 9
    test_int_to_python_widened();              // 10
    test_unknown_type_preserved();             // 11
    test_mixed_batch_translation_counts_lossy(); // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
