// Step 312: C# Generator Tests (12 tests)
// Verifies CSharpGenerator produces idiomatic C# from Whetstone AST:
// public void, Allman braces, foreach, class, interface, async Task, await,
// lambda, generic types, decorator → [Attribute].

#include "ast/CSharpGenerator.h"
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

// 1. Generate a simple function → public void
void test_generate_function() {
    TEST(generate_function);
    auto fn = std::make_unique<Function>("fn1", "DoWork");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    auto retVal = std::make_unique<StringLiteral>("s1", "done");
    retStmt->addChild("value", retVal.release());
    fn->addChild("body", retStmt.release());

    CSharpGenerator gen;
    std::string out = gen.generate(fn.get());
    CHECK(out.find("public void DoWork") != std::string::npos, "missing 'public void DoWork'");
    CHECK(out.find("return \"done\"") != std::string::npos, "missing return statement");
    CHECK(out.find("{\n") != std::string::npos, "missing Allman-style opening brace");
    PASS();
}

// 2. Generate variable → var/type
void test_generate_variable() {
    TEST(generate_variable);
    auto var = std::make_unique<Variable>("v1", "count");
    auto type = std::make_unique<PrimitiveType>("pt1", "int");
    var->addChild("type", type.release());
    auto init = std::make_unique<IntegerLiteral>("il1", 42);
    var->addChild("initializer", init.release());

    CSharpGenerator gen;
    std::string out = gen.generate(var.get());
    CHECK(out.find("int count") != std::string::npos, "missing 'int count'");
    CHECK(out.find("= 42") != std::string::npos, "missing initial value");
    CHECK(out.find(";") != std::string::npos, "missing semicolon");
    PASS();
}

// 3. Type mappings
void test_type_mappings() {
    TEST(type_mappings);
    CSharpGenerator gen;

    PrimitiveType intT("t1", "int");
    PrimitiveType strT("t2", "string");
    PrimitiveType boolT("t3", "bool");
    PrimitiveType voidT("t4", "void");
    PrimitiveType floatT("t5", "float");
    PrimitiveType dblT("t6", "double");

    CHECK(gen.generate(&intT) == "int", "int should map to int");
    CHECK(gen.generate(&strT) == "string", "string should map to string");
    CHECK(gen.generate(&boolT) == "bool", "bool should map to bool");
    CHECK(gen.generate(&voidT) == "void", "void should map to void");
    CHECK(gen.generate(&floatT) == "float", "float should map to float");
    CHECK(gen.generate(&dblT) == "double", "double should map to double");
    PASS();
}

// 4. Generate class with inheritance (Allman braces)
void test_generate_class() {
    TEST(generate_class);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Dog");
    cls->superClass = "Animal";

    auto field = std::make_unique<Variable>("f1", "breed");
    cls->addChild("fields", field.release());

    auto meth = std::make_unique<MethodDeclaration>("m1", "Bark");
    meth->className = "Dog";
    meth->visibility = "public";
    cls->addChild("methods", meth.release());

    CSharpGenerator gen;
    std::string out = gen.generate(cls.get());
    CHECK(out.find("class Dog") != std::string::npos, "missing 'class Dog'");
    CHECK(out.find(": Animal") != std::string::npos, "missing ': Animal' C# inheritance");
    CHECK(out.find("breed") != std::string::npos, "missing field");
    CHECK(out.find("Bark") != std::string::npos, "missing method");
    PASS();
}

// 5. Generate interface
void test_generate_interface() {
    TEST(generate_interface);
    auto iface = std::make_unique<InterfaceDeclaration>("if1", "IRenderable");
    auto m1 = std::make_unique<MethodDeclaration>("m1", "Render");
    auto m2 = std::make_unique<MethodDeclaration>("m2", "Update");
    iface->addChild("methods", m1.release());
    iface->addChild("methods", m2.release());

    CSharpGenerator gen;
    std::string out = gen.generate(iface.get());
    CHECK(out.find("interface IRenderable") != std::string::npos, "missing 'interface IRenderable'");
    CHECK(out.find("Render") != std::string::npos, "missing 'Render'");
    CHECK(out.find("Update") != std::string::npos, "missing 'Update'");
    PASS();
}

// 6. Generate async Task method
void test_generate_async_function() {
    TEST(generate_async_function);
    auto af = std::make_unique<AsyncFunction>("af1", "LoadAsync");
    auto awaitExpr = std::make_unique<AwaitExpression>("aw1");
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "FetchData";
    awaitExpr->addChild("expression", call.release());
    auto stmt = std::make_unique<ExpressionStatement>();
    stmt->id = "es1";
    stmt->addChild("expression", awaitExpr.release());
    af->addChild("body", stmt.release());

    CSharpGenerator gen;
    std::string out = gen.generate(af.get());
    CHECK(out.find("public async Task LoadAsync") != std::string::npos, "missing 'public async Task LoadAsync'");
    CHECK(out.find("await FetchData()") != std::string::npos, "missing 'await FetchData()'");
    PASS();
}

// 7. Generate C# lambda (x) => body
void test_generate_lambda() {
    TEST(generate_lambda);
    auto lam = std::make_unique<LambdaExpression>("lam1");
    auto p1 = std::make_unique<Parameter>("p1", "x");
    lam->addChild("parameters", p1.release());
    auto body = std::make_unique<BinaryOperation>("bo1", "*");
    body->addChild("left", new VariableReference("vr1", "x"));
    body->addChild("right", new IntegerLiteral("il1", 2));
    lam->addChild("body", body.release());

    CSharpGenerator gen;
    std::string out = gen.generate(lam.get());
    CHECK(out.find("(x) =>") != std::string::npos, "missing '(x) =>' lambda syntax");
    CHECK(out.find("x * 2") != std::string::npos, "missing lambda body");
    PASS();
}

// 8. Generate generic type
void test_generate_generic_type() {
    TEST(generate_generic_type);
    auto gen_type = std::make_unique<GenericType>("g1", "Dictionary");
    auto tp1 = std::make_unique<TypeParameter>("tp1", "TKey");
    auto tp2 = std::make_unique<TypeParameter>("tp2", "TValue");
    gen_type->addChild("typeParameters", tp1.release());
    gen_type->addChild("typeParameters", tp2.release());

    CSharpGenerator gen;
    std::string out = gen.generate(gen_type.get());
    CHECK(out.find("Dictionary<TKey, TValue>") != std::string::npos, "missing 'Dictionary<TKey, TValue>'");
    PASS();
}

// 9. Generate decorator annotation → [Attribute]
void test_generate_decorator() {
    TEST(generate_decorator);
    auto dec = std::make_unique<DecoratorAnnotation>("dec1", "Serializable");

    CSharpGenerator gen;
    std::string out = gen.generate(dec.get());
    CHECK(out.find("[Serializable]") != std::string::npos, "missing '[Serializable]'");
    PASS();
}

// 10. Collection types: List<>, HashSet<>, Dictionary<>
void test_collection_types() {
    TEST(collection_types);
    CSharpGenerator gen;

    auto listT = std::make_unique<ListType>();
    listT->id = "lt1";
    auto listEl = std::make_unique<PrimitiveType>("pt1", "string");
    listT->addChild("elementType", listEl.release());
    CHECK(gen.generate(listT.get()).find("List<string>") != std::string::npos, "List<string> expected");

    auto setT = std::make_unique<SetType>();
    setT->id = "st1";
    auto setEl = std::make_unique<PrimitiveType>("pt2", "int");
    setT->addChild("elementType", setEl.release());
    CHECK(gen.generate(setT.get()).find("HashSet<int>") != std::string::npos, "HashSet<int> expected");

    auto mapT = std::make_unique<MapType>();
    mapT->id = "mt1";
    auto kT = std::make_unique<PrimitiveType>("pt3", "string");
    auto vT = std::make_unique<PrimitiveType>("pt4", "int");
    mapT->addChild("keyType", kT.release());
    mapT->addChild("valueType", vT.release());
    CHECK(gen.generate(mapT.get()).find("Dictionary<string, int>") != std::string::npos, "Dictionary<string, int> expected");
    PASS();
}

// 11. Generate foreach loop
void test_generate_foreach_loop() {
    TEST(generate_foreach_loop);
    auto loop = std::make_unique<ForLoop>();
    loop->id = "fl1";
    loop->iteratorName = "item";
    auto iter = std::make_unique<VariableReference>("vr1", "collection");
    loop->addChild("iterable", iter.release());
    auto bodyStmt = std::make_unique<ExpressionStatement>();
    bodyStmt->id = "es1";
    auto call = std::make_unique<FunctionCall>();
    call->id = "fc1";
    call->functionName = "Process";
    call->addChild("arguments", new VariableReference("vr2", "item"));
    bodyStmt->addChild("expression", call.release());
    loop->addChild("body", bodyStmt.release());

    CSharpGenerator gen;
    std::string out = gen.generate(loop.get());
    CHECK(out.find("foreach") != std::string::npos, "missing 'foreach'");
    CHECK(out.find("item") != std::string::npos, "missing iterator 'item'");
    CHECK(out.find("collection") != std::string::npos, "missing 'collection'");
    PASS();
}

// 12. Full module generation
void test_generate_full_module() {
    TEST(generate_full_module);
    auto mod = std::make_unique<Module>("mod1", "test_module", "csharp");

    auto var = std::make_unique<Variable>("v1", "version");
    auto init = std::make_unique<StringLiteral>("s1", "2.0");
    var->addChild("initializer", init.release());
    mod->addChild("variables", var.release());

    auto fn = std::make_unique<Function>("fn1", "Main");
    auto retStmt = std::make_unique<Return>();
    retStmt->id = "r1";
    fn->addChild("body", retStmt.release());
    mod->addChild("functions", fn.release());

    CSharpGenerator gen;
    std::string out = gen.generate(mod.get());
    CHECK(out.find("Module:") != std::string::npos, "missing Module comment");
    CHECK(out.find("version") != std::string::npos, "missing variable");
    CHECK(out.find("public void Main") != std::string::npos, "missing function");
    PASS();
}

int main() {
    std::cout << "Step 312: C# Generator Tests\n";

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
    test_generate_foreach_loop();      // 11
    test_generate_full_module();       // 12

    std::cout << "\nResults: " << passed << "/" << (passed + failed)
              << " passed\n";
    return failed == 0 ? 0 : 1;
}
