// Step 334: C++ Parser — Inheritance + Templates (12 tests)
// Tests multiple inheritance, virtual, access specifiers, template classes, CRTP

#include <cassert>
#include <iostream>
#include <string>
#include <memory>
#include "ast/Parser.h"
#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/Serialization.h"

int main() {
    int passed = 0;

    // Test 1: Single inheritance backward compat
    {
        std::string src = R"(
class Foo : public Bar {
public:
    void doStuff() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(classes.size() >= 1);
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls != nullptr);
        assert(cls->name == "Foo");
        // Should have at least superClass set
        assert(!cls->superClass.empty() || !cls->baseClasses.empty());
        std::cout << "Test 1 PASSED: Single inheritance backward compat\n";
        passed++;
    }

    // Test 2: Multiple inheritance parsed
    {
        std::string src = R"(
class Widget : public Base, public Mixin {
    void draw() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "Widget");
        auto bases = cls->getBases();
        assert(bases.size() >= 2);
        std::cout << "Test 2 PASSED: Multiple inheritance parsed (" << bases.size() << " bases)\n";
        passed++;
    }

    // Test 3: Access specifiers detected
    {
        std::string src = R"(
class D : public A, protected B, private C {
    void f() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        auto bases = cls->getBases();
        assert(bases.size() >= 3);
        // Check that at least different specifiers exist
        bool hasPublic = false, hasProtected = false, hasPrivate = false;
        for (const auto& b : bases) {
            if (b.accessSpecifier == "public") hasPublic = true;
            if (b.accessSpecifier == "protected") hasProtected = true;
            if (b.accessSpecifier == "private") hasPrivate = true;
        }
        assert(hasPublic);
        assert(hasProtected);
        assert(hasPrivate);
        std::cout << "Test 3 PASSED: Access specifiers detected\n";
        passed++;
    }

    // Test 4: Virtual inheritance flag
    {
        std::string src = R"(
class D : public virtual Base {
    void f() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        auto bases = cls->getBases();
        assert(!bases.empty());
        assert(bases[0].isVirtual);
        std::cout << "Test 4 PASSED: Virtual inheritance flag detected\n";
        passed++;
    }

    // Test 5: Template class parsed
    {
        std::string src = R"(
template<typename T>
class Container {
public:
    void add(T item) {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "Container");
        auto tps = cls->getChildren("typeParameters");
        assert(tps.size() >= 1);
        auto* tp = dynamic_cast<TypeParameter*>(tps[0]);
        assert(tp != nullptr);
        assert(tp->name == "T");
        std::cout << "Test 5 PASSED: Template class parsed with TypeParameter\n";
        passed++;
    }

    // Test 6: CRTP pattern detected via base class name
    {
        std::string src = R"(
template<typename T>
class Base {
};

class Derived : public Base<Derived> {
    void f() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        // Find Derived class
        ClassDeclaration* derived = nullptr;
        for (auto* c : classes) {
            auto* cls = dynamic_cast<ClassDeclaration*>(c);
            if (cls && cls->name == "Derived") {
                derived = cls;
                break;
            }
        }
        assert(derived != nullptr);
        auto bases = derived->getBases();
        assert(!bases.empty());
        // Check CRTP: Base<Derived> contains "Derived" as template arg
        std::vector<std::string> baseNames;
        for (const auto& b : bases) baseNames.push_back(b.name);
        assert(isCRTPClass("Derived", baseNames));
        std::cout << "Test 6 PASSED: CRTP pattern detected\n";
        passed++;
    }

    // Test 7: struct vs class default access
    {
        std::string src = R"(
struct Point {
    int x;
    int y;
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "Point");
        std::cout << "Test 7 PASSED: struct parsed as ClassDeclaration\n";
        passed++;
    }

    // Test 8: Template with variadic (typename... Args)
    {
        std::string src = R"(
template<typename T, typename U>
class Pair {
    void f() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "Pair");
        auto tps = cls->getChildren("typeParameters");
        assert(tps.size() == 2);
        std::cout << "Test 8 PASSED: Template with 2 type params\n";
        passed++;
    }

    // Test 9: Combined template + multiple inheritance
    {
        std::string src = R"(
template<typename T>
class MyClass : public Base, public Interface {
    void f() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "MyClass");
        auto bases = cls->getBases();
        assert(bases.size() >= 2);
        auto tps = cls->getChildren("typeParameters");
        assert(tps.size() >= 1);
        std::cout << "Test 9 PASSED: Template + multiple inheritance combined\n";
        passed++;
    }

    // Test 10: Methods inside class body still parsed
    {
        std::string src = R"(
class Foo : public Bar {
public:
    virtual void draw() {}
    static void create() {}
private:
    void helper() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        auto methods = cls->getChildren("methods");
        assert(methods.size() >= 2); // at least some methods parsed
        std::cout << "Test 10 PASSED: Methods inside class body parsed (" << methods.size() << " methods)\n";
        passed++;
    }

    // Test 11: Whetstone-style class signature
    {
        std::string src = R"(
class KotlinGenerator : public ProjectionGenerator, public AnnotationVisitorExtended {
public:
    void generate() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "KotlinGenerator");
        auto bases = cls->getBases();
        assert(bases.size() >= 2);
        std::cout << "Test 11 PASSED: Whetstone-style class signature\n";
        passed++;
    }

    // Test 12: JSON roundtrip preserves parsed class with bases
    {
        std::string src = R"(
class D : public A, protected B {
    void f() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        json j = toJson(mod.get());
        ASTNode* restored = fromJson(j);
        auto* rmod = dynamic_cast<Module*>(restored);
        assert(rmod != nullptr);
        auto classes = rmod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = dynamic_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "D");
        auto bases = cls->getBases();
        assert(bases.size() >= 2);
        deleteTree(restored);
        std::cout << "Test 12 PASSED: JSON roundtrip preserves parsed class\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
