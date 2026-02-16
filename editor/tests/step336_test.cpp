// Step 336: Phase 12c Integration Tests — C++ Inheritance + Template roundtrip (8 tests)
// Full pipeline: parse C++ → AST → generate → verify cross-language adaptation

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
#include "ast/Parser.h"

int main() {
    int passed = 0;

    // Test 1: Parse Whetstone-style class with CRTP + multiple inheritance
    {
        std::string src = R"(
class KotlinGenerator : public ProjectionGenerator, public virtual AnnotationVisitorExtended, public SemannoAnnotationImpl<KotlinGenerator> {
public:
    void generate() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        assert(mod != nullptr);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = static_cast<ClassDeclaration*>(classes[0]);
        assert(cls->name == "KotlinGenerator");
        auto bases = cls->getBases();
        assert(bases.size() >= 2);
        bool foundPG = false;
        for (auto& b : bases) {
            if (b.name.find("ProjectionGenerator") != std::string::npos) foundPG = true;
        }
        assert(foundPG);
        std::cout << "Test 1 PASSED: Parse Whetstone-style class with multiple inheritance\n";
        passed++;
    }

    // Test 2: CRTP detection on parsed class
    {
        std::string src = R"(
class MyGen : public Base, public SemannoAnnotationImpl<MyGen> {
public:
    void foo() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        assert(!classes.empty());
        auto* cls = static_cast<ClassDeclaration*>(classes[0]);
        auto bases = cls->getBases();
        bool hasCRTP = false;
        for (auto& b : bases) {
            if (b.name.find("<MyGen>") != std::string::npos) hasCRTP = true;
        }
        assert(hasCRTP);
        std::vector<std::string> baseNames;
        for (auto& b : bases) baseNames.push_back(b.name);
        assert(isCRTPClass("MyGen", baseNames));
        std::cout << "Test 2 PASSED: CRTP detection on parsed class\n";
        passed++;
    }

    // Test 3: Round-trip: parse → generate → verify structure preserved
    {
        std::string src = R"(
class Widget : public Base, public Drawable {
public:
    void render() {}
};
)";
        auto mod = TreeSitterParser::parseCpp(src);
        auto classes = mod->getChildren("classes");
        auto* cls = static_cast<ClassDeclaration*>(classes[0]);

        CppGenerator gen;
        std::string output = gen.generate(cls);
        assert(output.find("class Widget") != std::string::npos);
        assert(output.find("Base") != std::string::npos);
        assert(output.find("Drawable") != std::string::npos);
        assert(output.find(":") != std::string::npos);
        std::cout << "Test 3 PASSED: Parse → generate roundtrip structure preserved\n";
        passed++;
    }

    // Test 4: Cross-language: C++ class → Java output (extends + implements)
    {
        auto* cls = new ClassDeclaration("cls4", "Widget");
        cls->addBase("Component", "public");
        cls->addBase("Drawable", "public");
        cls->addBase("Serializable", "public");

        JavaGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("class Widget") != std::string::npos);
        assert(out.find("extends Component") != std::string::npos);
        assert(out.find("implements") != std::string::npos);
        assert(out.find("Drawable") != std::string::npos);
        assert(out.find("Serializable") != std::string::npos);
        delete cls;
        std::cout << "Test 4 PASSED: C++ → Java (extends + implements)\n";
        passed++;
    }

    // Test 5: Cross-language: C++ class → Python output (multiple bases)
    {
        auto* cls = new ClassDeclaration("cls5", "Widget");
        cls->addBase("Component", "public");
        cls->addBase("Drawable", "public");
        cls->addBase("Serializable", "public");

        PythonGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("class Widget") != std::string::npos);
        assert(out.find("Component") != std::string::npos);
        assert(out.find("Drawable") != std::string::npos);
        assert(out.find("Serializable") != std::string::npos);
        delete cls;
        std::cout << "Test 5 PASSED: C++ → Python multiple bases\n";
        passed++;
    }

    // Test 6: Template class with multiple type params → generate → verify
    {
        auto* cls = new ClassDeclaration("cls6", "Map");
        auto* gt = new GenericType("gt1", "Map");
        gt->isClassTemplate = true;
        auto* tp1 = new TypeParameter("tp1", "K");
        auto* tp2 = new TypeParameter("tp2", "V");
        auto* tp3 = new TypeParameter("tp3", "Alloc");
        gt->addChild("typeParameters", tp1);
        gt->addChild("typeParameters", tp2);
        gt->addChild("typeParameters", tp3);
        cls->addChild("typeParameters", gt);

        CppGenerator gen;
        std::string out = gen.generate(cls);
        assert(out.find("template<") != std::string::npos);
        assert(out.find("typename K") != std::string::npos);
        assert(out.find("typename V") != std::string::npos);
        assert(out.find("typename Alloc") != std::string::npos);
        assert(out.find("class Map") != std::string::npos);
        delete cls;
        std::cout << "Test 6 PASSED: Template class with 3 type params\n";
        passed++;
    }

    // Test 7: Diamond inheritance detection using classMap API
    {
        // Build hierarchy: D inherits B and C, both inherit A → diamond
        std::map<std::string, std::vector<std::string>> classMap;
        classMap["D"] = {"B", "C"};
        classMap["B"] = {"A"};
        classMap["C"] = {"A"};
        assert(ClassDeclaration::hasDiamondInheritance("D", classMap));

        // Non-diamond: linear chain X → Y → Z
        std::map<std::string, std::vector<std::string>> linearMap;
        linearMap["X"] = {"Y"};
        linearMap["Y"] = {"Z"};
        assert(!ClassDeclaration::hasDiamondInheritance("X", linearMap));

        std::cout << "Test 7 PASSED: Diamond inheritance detection\n";
        passed++;
    }

    // Test 8: Mixed: template + multiple inheritance + virtual + methods → full pipeline
    {
        auto* cls = new ClassDeclaration("cls8", "AdvancedWidget");
        cls->addBase("BaseWidget", "public");
        cls->addBase("EventHandler", "public", true); // virtual
        cls->addBase("CRTP<AdvancedWidget>", "public");

        auto* gt = new GenericType("gt2", "AdvancedWidget");
        gt->isClassTemplate = true;
        auto* tp = new TypeParameter("tp4", "T");
        gt->addChild("typeParameters", tp);
        cls->addChild("typeParameters", gt);

        auto* meth = new MethodDeclaration("m1", "onEvent");
        meth->isVirtual = true;
        meth->isOverride = true;
        cls->addChild("methods", meth);

        // C++ output
        CppGenerator cppGen;
        std::string cppOut = cppGen.generate(cls);
        assert(cppOut.find("template<") != std::string::npos);
        assert(cppOut.find("class AdvancedWidget") != std::string::npos);
        assert(cppOut.find("BaseWidget") != std::string::npos);
        assert(cppOut.find("virtual") != std::string::npos);
        assert(cppOut.find("EventHandler") != std::string::npos);
        assert(cppOut.find("CRTP<AdvancedWidget>") != std::string::npos);
        assert(cppOut.find("onEvent") != std::string::npos);
        assert(cppOut.find("override") != std::string::npos);

        // JSON roundtrip
        nlohmann::json j = toJson(cls);
        auto* restored = fromJson(j);
        assert(restored != nullptr);
        assert(restored->conceptType == "ClassDeclaration");
        auto* rcls = static_cast<ClassDeclaration*>(restored);
        assert(rcls->name == "AdvancedWidget");
        auto rbases = rcls->getBases();
        assert(rbases.size() == 3);

        // Generate from restored
        std::string rOut = cppGen.generate(restored);
        assert(rOut.find("class AdvancedWidget") != std::string::npos);
        assert(rOut.find("BaseWidget") != std::string::npos);

        delete cls;
        delete restored;
        std::cout << "Test 8 PASSED: Mixed template + multi-inheritance + virtual pipeline\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8 tests passed\n";
    return (passed == 8) ? 0 : 1;
}
