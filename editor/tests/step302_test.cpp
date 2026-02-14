// Step 302: New AST Nodes — Class/Interface/Generic (12 tests)
// Tests ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
// GenericType, TypeParameter: construction, properties, children, serialization.

#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/Serialization.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <memory>

using json = nlohmann::json;

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// 1. ClassDeclaration construction and properties
void test_class_construction() {
    TEST(class_construction);
    ClassDeclaration cls("cls1", "Animal");
    cls.superClass = "Entity";
    cls.isAbstract = true;

    CHECK(cls.conceptType == "ClassDeclaration", "wrong conceptType");
    CHECK(cls.id == "cls1", "wrong id");
    CHECK(cls.name == "Animal", "wrong name");
    CHECK(cls.superClass == "Entity", "wrong superClass");
    CHECK(cls.isAbstract == true, "wrong isAbstract");
    PASS();
}

// 2. ClassDeclaration child roles: methods, fields, annotations
void test_class_children() {
    TEST(class_children);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Dog");
    cls->superClass = "Animal";

    auto method = new MethodDeclaration("m1", "bark");
    method->className = "Dog";
    cls->addChild("methods", method);

    auto field = new Variable("v1", "name");
    cls->addChild("fields", field);

    CHECK(cls->getChildren("methods").size() == 1, "expected 1 method");
    CHECK(cls->getChildren("fields").size() == 1, "expected 1 field");
    CHECK(static_cast<MethodDeclaration*>(cls->getChildren("methods")[0])->name == "bark",
          "method name wrong");
    PASS();
}

// 3. InterfaceDeclaration construction
void test_interface_construction() {
    TEST(interface_construction);
    InterfaceDeclaration iface("if1", "Serializable");

    CHECK(iface.conceptType == "InterfaceDeclaration", "wrong conceptType");
    CHECK(iface.name == "Serializable", "wrong name");

    auto method = new MethodDeclaration("m1", "serialize");
    method->isVirtual = true;
    iface.addChild("methods", method);
    CHECK(iface.getChildren("methods").size() == 1, "expected 1 method");
    PASS();
}

// 4. MethodDeclaration extends Function with class-specific fields
void test_method_properties() {
    TEST(method_properties);
    MethodDeclaration meth("m1", "process");
    meth.className = "Worker";
    meth.isStatic = true;
    meth.visibility = "private";
    meth.isOverride = true;
    meth.isVirtual = false;

    CHECK(meth.conceptType == "MethodDeclaration", "wrong conceptType");
    CHECK(meth.name == "process", "wrong name");
    CHECK(meth.className == "Worker", "wrong className");
    CHECK(meth.isStatic == true, "wrong isStatic");
    CHECK(meth.visibility == "private", "wrong visibility");
    CHECK(meth.isOverride == true, "wrong isOverride");
    CHECK(meth.isVirtual == false, "wrong isVirtual");
    PASS();
}

// 5. MethodDeclaration inherits Function children (parameters, body)
void test_method_inherits_function() {
    TEST(method_inherits_function);
    auto meth = std::make_unique<MethodDeclaration>("m1", "run");

    auto param = new Parameter();
    param->id = "p1";
    param->name = "x";
    meth->addChild("parameters", param);

    auto ret = new Return();
    ret->id = "r1";
    meth->addChild("body", ret);

    CHECK(meth->getChildren("parameters").size() == 1, "expected 1 parameter");
    CHECK(meth->getChildren("body").size() == 1, "expected 1 body stmt");
    PASS();
}

// 6. GenericType construction and children
void test_generic_type() {
    TEST(generic_type);
    GenericType gen("gt1", "Map");

    CHECK(gen.conceptType == "GenericType", "wrong conceptType");
    CHECK(gen.baseName == "Map", "wrong baseName");

    auto k = new TypeParameter("tp1", "K");
    k->constraint = "Comparable";
    auto v = new TypeParameter("tp2", "V");
    gen.addChild("typeParameters", k);
    gen.addChild("typeParameters", v);

    CHECK(gen.getChildren("typeParameters").size() == 2, "expected 2 type params");
    PASS();
}

// 7. TypeParameter construction with constraint
void test_type_parameter() {
    TEST(type_parameter);
    TypeParameter tp("tp1", "T");
    tp.constraint = "Hashable";

    CHECK(tp.conceptType == "TypeParameter", "wrong conceptType");
    CHECK(tp.name == "T", "wrong name");
    CHECK(tp.constraint == "Hashable", "wrong constraint");

    TypeParameter tp2("tp2", "U");
    CHECK(tp2.constraint.empty(), "unconstrained should be empty");
    PASS();
}

// 8. ClassDeclaration JSON roundtrip
void test_class_json_roundtrip() {
    TEST(class_json_roundtrip);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Shape");
    cls->superClass = "Drawable";
    cls->isAbstract = true;

    auto method = new MethodDeclaration("m1", "draw");
    method->className = "Shape";
    method->isVirtual = true;
    cls->addChild("methods", method);

    json j = toJson(cls.get());
    CHECK(j["concept"] == "ClassDeclaration", "concept wrong");
    CHECK(j["properties"]["name"] == "Shape", "name wrong");
    CHECK(j["properties"]["superClass"] == "Drawable", "superClass wrong");
    CHECK(j["properties"]["isAbstract"] == true, "isAbstract wrong");

    ASTNode* restored = fromJson(j);
    CHECK(restored != nullptr, "fromJson returned null");
    CHECK(restored->conceptType == "ClassDeclaration", "restored wrong type");
    auto* rc = static_cast<ClassDeclaration*>(restored);
    CHECK(rc->name == "Shape", "restored name wrong");
    CHECK(rc->superClass == "Drawable", "restored superClass wrong");
    CHECK(rc->isAbstract == true, "restored isAbstract wrong");

    // Check children survived roundtrip
    CHECK(restored->getChildren("methods").size() == 1, "restored methods wrong");
    delete restored;
    PASS();
}

// 9. InterfaceDeclaration JSON roundtrip
void test_interface_json_roundtrip() {
    TEST(interface_json_roundtrip);
    auto iface = std::make_unique<InterfaceDeclaration>("if1", "Iterable");

    json j = toJson(iface.get());
    CHECK(j["properties"]["name"] == "Iterable", "name wrong in JSON");

    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "InterfaceDeclaration", "restored wrong type");
    CHECK(static_cast<InterfaceDeclaration*>(restored)->name == "Iterable",
          "restored name wrong");
    delete restored;
    PASS();
}

// 10. MethodDeclaration JSON roundtrip preserves all fields
void test_method_json_roundtrip() {
    TEST(method_json_roundtrip);
    auto meth = std::make_unique<MethodDeclaration>("m1", "update");
    meth->className = "Controller";
    meth->isStatic = true;
    meth->visibility = "protected";
    meth->isOverride = true;
    meth->isVirtual = false;

    json j = toJson(meth.get());
    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "MethodDeclaration", "restored wrong type");
    auto* rm = static_cast<MethodDeclaration*>(restored);
    CHECK(rm->name == "update", "name");
    CHECK(rm->className == "Controller", "className");
    CHECK(rm->isStatic == true, "isStatic");
    CHECK(rm->visibility == "protected", "visibility");
    CHECK(rm->isOverride == true, "isOverride");
    CHECK(rm->isVirtual == false, "isVirtual");
    delete restored;
    PASS();
}

// 11. GenericType + TypeParameter JSON roundtrip
void test_generic_json_roundtrip() {
    TEST(generic_json_roundtrip);
    auto gen = std::make_unique<GenericType>("gt1", "Optional");
    auto tp = new TypeParameter("tp1", "T");
    tp->constraint = "Sendable";
    gen->addChild("typeParameters", tp);

    json j = toJson(gen.get());
    CHECK(j["properties"]["baseName"] == "Optional", "baseName wrong");

    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "GenericType", "type wrong");
    auto* rg = static_cast<GenericType*>(restored);
    CHECK(rg->baseName == "Optional", "baseName");

    auto typeParams = restored->getChildren("typeParameters");
    CHECK(typeParams.size() == 1, "expected 1 type param");
    auto* rtp = static_cast<TypeParameter*>(typeParams[0]);
    CHECK(rtp->name == "T", "param name");
    CHECK(rtp->constraint == "Sendable", "param constraint");
    delete restored;
    PASS();
}

// 12. Nested structure: class with generic methods and fields
void test_nested_class_structure() {
    TEST(nested_class_structure);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Repository");
    cls->isAbstract = false;

    // Generic type parameter on class
    auto gt = new GenericType("gt1", "Repository");
    auto tp = new TypeParameter("tp1", "T");
    gt->addChild("typeParameters", tp);
    cls->addChild("interfaces", gt);

    // Method with parameter
    auto meth = new MethodDeclaration("m1", "find");
    meth->className = "Repository";
    meth->visibility = "public";
    auto param = new Parameter();
    param->id = "p1";
    param->name = "id";
    meth->addChild("parameters", param);
    cls->addChild("methods", meth);

    // Field
    auto field = new Variable("v1", "items");
    cls->addChild("fields", field);

    // JSON roundtrip
    json j = toJson(cls.get());
    ASTNode* restored = fromJson(j);
    CHECK(restored->conceptType == "ClassDeclaration", "type");
    CHECK(restored->getChildren("methods").size() == 1, "methods");
    CHECK(restored->getChildren("fields").size() == 1, "fields");
    CHECK(restored->getChildren("interfaces").size() == 1, "interfaces");

    auto* restoredMeth = static_cast<MethodDeclaration*>(
        restored->getChildren("methods")[0]);
    CHECK(restoredMeth->name == "find", "method name");
    CHECK(restoredMeth->getChildren("parameters").size() == 1, "method params");
    delete restored;
    PASS();
}

int main() {
    std::cout << "=== Step 302: New AST Nodes — Class/Interface/Generic ===\n";
    test_class_construction();
    test_class_children();
    test_interface_construction();
    test_method_properties();
    test_method_inherits_function();
    test_generic_type();
    test_type_parameter();
    test_class_json_roundtrip();
    test_interface_json_roundtrip();
    test_method_json_roundtrip();
    test_generic_json_roundtrip();
    test_nested_class_structure();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
