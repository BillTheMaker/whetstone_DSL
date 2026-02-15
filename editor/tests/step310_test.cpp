// Step 310: Kotlin Generator Tests (12 tests)
// Verifies KotlinGenerator produces idiomatic Kotlin from Whetstone AST:
// fun, val, suspend fun, class, interface, lambda, generics, annotations.

#include "ast/KotlinGenerator.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Type.h"
#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
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

// 1. Generate a simple function
void test_generate_function() {
    TEST(generate_function);
    auto fn = std::make_unique<Function>("fn1", "greet");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    auto retVal = std::make_unique<StringLiteral>("s1", "hello");
    retStmt->addChild("value", retVal.release());
    fn->addChild("body", retStmt.release());

    KotlinGenerator gen;
    std::string out = gen.generate(fn.get());
    CHECK(out.find("fun greet") != std::string::npos, "missing 'fun greet'");
    CHECK(out.find("return \"hello\"") != std::string::npos, "missing return statement");
    CHECK(out.find("{") != std::string::npos, "missing opening brace");
    CHECK(out.find("}") != std::string::npos, "missing closing brace");
    PASS();
}

// 2. Generate val variable
void test_generate_variable() {
    TEST(generate_variable);
    auto var = std::make_unique<Variable>("v1", "count");
    auto type = std::make_unique<PrimitiveType>("pt1", "int");
    var->addChild("type", type.release());
    auto init = std::make_unique<IntegerLiteral>("il1", 42);
    var->addChild("initializer", init.release());

    KotlinGenerator gen;
    std::string out = gen.generate(var.get());
    CHECK(out.find("val count") != std::string::npos, "missing 'val count'");
    CHECK(out.find("Int") != std::string::npos, "missing Kotlin Int type");
    CHECK(out.find("42") != std::string::npos, "missing initial value");
    PASS();
}

// 3. Type mappings: int→Int, string→String, bool→Boolean, void→Unit
void test_type_mappings() {
    TEST(type_mappings);
    KotlinGenerator gen;

    PrimitiveType intT("t1", "int");
    PrimitiveType strT("t2", "string");
    PrimitiveType boolT("t3", "bool");
    PrimitiveType voidT("t4", "void");
    PrimitiveType dblT("t5", "double");

    CHECK(gen.generate(&intT) == "Int", "int should map to Int");
    CHECK(gen.generate(&strT) == "String", "string should map to String");
    CHECK(gen.generate(&boolT) == "Boolean", "bool should map to Boolean");
    CHECK(gen.generate(&voidT) == "Unit", "void should map to Unit");
    CHECK(gen.generate(&dblT) == "Double", "double should map to Double");
    PASS();
}

// 4. Generate class with fields and methods
void test_generate_class() {
    TEST(generate_class);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Person");
    cls->superClass = "Entity";

    auto field = std::make_unique<Variable>("f1", "name");
    cls->addChild("fields", field.release());

    auto meth = std::make_unique<MethodDeclaration>("m1", "greet");
    meth->className = "Person";
    cls->addChild("methods", meth.release());

    KotlinGenerator gen;
    std::string out = gen.generate(cls.get());
    CHECK(out.find("class Person") != std::string::npos, "missing 'class Person'");
    CHECK(out.find(": Entity()") != std::string::npos, "missing Kotlin-style superclass");
    CHECK(out.find("name") != std::string::npos, "missing field name");
    CHECK(out.find("fun greet") != std::string::npos, "missing method");
    PASS();
}

// 5. Generate interface
void test_generate_interface() {
    TEST(generate_interface);
    auto iface = std::make_unique<InterfaceDeclaration>("if1", "Drawable");
    auto m1 = std::make_unique<MethodDeclaration>("m1", "draw");
    auto m2 = std::make_unique<MethodDeclaration>("m2", "resize");
    iface->addChild("methods", m1.release());
    iface->addChild("methods", m2.release());

    KotlinGenerator gen;
    std::string out = gen.generate(iface.get());
    CHECK(out.find("interface Drawable") != std::string::npos, "missing 'interface Drawable'");
    CHECK(out.find("fun draw") != std::string::npos, "missing 'fun draw'");
    CHECK(out.find("fun resize") != std::string::npos, "missing 'fun resize'");
    PASS();
}

// 6. Generate suspend fun (async)
void test_generate_async_function() {
    TEST(generate_async_function);
    auto af = std::make_unique<AsyncFunction>("af1", "loadData");
    auto awaitExpr = std::make_unique<AwaitExpression>("aw1");
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "fetchFromApi";
    awaitExpr->addChild("expression", call.release());
    auto stmt = std::make_unique<ExpressionStatement>();
    stmt->id = "es1";
    stmt->addChild("expression", awaitExpr.release());
    af->addChild("body", stmt.release());

    KotlinGenerator gen;
    std::string out = gen.generate(af.get());
    CHECK(out.find("suspend fun loadData") != std::string::npos, "missing 'suspend fun loadData'");
    // Kotlin await is implicit — just the expression (no 'await' keyword)
    CHECK(out.find("fetchFromApi") != std::string::npos, "missing function call");
    PASS();
}

// 7. Generate Kotlin lambda { params -> body }
void test_generate_lambda() {
    TEST(generate_lambda);
    auto lam = std::make_unique<LambdaExpression>("lam1");
    auto p1 = std::make_unique<Parameter>("p1", "x");
    auto p2 = std::make_unique<Parameter>("p2", "y");
    lam->addChild("parameters", p1.release());
    lam->addChild("parameters", p2.release());
    auto body = std::make_unique<BinaryOperation>("bo1", "+");
    body->addChild("left", new VariableReference("vr1", "x"));
    body->addChild("right", new VariableReference("vr2", "y"));
    lam->addChild("body", body.release());

    KotlinGenerator gen;
    std::string out = gen.generate(lam.get());
    CHECK(out.find("{") != std::string::npos, "missing '{'");
    CHECK(out.find("->") != std::string::npos, "missing '->' lambda arrow");
    CHECK(out.find("x") != std::string::npos, "missing param x");
    CHECK(out.find("y") != std::string::npos, "missing param y");
    CHECK(out.find("}") != std::string::npos, "missing '}'");
    PASS();
}

// 8. Generate generic type
void test_generate_generic_type() {
    TEST(generate_generic_type);
    auto gen_type = std::make_unique<GenericType>("g1", "Map");
    auto tp1 = std::make_unique<TypeParameter>("tp1", "K");
    auto tp2 = std::make_unique<TypeParameter>("tp2", "V");
    gen_type->addChild("typeParameters", tp1.release());
    gen_type->addChild("typeParameters", tp2.release());

    KotlinGenerator gen;
    std::string out = gen.generate(gen_type.get());
    CHECK(out.find("Map<K, V>") != std::string::npos, "missing 'Map<K, V>'");
    PASS();
}

// 9. Generate decorator annotation → @name
void test_generate_decorator() {
    TEST(generate_decorator);
    auto dec = std::make_unique<DecoratorAnnotation>("dec1", "JvmStatic");

    KotlinGenerator gen;
    std::string out = gen.generate(dec.get());
    CHECK(out.find("@JvmStatic") != std::string::npos, "missing '@JvmStatic'");
    PASS();
}

// 10. Collection types: List→List, Set→Set, Map→Map
void test_collection_types() {
    TEST(collection_types);
    KotlinGenerator gen;

    auto listT = std::make_unique<ListType>();
    listT->id = "lt1";
    auto listEl = std::make_unique<PrimitiveType>("pt1", "string");
    listT->addChild("elementType", listEl.release());
    CHECK(gen.generate(listT.get()).find("List<String>") != std::string::npos, "List<String> expected");

    auto setT = std::make_unique<SetType>();
    setT->id = "st1";
    auto setEl = std::make_unique<PrimitiveType>("pt2", "int");
    setT->addChild("elementType", setEl.release());
    CHECK(gen.generate(setT.get()).find("Set<Int>") != std::string::npos, "Set<Int> expected");

    auto mapT = std::make_unique<MapType>();
    mapT->id = "mt1";
    auto kT = std::make_unique<PrimitiveType>("pt3", "string");
    auto vT = std::make_unique<PrimitiveType>("pt4", "int");
    mapT->addChild("keyType", kT.release());
    mapT->addChild("valueType", vT.release());
    CHECK(gen.generate(mapT.get()).find("Map<String, Int>") != std::string::npos, "Map<String, Int> expected");
    PASS();
}

// 11. Generate for loop → for (item in ...)
void test_generate_for_loop() {
    TEST(generate_for_loop);
    auto loop = std::make_unique<ForLoop>();
    loop->id = "fl1";
    loop->iteratorName = "item";
    auto iter = std::make_unique<VariableReference>("vr1", "items");
    loop->addChild("iterable", iter.release());
    auto bodyStmt = std::make_unique<ExpressionStatement>();
    bodyStmt->id = "es1";
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "println";
    call->addChild("arguments", new VariableReference("vr2", "item"));
    bodyStmt->addChild("expression", call.release());
    loop->addChild("body", bodyStmt.release());

    KotlinGenerator gen;
    std::string out = gen.generate(loop.get());
    CHECK(out.find("for (item in items)") != std::string::npos, "missing 'for (item in items)'");
    CHECK(out.find("println") != std::string::npos, "missing println call");
    PASS();
}

// 12. Cross-language: generate module with all node types
void test_generate_full_module() {
    TEST(generate_full_module);
    auto mod = std::make_unique<Module>("mod1", "test_module", "kotlin");

    auto var = std::make_unique<Variable>("v1", "version");
    auto init = std::make_unique<StringLiteral>("s1", "1.0");
    var->addChild("initializer", init.release());
    mod->addChild("variables", var.release());

    auto fn = std::make_unique<Function>("fn1", "main");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    fn->addChild("body", retStmt.release());
    mod->addChild("functions", fn.release());

    KotlinGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("Module:") != std::string::npos, "missing Module comment");
    CHECK(out.find("val version") != std::string::npos, "missing variable");
    CHECK(out.find("fun main") != std::string::npos, "missing function");
    PASS();
}

int main() {
    std::cout << "Step 310: Kotlin Generator Tests\n";

    test_generate_function();          // 1
    test_generate_variable();          // 2
    test_type_mappings();              // 3
    test_generate_class();             // 4
    test_generate_interface();         // 5
    test_generate_async_function();    // 6
    test_generate_lambda();            // 7
    test_generate_generic_type();      // 8
    test_generate_decorator();         // 9
    test_collection_types();           // 10
    test_generate_for_loop();          // 11
    test_generate_full_module();       // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
