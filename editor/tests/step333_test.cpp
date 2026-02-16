// Step 333: Template Class Declarations — 12 tests
// Tests isClassTemplate, isVariadic, isCRTP, JSON roundtrip, CompactAST

#include <cassert>
#include <iostream>
#include <string>
#include "ast/ClassDeclaration.h"
#include "ast/GenericType.h"
#include "ast/Serialization.h"
#include "CompactAST.h"

int main() {
    int passed = 0;

    // Test 1: GenericType isClassTemplate default is false
    {
        GenericType gt("gt1", "List");
        assert(!gt.isClassTemplate);
        assert(gt.baseName == "List");
        std::cout << "Test 1 PASSED: GenericType default isClassTemplate=false\n";
        passed++;
    }

    // Test 2: GenericType isClassTemplate=true for template class
    {
        GenericType gt("gt2", "Container");
        gt.isClassTemplate = true;
        assert(gt.isClassTemplate);
        std::cout << "Test 2 PASSED: GenericType isClassTemplate=true\n";
        passed++;
    }

    // Test 3: TypeParameter isVariadic default is false
    {
        TypeParameter tp("tp1", "T");
        assert(!tp.isVariadic);
        assert(tp.name == "T");
        std::cout << "Test 3 PASSED: TypeParameter default isVariadic=false\n";
        passed++;
    }

    // Test 4: TypeParameter isVariadic=true for variadic pack
    {
        TypeParameter tp("tp2", "Args");
        tp.isVariadic = true;
        assert(tp.isVariadic);
        std::cout << "Test 4 PASSED: TypeParameter isVariadic=true\n";
        passed++;
    }

    // Test 5: GenericType isClassTemplate JSON roundtrip
    {
        GenericType gt("gt3", "MyClass");
        gt.isClassTemplate = true;
        auto* tp = new TypeParameter("tp3", "T");
        gt.addChild("typeParameters", tp);

        json j = toJson(&gt);
        assert(j["properties"]["isClassTemplate"] == true);
        assert(j["properties"]["baseName"] == "MyClass");

        ASTNode* restored = fromJson(j);
        auto* rgt = dynamic_cast<GenericType*>(restored);
        assert(rgt != nullptr);
        assert(rgt->isClassTemplate == true);
        assert(rgt->baseName == "MyClass");
        auto tps = rgt->getChildren("typeParameters");
        assert(tps.size() == 1);
        deleteTree(restored);
        std::cout << "Test 5 PASSED: GenericType isClassTemplate JSON roundtrip\n";
        passed++;
    }

    // Test 6: TypeParameter isVariadic JSON roundtrip
    {
        TypeParameter tp("tp4", "Args");
        tp.isVariadic = true;
        tp.constraint = "Printable";

        json j = toJson(&tp);
        assert(j["properties"]["isVariadic"] == true);
        assert(j["properties"]["name"] == "Args");
        assert(j["properties"]["constraint"] == "Printable");

        ASTNode* restored = fromJson(j);
        auto* rtp = dynamic_cast<TypeParameter*>(restored);
        assert(rtp != nullptr);
        assert(rtp->isVariadic == true);
        assert(rtp->name == "Args");
        assert(rtp->constraint == "Printable");
        deleteTree(restored);
        std::cout << "Test 6 PASSED: TypeParameter isVariadic JSON roundtrip\n";
        passed++;
    }

    // Test 7: isCRTPClass detects CRTP pattern
    {
        // class Foo : public Base<Foo>
        std::string className = "Foo";
        std::vector<std::string> bases = {"Base<Foo>"};
        assert(isCRTPClass(className, bases));
        std::cout << "Test 7 PASSED: isCRTPClass detects CRTP\n";
        passed++;
    }

    // Test 8: isCRTPClass rejects non-CRTP
    {
        // class Foo : public Base<Bar>
        std::string className = "Foo";
        std::vector<std::string> bases = {"Base<Bar>"};
        assert(!isCRTPClass(className, bases));
        std::cout << "Test 8 PASSED: isCRTPClass rejects non-CRTP\n";
        passed++;
    }

    // Test 9: isCRTPClass with multiple bases, one CRTP
    {
        std::string className = "KotlinGenerator";
        std::vector<std::string> bases = {
            "ProjectionGenerator",
            "SemannoAnnotationImpl<KotlinGenerator>"
        };
        assert(isCRTPClass(className, bases));
        std::cout << "Test 9 PASSED: isCRTPClass with multiple bases (one CRTP)\n";
        passed++;
    }

    // Test 10: Template class with multiple type params + variadic
    {
        GenericType gt("gt5", "Tuple");
        gt.isClassTemplate = true;
        auto* tp1 = new TypeParameter("tp5", "Head");
        auto* tp2 = new TypeParameter("tp6", "Tail");
        tp2->isVariadic = true;
        gt.addChild("typeParameters", tp1);
        gt.addChild("typeParameters", tp2);

        json j = toJson(&gt);
        ASTNode* restored = fromJson(j);
        auto* rgt = dynamic_cast<GenericType*>(restored);
        assert(rgt->isClassTemplate);
        auto tps = rgt->getChildren("typeParameters");
        assert(tps.size() == 2);
        auto* rtp2 = dynamic_cast<TypeParameter*>(tps[1]);
        assert(rtp2->isVariadic);
        assert(rtp2->name == "Tail");
        deleteTree(restored);
        std::cout << "Test 10 PASSED: Template class with variadic type params roundtrip\n";
        passed++;
    }

    // Test 11: CompactAST getNodeName for GenericType
    {
        GenericType gt1("gt6", "List");
        assert(getNodeName(&gt1) == "List");

        GenericType gt2("gt7", "Container");
        gt2.isClassTemplate = true;
        assert(getNodeName(&gt2) == "template:Container");

        TypeParameter tp("tp7", "T");
        assert(getNodeName(&tp) == "T");
        std::cout << "Test 11 PASSED: CompactAST getNodeName for GenericType/TypeParameter\n";
        passed++;
    }

    // Test 12: Template class attached to ClassDeclaration + combined roundtrip
    {
        auto* cls = new ClassDeclaration("cls1", "Stack");
        cls->addBase("Container", "public", false);
        auto* gt = new GenericType("gt8", "Stack");
        gt->isClassTemplate = true;
        auto* tp = new TypeParameter("tp8", "T");
        tp->constraint = "Comparable";
        gt->addChild("typeParameters", tp);
        cls->addChild("typeParameters", gt);

        json j = toJson(cls);
        ASTNode* restored = fromJson(j);
        auto* rcls = dynamic_cast<ClassDeclaration*>(restored);
        assert(rcls->name == "Stack");
        auto tpChildren = rcls->getChildren("typeParameters");
        assert(tpChildren.size() == 1);
        auto* rgt = dynamic_cast<GenericType*>(tpChildren[0]);
        assert(rgt->isClassTemplate);
        assert(rgt->baseName == "Stack");
        auto rParams = rgt->getChildren("typeParameters");
        assert(rParams.size() == 1);
        auto* rtp = dynamic_cast<TypeParameter*>(rParams[0]);
        assert(rtp->name == "T");
        assert(rtp->constraint == "Comparable");

        deleteTree(restored);
        delete cls;
        std::cout << "Test 12 PASSED: Template class on ClassDeclaration combined roundtrip\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
