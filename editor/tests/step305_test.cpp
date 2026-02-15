// Step 305: Python + Java Parser Deepening (12 tests)
// Tests that PythonParser and JavaParser correctly produce the new AST node
// types: ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
// AsyncFunction, AwaitExpression, LambdaExpression, DecoratorAnnotation,
// and TypeParameter via tree-sitter parsing of real source strings.

#include "ast/Parser.h"
#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/AsyncNodes.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Parameter.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <functional>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// ---------------------------------------------------------------
// Python tests (1-6)
// ---------------------------------------------------------------

// 1. Python backward compat — simple function still yields Function
void test_python_backward_compat() {
    TEST(python_backward_compat);
    std::string src = "def greet(name):\n    return name\n";
    auto mod = TreeSitterParser::parsePython(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");
    CHECK(fns[0]->conceptType == "Function", "expected Function, got " + fns[0]->conceptType);
    auto* fn0 = dynamic_cast<Function*>(fns[0]);
    CHECK(fn0 != nullptr, "dynamic_cast to Function failed");
    CHECK(fn0->name == "greet", "expected name 'greet', got " + fn0->name);
    PASS();
}

// 2. Python class — class_definition yields ClassDeclaration
void test_python_class() {
    TEST(python_class);
    std::string src =
        "class Animal(LivingThing):\n"
        "    def speak(self):\n"
        "        return \"...\"\n";
    auto mod = TreeSitterParser::parsePython(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class");
    CHECK(classes[0]->conceptType == "ClassDeclaration",
          "expected ClassDeclaration, got " + classes[0]->conceptType);

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "dynamic_cast to ClassDeclaration failed");
    CHECK(cls->name == "Animal", "expected name 'Animal', got " + cls->name);
    CHECK(cls->superClass == "LivingThing",
          "expected superClass 'LivingThing', got " + cls->superClass);

    auto& methods = cls->getChildren("methods");
    CHECK(!methods.empty(), "expected at least one method");
    auto* meth = dynamic_cast<MethodDeclaration*>(methods[0]);
    CHECK(meth != nullptr, "dynamic_cast to MethodDeclaration failed");
    CHECK(meth->name == "speak", "expected method 'speak', got " + meth->name);
    CHECK(meth->className == "Animal", "expected className 'Animal', got " + meth->className);
    PASS();
}

// 3. Python async function — async def yields AsyncFunction
void test_python_async_function() {
    TEST(python_async_function);
    std::string src = "async def fetch_data():\n    pass\n";
    auto mod = TreeSitterParser::parsePython(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");

    auto* af = dynamic_cast<AsyncFunction*>(fns[0]);
    CHECK(af != nullptr, "expected AsyncFunction, dynamic_cast failed (got " + fns[0]->conceptType + ")");
    CHECK(af->name == "fetch_data", "expected name 'fetch_data', got " + af->name);
    CHECK(af->isAsync, "isAsync should be true");
    PASS();
}

// 4. Python await — await expression in body yields AwaitExpression
void test_python_await() {
    TEST(python_await);
    std::string src =
        "async def load():\n"
        "    result = await get_data()\n";
    auto mod = TreeSitterParser::parsePython(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    // The body should contain a statement with an await expression somewhere
    auto& body = fns[0]->getChildren("body");
    CHECK(!body.empty(), "expected body statements");

    // Walk body looking for an AwaitExpression
    bool foundAwait = false;
    std::function<void(ASTNode*)> findAwait = [&](ASTNode* node) {
        if (!node) return;
        if (node->conceptType == "AwaitExpression") { foundAwait = true; return; }
        for (auto* child : node->allChildren()) findAwait(child);
    };
    for (auto* stmt : body) findAwait(stmt);

    CHECK(foundAwait, "expected AwaitExpression in function body");
    PASS();
}

// 5. Python lambda — lambda expression yields LambdaExpression
void test_python_lambda() {
    TEST(python_lambda);
    std::string src =
        "def make():\n"
        "    return lambda x: x + 1\n";
    auto mod = TreeSitterParser::parsePython(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    // Walk body looking for LambdaExpression
    bool foundLambda = false;
    std::string lambdaType;
    std::function<void(ASTNode*)> findLambda = [&](ASTNode* node) {
        if (!node) return;
        if (node->conceptType == "LambdaExpression") {
            foundLambda = true;
            lambdaType = node->conceptType;
            return;
        }
        for (auto* child : node->allChildren()) findLambda(child);
    };
    auto& body = fns[0]->getChildren("body");
    for (auto* stmt : body) findLambda(stmt);

    CHECK(foundLambda, "expected LambdaExpression in function body");
    PASS();
}

// 6. Python decorator — @decorator yields DecoratorAnnotation
void test_python_decorator() {
    TEST(python_decorator);
    std::string src =
        "@cache\n"
        "def expensive():\n"
        "    return 42\n";
    auto mod = TreeSitterParser::parsePython(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    // The function should have a DecoratorAnnotation in annotations
    auto& annos = fns[0]->getChildren("annotations");
    bool foundDecorator = false;
    for (auto* anno : annos) {
        if (anno->conceptType == "DecoratorAnnotation") {
            auto* dec = dynamic_cast<DecoratorAnnotation*>(anno);
            CHECK(dec != nullptr, "dynamic_cast to DecoratorAnnotation failed");
            CHECK(dec->name == "cache", "expected decorator 'cache', got " + dec->name);
            foundDecorator = true;
            break;
        }
    }
    CHECK(foundDecorator, "expected DecoratorAnnotation on function");
    PASS();
}

// ---------------------------------------------------------------
// Java tests (7-12)
// ---------------------------------------------------------------

// 7. Java backward compat — method still appears as Function in module.functions
void test_java_backward_compat() {
    TEST(java_backward_compat);
    std::string src =
        "class Dog {\n"
        "    void bark() { }\n"
        "}\n";
    auto mod = TreeSitterParser::parseJava(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function (backward compat)");

    bool found = false;
    for (auto* fn : fns) {
        auto* func = dynamic_cast<Function*>(fn);
        if (func && func->name.find("bark") != std::string::npos) { found = true; break; }
    }
    CHECK(found, "expected a Function with 'bark' in name");
    PASS();
}

// 8. Java ClassDeclaration — class parsed into classes child
void test_java_class_declaration() {
    TEST(java_class_declaration);
    std::string src =
        "class Dog extends Animal {\n"
        "    void bark() { }\n"
        "}\n";
    auto mod = TreeSitterParser::parseJava(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "dynamic_cast to ClassDeclaration failed (got " + classes[0]->conceptType + ")");
    CHECK(cls->name == "Dog", "expected name 'Dog', got " + cls->name);
    CHECK(cls->superClass == "Animal", "expected superClass 'Animal', got " + cls->superClass);
    PASS();
}

// 9. Java InterfaceDeclaration — interface parsed into classes child
void test_java_interface_declaration() {
    TEST(java_interface_declaration);
    std::string src =
        "interface Drawable {\n"
        "    void draw();\n"
        "}\n";
    auto mod = TreeSitterParser::parseJava(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class/interface entry");

    bool foundIface = false;
    for (auto* entry : classes) {
        if (entry->conceptType == "InterfaceDeclaration") {
            auto* iface = dynamic_cast<InterfaceDeclaration*>(entry);
            CHECK(iface != nullptr, "dynamic_cast to InterfaceDeclaration failed");
            CHECK(iface->name == "Drawable", "expected name 'Drawable', got " + iface->name);
            foundIface = true;
            break;
        }
    }
    CHECK(foundIface, "expected InterfaceDeclaration in classes");
    PASS();
}

// 10. Java MethodDeclaration — method as child of ClassDeclaration
void test_java_method_declaration() {
    TEST(java_method_declaration);
    std::string src =
        "class Service {\n"
        "    public static void process() { }\n"
        "}\n";
    auto mod = TreeSitterParser::parseJava(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration");

    auto& methods = cls->getChildren("methods");
    CHECK(!methods.empty(), "expected at least one method in class");

    auto* meth = dynamic_cast<MethodDeclaration*>(methods[0]);
    CHECK(meth != nullptr, "dynamic_cast to MethodDeclaration failed");
    CHECK(meth->name == "process", "expected method 'process', got " + meth->name);
    CHECK(meth->className == "Service", "expected className 'Service', got " + meth->className);
    CHECK(meth->isStatic, "expected isStatic=true for 'public static'");
    CHECK(meth->visibility == "public", "expected visibility 'public', got " + meth->visibility);
    PASS();
}

// 11. Java lambda — lambda expression yields LambdaExpression
void test_java_lambda() {
    TEST(java_lambda);
    std::string src =
        "class App {\n"
        "    void run() {\n"
        "        Runnable r = () -> { };\n"
        "    }\n"
        "}\n";
    auto mod = TreeSitterParser::parseJava(src);
    CHECK(mod != nullptr, "module null");

    // Walk entire AST looking for LambdaExpression
    bool foundLambda = false;
    std::function<void(ASTNode*)> findLambda = [&](ASTNode* node) {
        if (!node) return;
        if (node->conceptType == "LambdaExpression") { foundLambda = true; return; }
        for (auto* child : node->allChildren()) findLambda(child);
    };
    findLambda(mod.get());

    CHECK(foundLambda, "expected LambdaExpression somewhere in AST");
    PASS();
}

// 12. Java generic class — type parameters yield TypeParameter children
void test_java_generic_class() {
    TEST(java_generic_class);
    std::string src =
        "class Box<T extends Comparable> {\n"
        "    T value;\n"
        "}\n";
    auto mod = TreeSitterParser::parseJava(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration");
    CHECK(cls->name == "Box", "expected name 'Box', got " + cls->name);

    auto& typeParams = cls->getChildren("typeParameters");
    CHECK(!typeParams.empty(), "expected at least one TypeParameter");

    auto* tp = dynamic_cast<TypeParameter*>(typeParams[0]);
    CHECK(tp != nullptr, "dynamic_cast to TypeParameter failed");
    CHECK(tp->name == "T", "expected type param 'T', got " + tp->name);
    CHECK(tp->constraint == "Comparable",
          "expected constraint 'Comparable', got " + tp->constraint);
    PASS();
}

int main() {
    std::cout << "=== Step 305: Python + Java Parser Deepening ===\n";
    test_python_backward_compat();
    test_python_class();
    test_python_async_function();
    test_python_await();
    test_python_lambda();
    test_python_decorator();
    test_java_backward_compat();
    test_java_class_declaration();
    test_java_interface_declaration();
    test_java_method_declaration();
    test_java_lambda();
    test_java_generic_class();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
