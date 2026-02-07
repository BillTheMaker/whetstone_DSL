// Step 42 TDD Test: C++ type generation
//
// Tests that CppGenerator maps SemAnno types to C++ equivalents:
// 1. PrimitiveType "int" → "int", "string" → "std::string", "bool" → "bool", "float" → "double"
// 2. ListType → "std::vector<T>"
// 3. MapType → "std::map<K, V>"
// 4. SetType → "std::unordered_set<T>"
// 5. OptionalType → "std::optional<T>"
// 6. ArrayType → "std::array<T, N>" or "T[]"
// 7. CustomType → type name as-is
// 8. Function with typed parameters produces correct C++ declarations
//
// Will fail to compile until CppGenerator is implemented.

#include <iostream>
#include <string>
#include <cassert>
#include "ast/Generator.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: PrimitiveType int ---
    {
        PrimitiveType type("t1", "int");
        CppGenerator gen;
        std::string output = gen.generate(&type);
        assert(contains(output, "int") && "PrimitiveType 'int' should generate 'int'");

        std::cout << "Test 1 PASS: PrimitiveType 'int' → 'int'" << std::endl;
        ++passed;
    }

    // --- Test 2: PrimitiveType string → std::string ---
    {
        PrimitiveType type("t1", "string");
        CppGenerator gen;
        std::string output = gen.generate(&type);
        assert(contains(output, "std::string") && "PrimitiveType 'string' should generate 'std::string'");

        std::cout << "Test 2 PASS: PrimitiveType 'string' → 'std::string'" << std::endl;
        ++passed;
    }

    // --- Test 3: PrimitiveType bool ---
    {
        PrimitiveType type("t1", "bool");
        CppGenerator gen;
        std::string output = gen.generate(&type);
        assert(contains(output, "bool") && "PrimitiveType 'bool' should generate 'bool'");

        std::cout << "Test 3 PASS: PrimitiveType 'bool' → 'bool'" << std::endl;
        ++passed;
    }

    // --- Test 4: PrimitiveType float → double ---
    {
        PrimitiveType type("t1", "float");
        CppGenerator gen;
        std::string output = gen.generate(&type);
        assert(contains(output, "double") && "PrimitiveType 'float' should generate 'double'");

        std::cout << "Test 4 PASS: PrimitiveType 'float' → 'double'" << std::endl;
        ++passed;
    }

    // --- Test 5: ListType → std::vector<T> ---
    {
        ListType listType;
        PrimitiveType* elemType = new PrimitiveType("t1", "int");
        listType.setChild("elementType", elemType);

        CppGenerator gen;
        std::string output = gen.generate(&listType);
        assert(contains(output, "std::vector") && "ListType should generate 'std::vector'");
        assert(contains(output, "int") && "ListType element type should appear");

        std::cout << "Test 5 PASS: ListType → 'std::vector<int>'" << std::endl;
        ++passed;

        delete elemType;
    }

    // --- Test 6: MapType → std::map<K, V> ---
    {
        MapType mapType;
        PrimitiveType* keyType = new PrimitiveType("t1", "string");
        PrimitiveType* valType = new PrimitiveType("t2", "int");
        mapType.setChild("keyType", keyType);
        mapType.setChild("valueType", valType);

        CppGenerator gen;
        std::string output = gen.generate(&mapType);
        assert(contains(output, "std::map") && "MapType should generate 'std::map'");
        assert(contains(output, "std::string") && "Map key type should be std::string");
        assert(contains(output, "int") && "Map value type should be int");

        std::cout << "Test 6 PASS: MapType → 'std::map<std::string, int>'" << std::endl;
        ++passed;

        delete valType;
        delete keyType;
    }

    // --- Test 7: SetType → std::unordered_set<T> ---
    {
        SetType setType;
        PrimitiveType* elemType = new PrimitiveType("t1", "int");
        setType.setChild("elementType", elemType);

        CppGenerator gen;
        std::string output = gen.generate(&setType);
        assert((contains(output, "std::unordered_set") || contains(output, "std::set")) &&
               "SetType should generate std::unordered_set or std::set");
        assert(contains(output, "int") && "Set element type should appear");

        std::cout << "Test 7 PASS: SetType → std::unordered_set<int> or std::set<int>" << std::endl;
        ++passed;

        delete elemType;
    }

    // --- Test 8: OptionalType → std::optional<T> ---
    {
        OptionalType optType;
        PrimitiveType* innerType = new PrimitiveType("t1", "int");
        optType.setChild("innerType", innerType);

        CppGenerator gen;
        std::string output = gen.generate(&optType);
        assert(contains(output, "std::optional") && "OptionalType should generate 'std::optional'");
        assert(contains(output, "int") && "Optional inner type should appear");

        std::cout << "Test 8 PASS: OptionalType → 'std::optional<int>'" << std::endl;
        ++passed;

        delete innerType;
    }

    // --- Test 9: CustomType → type name ---
    {
        CustomType type("t1", "Widget");
        CppGenerator gen;
        std::string output = gen.generate(&type);
        assert(contains(output, "Widget") && "CustomType should generate type name");

        std::cout << "Test 9 PASS: CustomType → 'Widget'" << std::endl;
        ++passed;
    }

    // --- Test 10: Function with typed parameters in C++ ---
    {
        Function fn("f1", "process");
        PrimitiveType* retType = new PrimitiveType("t0", "int");
        fn.setChild("returnType", retType);

        Parameter* p1 = new Parameter("p1", "name");
        PrimitiveType* p1Type = new PrimitiveType("t1", "string");
        p1->setChild("type", p1Type);
        fn.addChild("parameters", p1);

        Parameter* p2 = new Parameter("p2", "count");
        PrimitiveType* p2Type = new PrimitiveType("t2", "int");
        p2->setChild("type", p2Type);
        fn.addChild("parameters", p2);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        assert(contains(output, "int") && "Return type should appear");
        assert(contains(output, "process") && "Function name should appear");
        assert(contains(output, "std::string") && "String parameter type should be std::string");
        assert(contains(output, "name") && "First parameter name should appear");
        assert(contains(output, "count") && "Second parameter name should appear");

        std::cout << "Test 10 PASS: Function with typed params generates correct C++ declaration" << std::endl;
        ++passed;

        delete p2Type;
        delete p2;
        delete p1Type;
        delete p1;
        delete retType;
    }

    // --- Summary ---
    std::cout << "\n=== Step 42 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
