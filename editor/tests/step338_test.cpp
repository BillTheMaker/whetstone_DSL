// Step 338: EnumDeclaration + NamespaceDeclaration (12 tests)
// Tests EnumDeclaration, EnumMember, NamespaceDeclaration, TypeAlias construction,
// serialization roundtrip, and CompactAST output.

#include <cassert>
#include <iostream>
#include <string>
#include "ast/EnumNamespaceNodes.h"
#include "ast/PreprocessorNodes.h"
#include "ast/AsyncNodes.h"
#include "ast/Serialization.h"
#include "CompactAST.h"
#include "ast/Function.h"

int main() {
    int passed = 0;

    // Test 1: EnumDeclaration construction — scoped enum
    {
        auto* e = new EnumDeclaration("e1", "Color", true);
        assert(e->conceptType == "EnumDeclaration");
        assert(e->name == "Color");
        assert(e->isScoped);
        assert(e->underlyingType.empty());
        delete e;
        std::cout << "Test 1 PASSED: EnumDeclaration scoped\n";
        passed++;
    }

    // Test 2: EnumDeclaration — unscoped with underlying type
    {
        auto* e = new EnumDeclaration("e2", "Flags", false);
        e->underlyingType = "uint8_t";
        assert(!e->isScoped);
        assert(e->underlyingType == "uint8_t");
        delete e;
        std::cout << "Test 2 PASSED: EnumDeclaration unscoped with underlying type\n";
        passed++;
    }

    // Test 3: EnumMember with explicit value
    {
        auto* m = new EnumMember("m1", "Red", "0");
        assert(m->conceptType == "EnumMember");
        assert(m->name == "Red");
        assert(m->value == "0");
        auto* m2 = new EnumMember("m2", "Green");
        assert(m2->value.empty());
        delete m; delete m2;
        std::cout << "Test 3 PASSED: EnumMember with explicit value\n";
        passed++;
    }

    // Test 4: Enum with 4 members as children
    {
        auto* e = new EnumDeclaration("e3", "Direction", true);
        e->addChild("members", new EnumMember("m3", "North", "0"));
        e->addChild("members", new EnumMember("m4", "South", "1"));
        e->addChild("members", new EnumMember("m5", "East", "2"));
        e->addChild("members", new EnumMember("m6", "West", "3"));
        auto members = e->getChildren("members");
        assert(members.size() == 4);
        assert(static_cast<EnumMember*>(members[2])->name == "East");
        delete e;
        std::cout << "Test 4 PASSED: Enum with 4 members as children\n";
        passed++;
    }

    // Test 5: NamespaceDeclaration with body children
    {
        auto* ns = new NamespaceDeclaration("ns1", "whetstone");
        auto* fn = new Function("fn1", "helper");
        ns->addChild("body", fn);
        auto body = ns->getChildren("body");
        assert(body.size() == 1);
        assert(body[0]->conceptType == "Function");
        assert(ns->name == "whetstone");
        delete ns;
        std::cout << "Test 5 PASSED: NamespaceDeclaration with body\n";
        passed++;
    }

    // Test 6: TypeAlias — using vs typedef
    {
        auto* ua = new TypeAlias("ta1", "StringVec", "std::vector<std::string>", true);
        assert(ua->conceptType == "TypeAlias");
        assert(ua->aliasName == "StringVec");
        assert(ua->targetType == "std::vector<std::string>");
        assert(ua->isUsing);

        auto* td = new TypeAlias("ta2", "UINT", "unsigned int", false);
        assert(!td->isUsing);
        delete ua; delete td;
        std::cout << "Test 6 PASSED: TypeAlias using vs typedef\n";
        passed++;
    }

    // Test 7: EnumDeclaration JSON roundtrip
    {
        auto* e = new EnumDeclaration("e4", "Status", true);
        e->underlyingType = "int";
        e->addChild("members", new EnumMember("m7", "OK", "0"));
        e->addChild("members", new EnumMember("m8", "Error", "1"));
        json j = toJson(e);
        auto* restored = fromJson(j);
        assert(restored->conceptType == "EnumDeclaration");
        auto* re = static_cast<EnumDeclaration*>(restored);
        assert(re->name == "Status");
        assert(re->isScoped);
        assert(re->underlyingType == "int");
        auto members = re->getChildren("members");
        assert(members.size() == 2);
        assert(static_cast<EnumMember*>(members[1])->name == "Error");
        assert(static_cast<EnumMember*>(members[1])->value == "1");
        delete e; delete restored;
        std::cout << "Test 7 PASSED: EnumDeclaration JSON roundtrip\n";
        passed++;
    }

    // Test 8: NamespaceDeclaration JSON roundtrip
    {
        auto* ns = new NamespaceDeclaration("ns2", "detail");
        ns->addChild("body", new Function("fn2", "impl"));
        json j = toJson(ns);
        auto* restored = fromJson(j);
        assert(restored->conceptType == "NamespaceDeclaration");
        auto* rns = static_cast<NamespaceDeclaration*>(restored);
        assert(rns->name == "detail");
        auto body = rns->getChildren("body");
        assert(body.size() == 1);
        delete ns; delete restored;
        std::cout << "Test 8 PASSED: NamespaceDeclaration JSON roundtrip\n";
        passed++;
    }

    // Test 9: TypeAlias JSON roundtrip
    {
        auto* ta = new TypeAlias("ta3", "Vec3", "std::array<float, 3>", true);
        json j = toJson(ta);
        auto* restored = fromJson(j);
        assert(restored->conceptType == "TypeAlias");
        auto* rta = static_cast<TypeAlias*>(restored);
        assert(rta->aliasName == "Vec3");
        assert(rta->targetType == "std::array<float, 3>");
        assert(rta->isUsing);
        delete ta; delete restored;
        std::cout << "Test 9 PASSED: TypeAlias JSON roundtrip\n";
        passed++;
    }

    // Test 10: CompactAST node names
    {
        auto* e = new EnumDeclaration("e5", "Fruit", true);
        assert(getNodeName(e) == "Fruit");
        auto* m = new EnumMember("m9", "Apple");
        assert(getNodeName(m) == "Apple");
        auto* ns = new NamespaceDeclaration("ns3", "ast");
        assert(getNodeName(ns) == "ast");
        auto* ta = new TypeAlias("ta4", "Ptr", "std::shared_ptr<T>", true);
        assert(getNodeName(ta) == "Ptr");
        delete e; delete m; delete ns; delete ta;
        std::cout << "Test 10 PASSED: CompactAST node names\n";
        passed++;
    }

    // Test 11: Nested namespace (namespace inside namespace)
    {
        auto* outer = new NamespaceDeclaration("ns4", "whetstone");
        auto* inner = new NamespaceDeclaration("ns5", "detail");
        auto* fn = new Function("fn3", "helper");
        inner->addChild("body", fn);
        outer->addChild("body", inner);
        auto outerBody = outer->getChildren("body");
        assert(outerBody.size() == 1);
        assert(outerBody[0]->conceptType == "NamespaceDeclaration");
        auto innerBody = outerBody[0]->getChildren("body");
        assert(innerBody.size() == 1);
        assert(innerBody[0]->conceptType == "Function");
        delete outer;
        std::cout << "Test 11 PASSED: Nested namespace\n";
        passed++;
    }

    // Test 12: Enum inside namespace
    {
        auto* ns = new NamespaceDeclaration("ns6", "types");
        auto* e = new EnumDeclaration("e6", "Color", true);
        e->addChild("members", new EnumMember("m10", "Red"));
        e->addChild("members", new EnumMember("m11", "Blue"));
        ns->addChild("body", e);
        auto body = ns->getChildren("body");
        assert(body.size() == 1);
        assert(body[0]->conceptType == "EnumDeclaration");
        auto* re = static_cast<EnumDeclaration*>(body[0]);
        assert(re->getChildren("members").size() == 2);
        // JSON roundtrip of the whole structure
        json j = toJson(ns);
        auto* restored = fromJson(j);
        auto rbody = restored->getChildren("body");
        assert(rbody.size() == 1);
        assert(rbody[0]->conceptType == "EnumDeclaration");
        delete ns; delete restored;
        std::cout << "Test 12 PASSED: Enum inside namespace\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12 tests passed\n";
    return (passed == 12) ? 0 : 1;
}
