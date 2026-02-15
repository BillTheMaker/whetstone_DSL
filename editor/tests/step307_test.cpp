// Step 307: Go + C++ + Elisp Parser Deepening (12 tests)
// Tests that Go, C++, and Elisp parsers correctly produce the new AST node
// types: ClassDeclaration, InterfaceDeclaration, MethodDeclaration,
// GenericType, LambdaExpression via tree-sitter parsing.

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
// Go tests (1-5)
// ---------------------------------------------------------------

// 1. Go backward compat — simple function still yields Function
void test_go_backward_compat() {
    TEST(go_backward_compat);
    std::string src = "package main\n\nfunc greet(name string) string {\n    return name\n}\n";
    auto mod = TreeSitterParser::parseGo(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");
    auto* fn0 = dynamic_cast<Function*>(fns[0]);
    CHECK(fn0 != nullptr, "dynamic_cast to Function failed");
    CHECK(fn0->name == "greet", "expected name 'greet', got " + fn0->name);
    PASS();
}

// 2. Go struct → ClassDeclaration
void test_go_struct() {
    TEST(go_struct);
    std::string src =
        "package main\n\n"
        "type Point struct {\n"
        "    X float64\n"
        "    Y float64\n"
        "}\n";
    auto mod = TreeSitterParser::parseGo(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration, got " + classes[0]->conceptType);
    CHECK(cls->name == "Point", "expected name 'Point', got " + cls->name);
    PASS();
}

// 3. Go interface → InterfaceDeclaration
void test_go_interface() {
    TEST(go_interface);
    std::string src =
        "package main\n\n"
        "type Stringer interface {\n"
        "    String() string\n"
        "}\n";
    auto mod = TreeSitterParser::parseGo(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    bool foundInterface = false;
    for (auto* entry : classes) {
        if (entry->conceptType == "InterfaceDeclaration") {
            auto* iface = dynamic_cast<InterfaceDeclaration*>(entry);
            CHECK(iface != nullptr, "dynamic_cast to InterfaceDeclaration failed");
            CHECK(iface->name == "Stringer", "expected name 'Stringer', got " + iface->name);
            foundInterface = true;
            break;
        }
    }
    CHECK(foundInterface, "expected InterfaceDeclaration in classes");
    PASS();
}

// 4. Go method receiver → MethodDeclaration on ClassDeclaration
void test_go_method_receiver() {
    TEST(go_method_receiver);
    std::string src =
        "package main\n\n"
        "type Dog struct {\n"
        "    Name string\n"
        "}\n\n"
        "func (d *Dog) Bark() string {\n"
        "    return \"Woof!\"\n"
        "}\n";
    auto mod = TreeSitterParser::parseGo(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected class from struct");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration");
    CHECK(cls->name == "Dog", "expected name 'Dog', got " + cls->name);

    auto& methods = cls->getChildren("methods");
    CHECK(!methods.empty(), "expected at least one method from receiver");

    auto* meth = dynamic_cast<MethodDeclaration*>(methods[0]);
    CHECK(meth != nullptr, "expected MethodDeclaration");
    CHECK(meth->name == "Bark", "expected method 'Bark', got " + meth->name);
    CHECK(meth->className == "Dog", "expected className 'Dog', got " + meth->className);
    PASS();
}

// 5. Go func literal → LambdaExpression
void test_go_func_literal() {
    TEST(go_func_literal);
    std::string src =
        "package main\n\n"
        "func make() {\n"
        "    fn := func(x int) int { return x + 1 }\n"
        "}\n";
    auto mod = TreeSitterParser::parseGo(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    CHECK(findNodeOfType(fns[0], "LambdaExpression"),
          "expected LambdaExpression from func literal in body");
    PASS();
}

// ---------------------------------------------------------------
// C++ tests (6-10)
// ---------------------------------------------------------------

// 6. C++ backward compat — simple function still yields Function
void test_cpp_backward_compat() {
    TEST(cpp_backward_compat);
    std::string src = "int add(int a, int b) {\n    return a + b;\n}\n";
    auto mod = TreeSitterParser::parseCpp(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");
    auto* fn0 = dynamic_cast<Function*>(fns[0]);
    CHECK(fn0 != nullptr, "dynamic_cast to Function failed");
    CHECK(fn0->name == "add", "expected name 'add', got " + fn0->name);
    PASS();
}

// 7. C++ class → ClassDeclaration with MethodDeclaration
void test_cpp_class() {
    TEST(cpp_class);
    std::string src =
        "class Animal {\n"
        "public:\n"
        "    void speak() {\n"
        "        return;\n"
        "    }\n"
        "};\n";
    auto mod = TreeSitterParser::parseCpp(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected at least one class");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration, got " + classes[0]->conceptType);
    CHECK(cls->name == "Animal", "expected name 'Animal', got " + cls->name);

    auto& methods = cls->getChildren("methods");
    CHECK(!methods.empty(), "expected at least one method");
    auto* meth = dynamic_cast<MethodDeclaration*>(methods[0]);
    CHECK(meth != nullptr, "expected MethodDeclaration");
    CHECK(meth->name == "speak", "expected method 'speak', got " + meth->name);
    CHECK(meth->className == "Animal", "expected className 'Animal', got " + meth->className);
    PASS();
}

// 8. C++ struct → ClassDeclaration
void test_cpp_struct() {
    TEST(cpp_struct);
    std::string src =
        "struct Point {\n"
        "    double x;\n"
        "    double y;\n"
        "};\n";
    auto mod = TreeSitterParser::parseCpp(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected class from struct");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration, got " + classes[0]->conceptType);
    CHECK(cls->name == "Point", "expected name 'Point', got " + cls->name);
    PASS();
}

// 9. C++ template class → GenericType on ClassDeclaration
void test_cpp_template() {
    TEST(cpp_template);
    std::string src =
        "template<typename T>\n"
        "class Container {\n"
        "public:\n"
        "    T value;\n"
        "};\n";
    auto mod = TreeSitterParser::parseCpp(src);
    CHECK(mod != nullptr, "module null");

    auto& classes = mod->getChildren("classes");
    CHECK(!classes.empty(), "expected class from template");

    auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
    CHECK(cls != nullptr, "expected ClassDeclaration");
    CHECK(cls->name == "Container", "expected name 'Container', got " + cls->name);

    // Should have a GenericType annotation or child
    CHECK(findNodeOfType(cls, "TypeParameter"),
          "expected TypeParameter from template parameter");
    PASS();
}

// 10. C++ lambda → LambdaExpression
void test_cpp_lambda() {
    TEST(cpp_lambda);
    std::string src =
        "void make() {\n"
        "    auto fn = [](int x) { return x + 1; };\n"
        "}\n";
    auto mod = TreeSitterParser::parseCpp(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    CHECK(findNodeOfType(fns[0], "LambdaExpression"),
          "expected LambdaExpression from C++ lambda in body");
    PASS();
}

// ---------------------------------------------------------------
// Elisp tests (11-12)
// ---------------------------------------------------------------

// 11. Elisp backward compat — defun still yields Function
void test_elisp_backward_compat() {
    TEST(elisp_backward_compat);
    std::string src = "(defun greet (name)\n  (message name))\n";
    auto mod = TreeSitterParser::parseElisp(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected at least one function");
    auto* fn0 = dynamic_cast<Function*>(fns[0]);
    CHECK(fn0 != nullptr, "dynamic_cast to Function failed");
    CHECK(fn0->name == "greet", "expected name 'greet', got " + fn0->name);
    PASS();
}

// 12. Elisp lambda → LambdaExpression
void test_elisp_lambda() {
    TEST(elisp_lambda);
    std::string src =
        "(defun make ()\n"
        "  (lambda (x) (+ x 1)))\n";
    auto mod = TreeSitterParser::parseElisp(src);
    CHECK(mod != nullptr, "module null");

    auto& fns = mod->getChildren("functions");
    CHECK(!fns.empty(), "expected function");

    CHECK(findNodeOfType(fns[0], "LambdaExpression"),
          "expected LambdaExpression from (lambda ...) in function body");
    PASS();
}

int main() {
    std::cout << "=== Step 307: Go + C++ + Elisp Parser Deepening ===\n";
    test_go_backward_compat();
    test_go_struct();
    test_go_interface();
    test_go_method_receiver();
    test_go_func_literal();
    test_cpp_backward_compat();
    test_cpp_class();
    test_cpp_struct();
    test_cpp_template();
    test_cpp_lambda();
    test_elisp_backward_compat();
    test_elisp_lambda();
    std::cout << "\nResults: " << passed << "/" << (passed + failed) << " passed\n";
    return failed > 0 ? 1 : 0;
}
