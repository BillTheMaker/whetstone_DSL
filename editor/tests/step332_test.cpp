// Step 332: Multiple Inheritance in ClassDeclaration (12 tests)

#include "ast/ClassDeclaration.h"
#include "ast/Serialization.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. Single base class (backward compat via superClass)
void test_single_base_backward_compat() {
    TEST(single_base_backward_compat);
    ClassDeclaration cd("c1", "Foo");
    cd.superClass = "Bar";

    auto bases = cd.getBases();
    CHECK(bases.size() == 1, "should have 1 base");
    CHECK(bases[0].name == "Bar", "base name");
    CHECK(bases[0].accessSpecifier == "public", "default public");
    CHECK(!bases[0].isVirtual, "default not virtual");
    PASS();
}

// 2. Multiple bases via addBase
void test_multiple_bases() {
    TEST(multiple_bases);
    ClassDeclaration cd("c1", "Diamond");
    cd.addBase("Base1", "public");
    cd.addBase("Base2", "private");
    cd.addBase("Base3", "protected");

    CHECK(cd.baseClasses.size() == 3, "3 bases");
    CHECK(cd.baseClasses[0].name == "Base1", "first base");
    CHECK(cd.baseClasses[1].name == "Base2", "second base");
    CHECK(cd.baseClasses[2].name == "Base3", "third base");
    // superClass set to first
    CHECK(cd.superClass == "Base1", "superClass = first base");
    PASS();
}

// 3. Access specifiers
void test_access_specifiers() {
    TEST(access_specifiers);
    ClassDeclaration cd("c1", "Derived");
    cd.addBase("PubBase", "public");
    cd.addBase("PrivBase", "private");
    cd.addBase("ProtBase", "protected");

    CHECK(cd.baseClasses[0].accessSpecifier == "public", "public");
    CHECK(cd.baseClasses[1].accessSpecifier == "private", "private");
    CHECK(cd.baseClasses[2].accessSpecifier == "protected", "protected");
    PASS();
}

// 4. Virtual inheritance flag
void test_virtual_inheritance() {
    TEST(virtual_inheritance);
    ClassDeclaration cd("c1", "VDerived");
    cd.addBase("VBase", "public", true);
    cd.addBase("NormalBase", "public", false);

    CHECK(cd.baseClasses[0].isVirtual, "first is virtual");
    CHECK(!cd.baseClasses[1].isVirtual, "second not virtual");
    PASS();
}

// 5. BaseClass JSON roundtrip via Serialization
void test_base_class_json_roundtrip() {
    TEST(base_class_json_roundtrip);
    ClassDeclaration cd("c1", "Multi");
    cd.addBase("A", "public", false);
    cd.addBase("B", "private", true);
    cd.addBase("C", "protected", false);
    cd.isAbstract = true;

    json j = propertiesToJson(&cd);
    CHECK(j.contains("baseClasses"), "has baseClasses");
    CHECK(j["baseClasses"].size() == 3, "3 in JSON");
    CHECK(j["baseClasses"][1]["name"] == "B", "B name");
    CHECK(j["baseClasses"][1]["isVirtual"] == true, "B virtual");
    CHECK(j["baseClasses"][1]["accessSpecifier"] == "private", "B private");

    // Roundtrip
    ClassDeclaration cd2;
    setPropertiesFromJson(&cd2, j);
    CHECK(cd2.name == "Multi", "name preserved");
    CHECK(cd2.isAbstract, "isAbstract preserved");
    CHECK(cd2.baseClasses.size() == 3, "3 bases roundtrip");
    CHECK(cd2.baseClasses[0].name == "A", "A roundtrip");
    CHECK(cd2.baseClasses[1].name == "B", "B roundtrip");
    CHECK(cd2.baseClasses[1].isVirtual, "B virtual roundtrip");
    CHECK(cd2.baseClasses[2].accessSpecifier == "protected", "C access roundtrip");
    PASS();
}

// 6. Mixed virtual and non-virtual
void test_mixed_virtual() {
    TEST(mixed_virtual);
    ClassDeclaration cd("c1", "MixedDerived");
    cd.addBase("VirtA", "public", true);
    cd.addBase("NormB", "public", false);
    cd.addBase("VirtC", "private", true);

    int virtualCount = 0;
    for (const auto& b : cd.baseClasses) {
        if (b.isVirtual) virtualCount++;
    }
    CHECK(virtualCount == 2, "2 virtual bases");
    PASS();
}

// 7. Diamond detection: D → B + C, B → A, C → A
void test_diamond_detection() {
    TEST(diamond_detection);
    std::map<std::string, std::vector<std::string>> classMap;
    classMap["A"] = {};
    classMap["B"] = {"A"};
    classMap["C"] = {"A"};
    classMap["D"] = {"B", "C"};

    CHECK(ClassDeclaration::hasDiamondInheritance("D", classMap), "D has diamond");
    CHECK(!ClassDeclaration::hasDiamondInheritance("B", classMap), "B no diamond");
    CHECK(!ClassDeclaration::hasDiamondInheritance("A", classMap), "A no diamond");
    PASS();
}

// 8. Empty bases list
void test_empty_bases() {
    TEST(empty_bases);
    ClassDeclaration cd("c1", "Standalone");
    CHECK(cd.baseClasses.empty(), "no bases");
    CHECK(cd.superClass.empty(), "no superClass");
    auto bases = cd.getBases();
    CHECK(bases.empty(), "getBases empty");

    // JSON should not have baseClasses key
    json j = propertiesToJson(&cd);
    CHECK(!j.contains("baseClasses"), "no baseClasses in JSON");
    PASS();
}

// 9. Legacy superClass migration via getBases()
void test_legacy_superclass_migration() {
    TEST(legacy_superclass_migration);
    ClassDeclaration cd("c1", "Legacy");
    cd.superClass = "OldBase";
    // baseClasses not set — getBases should migrate

    auto bases = cd.getBases();
    CHECK(bases.size() == 1, "1 migrated base");
    CHECK(bases[0].name == "OldBase", "migrated name");
    CHECK(bases[0].accessSpecifier == "public", "default public");
    CHECK(!bases[0].isVirtual, "default not virtual");
    PASS();
}

// 10. addBase helper
void test_addBase_helper() {
    TEST(addBase_helper);
    ClassDeclaration cd("c1", "TestClass");
    cd.addBase("First");  // defaults: public, not virtual
    cd.addBase("Second", "private");  // not virtual
    cd.addBase("Third", "protected", true);  // virtual

    CHECK(cd.baseClasses.size() == 3, "3 bases");
    CHECK(cd.baseClasses[0].accessSpecifier == "public", "First default public");
    CHECK(!cd.baseClasses[0].isVirtual, "First default not virtual");
    CHECK(cd.baseClasses[1].accessSpecifier == "private", "Second private");
    CHECK(cd.baseClasses[2].isVirtual, "Third virtual");
    PASS();
}

// 11. Full serialization roundtrip via toJson/fromJson
void test_full_serialization_roundtrip() {
    TEST(full_serialization_roundtrip);
    auto* cd = new ClassDeclaration("c1", "FullTest");
    cd->addBase("Alpha", "public", false);
    cd->addBase("Beta", "private", true);
    cd->isAbstract = true;

    json j = toJson(cd);
    CHECK(j["concept"] == "ClassDeclaration", "conceptType");
    CHECK(j["properties"]["baseClasses"].size() == 2, "2 bases in full JSON");

    auto* restored = static_cast<ClassDeclaration*>(fromJson(j));
    CHECK(restored != nullptr, "restored not null");
    CHECK(restored->name == "FullTest", "name restored");
    CHECK(restored->baseClasses.size() == 2, "2 bases restored");
    CHECK(restored->baseClasses[0].name == "Alpha", "Alpha restored");
    CHECK(restored->baseClasses[1].isVirtual, "Beta virtual restored");
    CHECK(restored->isAbstract, "isAbstract restored");

    delete cd;
    delete restored;
    PASS();
}

// 12. No diamond: linear chain A → B → C (no sharing)
void test_no_diamond_linear() {
    TEST(no_diamond_linear);
    std::map<std::string, std::vector<std::string>> classMap;
    classMap["A"] = {};
    classMap["B"] = {"A"};
    classMap["C"] = {"B"};

    CHECK(!ClassDeclaration::hasDiamondInheritance("C", classMap), "linear = no diamond");
    CHECK(!ClassDeclaration::hasDiamondInheritance("B", classMap), "B = no diamond");

    // Multiple bases but no shared ancestor
    classMap["X"] = {};
    classMap["Y"] = {};
    classMap["Z"] = {"X", "Y"};
    CHECK(!ClassDeclaration::hasDiamondInheritance("Z", classMap), "disjoint bases = no diamond");
    PASS();
}

int main() {
    std::cout << "=== Step 332: Multiple Inheritance in ClassDeclaration ===\n";
    try {
        test_single_base_backward_compat();
        test_multiple_bases();
        test_access_specifiers();
        test_virtual_inheritance();
        test_base_class_json_roundtrip();
        test_mixed_virtual();
        test_diamond_detection();
        test_empty_bases();
        test_legacy_superclass_migration();
        test_addBase_helper();
        test_full_serialization_roundtrip();
        test_no_diamond_linear();
    } catch (const std::exception& e) {
        std::cout << "EXCEPTION: " << e.what() << "\n";
        ++failed;
    }
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
