// Step 306: TypeScript/JavaScript + Rust Parser Deepening (12 tests)
// Tests that JS/TS and Rust parsers correctly produce the new AST node
// types: ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
// AsyncFunction, AwaitExpression, LambdaExpression via tree-sitter parsing.

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
#include <functional>

static int passed = 0, failed = 0;
#define TEST(name) { std::cout << "  " << #name << "... "; }
#define PASS() { std::cout << "PASS\n"; ++passed; }
#define FAIL(msg) { std::cout << "FAIL: " << msg << "\n"; ++failed; }
#define CHECK(cond, msg) if (!(cond)) { FAIL(msg); return; } else {}

// Helper: recursively find a node of a given conceptType
static bool findNodeOfType(ASTNode* node, const std::string& type) {
    if (!node) return false;
    if (node->conceptType == type) return true;
    for (auto* child : node->allChildren()) {
        if (findNodeOfType(child, type)) return true;
    }
    return false;
}

// ---------------------------------------------------------------
// JavaScript/TypeScript tests (1-6)
// ---------------------------------------------------------------

// 1. JS backward compat — simple function still yields Function
void test_js_backward_compat() {
    TEST(js_backward_compat);
    std::string src = "function greet(name) {\n    return name;\n}\n";
    auto mod = TreeSitterParser::parseJavaScript(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");
    CHECK(fns[0]->conceptType == "Function", "expected Function, got " + fns[0]->conceptType);
    auto* fn0 = dynamic_cast<Function*>(fns[0]);
    CHECK(fn0 != nullptr, "dynamic_cast to Function failed");
    CHECK(fn0->name == "greet", "expected name 'greet', got " + fn0->name);
    PASS();
}

// 2. JS class → ClassDeclaration with superclass and MethodDeclaration
void test_js_class() {
    TEST(js_class);
    std::string src =
        "class Animal extends LivingThing {\n"
        "    speak() {\n"
        "        return \"...\";\n"
        "    }\n"
        "}\n";
    auto mod = TreeSitterParser::parseJavaScript(src);
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

// 3. JS async function → AsyncFunction
void test_js_async_function() {
    TEST(js_async_function);
    std::string src = "async function fetchData() {\n    return 42;\n}\n";
    auto mod = TreeSitterParser::parseJavaScript(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");

    auto* af = dynamic_cast<AsyncFunction*>(fns[0]);
    CHECK(af != nullptr, "expected AsyncFunction, got " + fns[0]->conceptType);
    CHECK(af->name == "fetchData", "expected name 'fetchData', got " + af->name);
    CHECK(af->isAsync, "isAsync should be true");
    PASS();
}

// 4. JS await expression → AwaitExpression in body
void test_js_await() {
    TEST(js_await);
    std::string src =
        "async function load() {\n"
        "    const result = await getData();\n"
        "}\n";
    auto mod = TreeSitterParser::parseJavaScript(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    CHECK(findNodeOfType(fns[0], "AwaitExpression"),
          "expected AwaitExpression in function body");
    PASS();
}

// 5. JS arrow function in expression context → LambdaExpression
void test_js_arrow_lambda() {
    TEST(js_arrow_lambda);
    std::string src =
        "function make() {\n"
        "    const fn = (x) => x + 1;\n"
        "}\n";
    auto mod = TreeSitterParser::parseJavaScript(src);
    CHECK(mod != nullptr, "module null");

    // The top-level function "make" should be in functions
    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    // Walk body looking for LambdaExpression
    CHECK(findNodeOfType(fns[0], "LambdaExpression"),
          "expected LambdaExpression from arrow function in body");
    PASS();
}

// 6. TypeScript class — same as JS but through TS parser
void test_ts_class() {
    TEST(ts_class);
    std::string src =
        "class Service {\n"
        "    process(data: string): void {\n"
        "        console.log(data);\n"
        "    }\n"
        "}\n";
    auto mod = TreeSitterParser::parseTypeScript(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration, got " + classes[0]->conceptType);
    CHECK(cls->name == "Service", "expected name 'Service', got " + cls->name);

    auto& methods = cls->getChildren("methods");
    CHECK(!methods.empty(), "expected at least one method");
    auto* meth = dynamic_cast<MethodDeclaration*>(methods[0]);
    CHECK(meth != nullptr, "expected MethodDeclaration");
    CHECK(meth->name == "process", "expected method 'process', got " + meth->name);
    PASS();
}

// ---------------------------------------------------------------
// Rust tests (7-12)
// ---------------------------------------------------------------

// 7. Rust backward compat — function still yields Function
void test_rust_backward_compat() {
    TEST(rust_backward_compat);
    std::string src = "fn greet(name: &str) -> String {\n    name.to_string()\n}\n";
    auto mod = TreeSitterParser::parseRust(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");
    auto* fn0 = dynamic_cast<Function*>(fns[0]);
    CHECK(fn0 != nullptr, "dynamic_cast to Function failed");
    CHECK(fn0->name == "greet", "expected name 'greet', got " + fn0->name);
    PASS();
}

// 8. Rust struct → ClassDeclaration
void test_rust_struct() {
    TEST(rust_struct);
    std::string src =
        "struct Point {\n"
        "    x: f64,\n"
        "    y: f64,\n"
        "}\n";
    auto mod = TreeSitterParser::parseRust(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration, got " + classes[0]->conceptType);
    CHECK(cls->name == "Point", "expected name 'Point', got " + cls->name);
    PASS();
}

// 9. Rust trait → InterfaceDeclaration
void test_rust_trait() {
    TEST(rust_trait);
    std::string src =
        "trait Drawable {\n"
        "    fn draw(&self);\n"
        "}\n";
    auto mod = TreeSitterParser::parseRust(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    bool foundTrait = false;
    for (auto* entry : classes) {
        if (entry->conceptType == "InterfaceDeclaration") {
            auto* iface = dynamic_cast<InterfaceDeclaration*>(entry);
            CHECK(iface != nullptr, "dynamic_cast to InterfaceDeclaration failed");
            CHECK(iface->name == "Drawable", "expected name 'Drawable', got " + iface->name);
            foundTrait = true;
            break;
        }
    }
    CHECK(foundTrait, "expected InterfaceDeclaration in classes");
    PASS();
}

// 10. Rust impl → MethodDeclaration on ClassDeclaration
void test_rust_impl_methods() {
    TEST(rust_impl_methods);
    std::string src =
        "struct Dog {\n"
        "    name: String,\n"
        "}\n"
        "\n"
        "impl Dog {\n"
        "    fn bark(&self) {\n"
        "        println!(\"Woof!\");\n"
        "    }\n"
        "}\n";
    auto mod = TreeSitterParser::parseRust(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected class from struct");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration");
    CHECK(cls->name == "Dog", "expected name 'Dog', got " + cls->name);

    auto& methods = cls->getChildren("methods");
    CHECK(!methods.empty(), "expected at least one method from impl");

    auto* meth = dynamic_cast<MethodDeclaration*>(methods[0]);
    CHECK(meth != nullptr, "expected MethodDeclaration");
    CHECK(meth->name == "bark", "expected method 'bark', got " + meth->name);
    CHECK(meth->className == "Dog", "expected className 'Dog', got " + meth->className);
    PASS();
}

// 11. Rust async fn → AsyncFunction
void test_rust_async_function() {
    TEST(rust_async_function);
    std::string src = "async fn fetch_data() -> String {\n    String::new()\n}\n";
    auto mod = TreeSitterParser::parseRust(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");

    auto* af = dynamic_cast<AsyncFunction*>(fns[0]);
    CHECK(af != nullptr, "expected AsyncFunction, got " + fns[0]->conceptType);
    CHECK(af->name == "fetch_data", "expected name 'fetch_data', got " + af->name);
    CHECK(af->isAsync, "isAsync should be true");
    PASS();
}

// 12. Rust closure → LambdaExpression
void test_rust_closure() {
    TEST(rust_closure);
    std::string src =
        "fn make() {\n"
        "    let add = |x, y| x + y;\n"
        "}\n";
    auto mod = TreeSitterParser::parseRust(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    CHECK(findNodeOfType(fns[0], "LambdaExpression"),
          "expected LambdaExpression from closure in function body");
    PASS();
}

int main() {
    std::cout << "=== Step 306: TypeScript/JavaScript + Rust Parser Deepening ===\n";
    test_js_backward_compat();
    test_js_class();
    test_js_async_function();
    test_js_await();
    test_js_arrow_lambda();
    test_ts_class();
    test_rust_backward_compat();
    test_rust_struct();
    test_rust_trait();
    test_rust_impl_methods();
    test_rust_async_function();
    test_rust_closure();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
