// Step 304: Serialization + Dispatch for New Nodes (12 tests)
// Tests that all 9 new AST node types (ClassDeclaration, InterfaceDeclaration,
// MethodDeclaration, GenericType, TypeParameter, AsyncFunction, AwaitExpression,
// LambdaExpression, DecoratorAnnotation) serialize/deserialize correctly and
// dispatch through PythonGenerator and CppGenerator with non-empty output.

#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/Module.h"
#include "ast/Function.h"
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

// 1. ClassDeclaration dispatch — Python generates "class Name:"
void test_class_python_dispatch() {
    TEST(class_python_dispatch);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Animal");
    cls->superClass = "LivingThing";

    PythonGenerator pyGen;
    std::string out = pyGen.generate(cls.get());
    CHECK(!out.empty(), "output should not be empty");
    CHECK(out.find("class Animal") != std::string::npos, "should contain 'class Animal'");
    CHECK(out.find("LivingThing") != std::string::npos, "should contain superclass");
    PASS();
}

// 2. ClassDeclaration dispatch — C++ generates "class Name : public Super {"
void test_class_cpp_dispatch() {
    TEST(class_cpp_dispatch);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Animal");
    cls->superClass = "LivingThing";

    CppGenerator cppGen;
    std::string out = cppGen.generate(cls.get());
    CHECK(!out.empty(), "output should not be empty");
    CHECK(out.find("class Animal") != std::string::npos, "should contain 'class Animal'");
    CHECK(out.find("public LivingThing") != std::string::npos, "should contain ': public LivingThing'");
    PASS();
}

// 3. InterfaceDeclaration dispatch — both generators produce output
void test_interface_dispatch() {
    TEST(interface_dispatch);
    auto iface = std::make_unique<InterfaceDeclaration>("if1", "Drawable");

    PythonGenerator pyGen;
    CppGenerator cppGen;
    std::string pyOut = pyGen.generate(iface.get());
    std::string cppOut = cppGen.generate(iface.get());
    CHECK(!pyOut.empty(), "Python output should not be empty");
    CHECK(!cppOut.empty(), "C++ output should not be empty");
    CHECK(pyOut.find("Drawable") != std::string::npos, "Python should contain name");
    CHECK(cppOut.find("Drawable") != std::string::npos, "C++ should contain name");
    PASS();
}

// 4. MethodDeclaration dispatch — Python generates "def name(self):"
void test_method_python_dispatch() {
    TEST(method_python_dispatch);
    auto meth = std::make_unique<MethodDeclaration>("m1", "draw");
    meth->isStatic = false;
    meth->isVirtual = true;

    PythonGenerator pyGen;
    std::string out = pyGen.generate(meth.get());
    CHECK(!out.empty(), "output should not be empty");
    CHECK(out.find("def draw") != std::string::npos, "should contain 'def draw'");
    PASS();
}

// 5. GenericType dispatch — Python "Name[T]", C++ "Name<T>"
void test_generic_type_dispatch() {
    TEST(generic_type_dispatch);
    auto gen = std::make_unique<GenericType>("gt1", "Container");
    auto tp = new TypeParameter("tp1", "T");
    gen->addChild("typeParameters", tp);

    PythonGenerator pyGen;
    CppGenerator cppGen;
    std::string pyOut = pyGen.generate(gen.get());
    std::string cppOut = cppGen.generate(gen.get());
    CHECK(pyOut.find("Container") != std::string::npos, "Python should contain 'Container'");
    CHECK(pyOut.find("T") != std::string::npos, "Python should contain 'T'");
    CHECK(cppOut.find("Container") != std::string::npos, "C++ should contain 'Container'");
    CHECK(cppOut.find("<") != std::string::npos, "C++ should use angle brackets");
    PASS();
}

// 6. AsyncFunction dispatch — Python "async def name():", C++ "std::future<void>"
void test_async_function_dispatch() {
    TEST(async_function_dispatch);
    auto af = std::make_unique<AsyncFunction>("af1", "fetchData");

    PythonGenerator pyGen;
    CppGenerator cppGen;
    std::string pyOut = pyGen.generate(af.get());
    std::string cppOut = cppGen.generate(af.get());
    CHECK(pyOut.find("async def fetchData") != std::string::npos, "Python should have 'async def fetchData'");
    CHECK(cppOut.find("std::future") != std::string::npos, "C++ should have 'std::future'");
    CHECK(cppOut.find("fetchData") != std::string::npos, "C++ should have function name");
    PASS();
}

// 7. AwaitExpression dispatch — Python "await expr", C++ "co_await expr"
void test_await_expression_dispatch() {
    TEST(await_expression_dispatch);
    auto aw = std::make_unique<AwaitExpression>("aw1");
    auto call = []{ auto* c = new FunctionCall(); c->id = "fc1"; c->functionName = "getData"; return c; }();
    aw->setChild("expression", call);

    PythonGenerator pyGen;
    CppGenerator cppGen;
    std::string pyOut = pyGen.generate(aw.get());
    std::string cppOut = cppGen.generate(aw.get());
    CHECK(pyOut.find("await") != std::string::npos, "Python should contain 'await'");
    CHECK(cppOut.find("co_await") != std::string::npos, "C++ should contain 'co_await'");
    PASS();
}

// 8. LambdaExpression dispatch — Python "lambda", C++ "[captures](...)"
void test_lambda_dispatch() {
    TEST(lambda_dispatch);
    auto lam = std::make_unique<LambdaExpression>("lam1");
    lam->captureList = {"x", "y"};
    auto param = new Parameter();
    param->id = "p1";
    param->name = "item";
    lam->addChild("parameters", param);

    PythonGenerator pyGen;
    CppGenerator cppGen;
    std::string pyOut = pyGen.generate(lam.get());
    std::string cppOut = cppGen.generate(lam.get());
    CHECK(pyOut.find("lambda") != std::string::npos, "Python should contain 'lambda'");
    CHECK(cppOut.find("[x, y]") != std::string::npos, "C++ should contain captures '[x, y]'");
    CHECK(cppOut.find("item") != std::string::npos, "C++ should contain parameter name");
    PASS();
}

// 9. DecoratorAnnotation dispatch — Python "@name", C++ "// @name"
void test_decorator_dispatch() {
    TEST(decorator_dispatch);
    auto dec = std::make_unique<DecoratorAnnotation>("dec1", "cache");

    PythonGenerator pyGen;
    CppGenerator cppGen;
    std::string pyOut = pyGen.generate(dec.get());
    std::string cppOut = cppGen.generate(dec.get());
    CHECK(pyOut.find("@cache") != std::string::npos, "Python should contain '@cache'");
    CHECK(cppOut.find("@cache") != std::string::npos, "C++ should contain '// @cache'");
    PASS();
}

// 10. ClassDeclaration with methods — full nested dispatch
void test_class_with_methods_dispatch() {
    TEST(class_with_methods_dispatch);
    auto cls = std::make_unique<ClassDeclaration>("cls1", "Shape");
    auto meth = new MethodDeclaration("m1", "area");
    meth->isVirtual = true;
    cls->addChild("methods", meth);

    PythonGenerator pyGen;
    std::string pyOut = pyGen.generate(cls.get());
    CHECK(pyOut.find("class Shape") != std::string::npos, "should contain class name");
    CHECK(pyOut.find("def area") != std::string::npos, "should contain method name");
    PASS();
}

// 11. AsyncFunction with body containing AwaitExpression — nested dispatch
void test_async_with_await_dispatch() {
    TEST(async_with_await_dispatch);
    auto af = std::make_unique<AsyncFunction>("af1", "loadUser");
    auto aw = new AwaitExpression("aw1");
    aw->setChild("expression", []{ auto* c = new FunctionCall(); c->id = "fc1"; c->functionName = "httpGet"; return c; }());
    auto exprStmt = new ExpressionStatement();
    exprStmt->id = "es1";
    exprStmt->setChild("expression", aw);
    af->addChild("body", exprStmt);

    CppGenerator cppGen;
    std::string out = cppGen.generate(af.get());
    CHECK(out.find("std::future") != std::string::npos, "should have std::future");
    CHECK(out.find("loadUser") != std::string::npos, "should have function name");
    PASS();
}

// 12. All 9 new node types roundtrip through JSON and re-dispatch identically
void test_all_new_nodes_json_roundtrip_dispatch() {
    TEST(all_new_nodes_json_roundtrip_dispatch);
    PythonGenerator pyGen;

    // Build one of each type and verify roundtrip preserves dispatch output
    auto cls = std::make_unique<ClassDeclaration>("c1", "Foo");
    auto iface = std::make_unique<InterfaceDeclaration>("i1", "Bar");
    auto meth = std::make_unique<MethodDeclaration>("m1", "baz");
    auto gen = std::make_unique<GenericType>("g1", "Box");
    gen->addChild("typeParameters", new TypeParameter("tp1", "T"));
    auto tp = std::make_unique<TypeParameter>("tp2", "U");
    auto af = std::make_unique<AsyncFunction>("af1", "load");
    auto aw = std::make_unique<AwaitExpression>("aw1");
    aw->setChild("expression", []{ auto* c = new FunctionCall(); c->id = "fc1"; c->functionName = "f"; return c; }());
    auto lam = std::make_unique<LambdaExpression>("l1");
    lam->captureList = {"z"};
    auto dec = std::make_unique<DecoratorAnnotation>("d1", "test");

    struct Case { const char* label; ASTNode* node; };
    Case cases[] = {
        {"ClassDeclaration", cls.get()},
        {"InterfaceDeclaration", iface.get()},
        {"MethodDeclaration", meth.get()},
        {"GenericType", gen.get()},
        {"TypeParameter", tp.get()},
        {"AsyncFunction", af.get()},
        {"AwaitExpression", aw.get()},
        {"LambdaExpression", lam.get()},
        {"DecoratorAnnotation", dec.get()},
    };

    for (auto& c : cases) {
        std::string origOut = pyGen.generate(c.node);
        CHECK(!origOut.empty(), std::string("empty output for ") + c.label);

        json j = toJson(c.node);
        ASTNode* restored = fromJson(j);
        CHECK(restored != nullptr, std::string("null fromJson for ") + c.label);
        CHECK(restored->conceptType == c.label, std::string("wrong type for ") + c.label);

        std::string restoredOut = pyGen.generate(restored);
        CHECK(origOut == restoredOut, std::string("roundtrip output mismatch for ") + c.label);
        delete restored;
    }
    PASS();
}

int main() {
    std::cout << "=== Step 304: Serialization + Dispatch for New Nodes ===\n";
    test_class_python_dispatch();
    test_class_cpp_dispatch();
    test_interface_dispatch();
    test_method_python_dispatch();
    test_generic_type_dispatch();
    test_async_function_dispatch();
    test_await_expression_dispatch();
    test_lambda_dispatch();
    test_decorator_dispatch();
    test_class_with_methods_dispatch();
    test_async_with_await_dispatch();
    test_all_new_nodes_json_roundtrip_dispatch();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
