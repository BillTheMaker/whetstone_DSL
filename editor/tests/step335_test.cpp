// Step 335: C++ Generator — Inheritance + Templates (12 tests)
// Tests C++ output for multiple inheritance, virtual, template classes,
// cross-language adaptation

#include <cassert>
#include <iostream>
#include <string>
#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/AsyncNodes.h"
#include "ast/CppGenerator.h"
#include "ast/PythonGenerator.h"
#include "ast/JavaGenerator.h"
#include "ast/RustGenerator.h"
#include "ast/Serialization.h"

int main() {
    int passed = 0;

    // Test 1: C++ multiple inheritance output
    {
        auto* cls = new ClassDeclaration("cls1", "Widget");
        cls->addBase("Base", "public");
        cls->addBase("Mixin", "public");
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("class Widget") != std::string::npos);
        assert(out.find("public Base") != std::string::npos);
        assert(out.find("public Mixin") != std::string::npos);
        delete cls;
        std::cout << "Test 1 PASSED: C++ multiple inheritance output\n";
        passed++;
    }

    // Test 2: Access specifier output
    {
        auto* cls = new ClassDeclaration("cls2", "D");
        cls->addBase("A", "public");
        cls->addBase("B", "protected");
        cls->addBase("C", "private");
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("public A") != std::string::npos);
        assert(out.find("protected B") != std::string::npos);
        assert(out.find("private C") != std::string::npos);
        delete cls;
        std::cout << "Test 2 PASSED: Access specifier output\n";
        passed++;
    }

    // Test 3: Virtual keyword present
    {
        auto* cls = new ClassDeclaration("cls3", "D");
        cls->addBase("Base", "public", true);
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("virtual") != std::string::npos);
        assert(out.find("Base") != std::string::npos);
        delete cls;
        std::cout << "Test 3 PASSED: Virtual keyword present\n";
        passed++;
    }

    // Test 4: Template class output
    {
        auto* cls = new ClassDeclaration("cls4", "Container");
        auto* gt = new GenericType("gt1", "Container");
        gt->isClassTemplate = true;
        auto* tp = new TypeParameter("tp1", "T");
        gt->addChild("typeParameters", tp);
        cls->addChild("typeParameters", gt);
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("template") != std::string::npos);
        assert(out.find("typename T") != std::string::npos || out.find("T") != std::string::npos);
        assert(out.find("class Container") != std::string::npos);
        delete cls;
        std::cout << "Test 4 PASSED: Template class output\n";
        passed++;
    }

    // Test 5: CRTP output
    {
        auto* cls = new ClassDeclaration("cls5", "Derived");
        cls->addBase("Base<Derived>", "public");
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("Base<Derived>") != std::string::npos);
        delete cls;
        std::cout << "Test 5 PASSED: CRTP output\n";
        passed++;
    }

    // Test 6: Cross-language C++ → Python (multiple bases → direct multiple inheritance)
    {
        auto* cls = new ClassDeclaration("cls6", "D");
        cls->addBase("A", "public");
        cls->addBase("B", "public");
        PythonGenerator gen;
        std::string out = gen.generate(cls);
        // Python should show both bases
        assert(out.find("class D") != std::string::npos);
        assert(out.find("A") != std::string::npos);
        assert(out.find("B") != std::string::npos);
        delete cls;
        std::cout << "Test 6 PASSED: C++ → Python multiple inheritance\n";
        passed++;
    }

    // Test 7: Cross-language C++ → Java (multiple bases → extends + implements)
    {
        auto* cls = new ClassDeclaration("cls7", "Widget");
        cls->addBase("Base", "public");
        cls->addBase("Drawable", "public");
        JavaGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("class Widget") != std::string::npos);
        // Java should at least show first as extends
        assert(out.find("Base") != std::string::npos);
        delete cls;
        std::cout << "Test 7 PASSED: C++ → Java inheritance output\n";
        passed++;
    }

    // Test 8: Cross-language C++ → Rust (no inheritance, traits)
    {
        auto* cls = new ClassDeclaration("cls8", "Widget");
        cls->addBase("Base", "public");
        RustGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("Widget") != std::string::npos);
        // Rust uses struct, not class inheritance
        assert(!out.empty());
        delete cls;
        std::cout << "Test 8 PASSED: C++ → Rust struct output\n";
        passed++;
    }

    // Test 9: Template class with variadic
    {
        auto* cls = new ClassDeclaration("cls9", "Tuple");
        auto* gt = new GenericType("gt2", "Tuple");
        gt->isClassTemplate = true;
        auto* tp1 = new TypeParameter("tp2", "Head");
        auto* tp2 = new TypeParameter("tp3", "Tail");
        tp2->isVariadic = true;
        gt->addChild("typeParameters", tp1);
        gt->addChild("typeParameters", tp2);
        cls->addChild("typeParameters", gt);
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("template") != std::string::npos);
        assert(out.find("Tail") != std::string::npos);
        assert(out.find("...") != std::string::npos); // variadic pack
        delete cls;
        std::cout << "Test 9 PASSED: Variadic template class output\n";
        passed++;
    }

    // Test 10: Combined template + multiple inheritance output
    {
        auto* cls = new ClassDeclaration("cls10", "MyGen");
        cls->addBase("ProjectionGenerator", "public");
        cls->addBase("SemannoAnnotationImpl<MyGen>", "public");
        auto* gt = new GenericType("gt3", "MyGen");
        gt->isClassTemplate = true;
        auto* tp = new TypeParameter("tp4", "T");
        gt->addChild("typeParameters", tp);
        cls->addChild("typeParameters", gt);
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("template") != std::string::npos);
        assert(out.find("class MyGen") != std::string::npos);
        assert(out.find("ProjectionGenerator") != std::string::npos);
        assert(out.find("SemannoAnnotationImpl<MyGen>") != std::string::npos);
        delete cls;
        std::cout << "Test 10 PASSED: Combined template + multiple inheritance\n";
        passed++;
    }

    // Test 11: Method with visibility in class
    {
        auto* cls = new ClassDeclaration("cls11", "Foo");
        cls->addBase("Bar", "public");
        auto* meth = new MethodDeclaration("m1", "doStuff");
        meth->className = "Foo";
        meth->isVirtual = true;
        cls->addChild("methods", meth);
        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("virtual") != std::string::npos);
        assert(out.find("doStuff") != std::string::npos);
        delete cls;
        std::cout << "Test 11 PASSED: Method with visibility in class\n";
        passed++;
    }

    // Test 12: Roundtrip: AST → generate → verify structure
    {
        auto* cls = new ClassDeclaration("cls12", "Stack");
        cls->addBase("Container", "public");
        cls->addBase("Serializable", "public");
        cls->isAbstract = true;
        auto* field = new Variable("v1", "size_");
        cls->addChild("fields", field);

        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("class Stack") != std::string::npos);
        assert(out.find("Container") != std::string::npos);
        assert(out.find("Serializable") != std::string::npos);
        assert(out.find("size_") != std::string::npos);
        assert(out.length() > 20);
        delete cls;
        std::cout << "Test 12 PASSED: Full class generation roundtrip\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
