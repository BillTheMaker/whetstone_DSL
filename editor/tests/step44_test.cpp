// Step 44 TDD Test: C++-specific idioms
//
// Tests that CppGenerator handles C++-specific features:
// 1. FunctionCall with proper argument passing
// 2. const correctness in parameter declarations
// 3. Reference parameters (& and const&)
// 4. LangSpecific annotation for C++ → #include directives
// 5. LangSpecific annotation → using namespace declarations
//
// Will fail to compile until CppGenerator is fully implemented.

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

    // --- Test 1: FunctionCall generates proper C++ call syntax ---
    {
        FunctionCall call;
        call.functionName = "std::sort";
        VariableReference* arg1 = new VariableReference("v1", "vec.begin()");
        VariableReference* arg2 = new VariableReference("v2", "vec.end()");
        call.addChild("arguments", arg1);
        call.addChild("arguments", arg2);

        CppGenerator gen;
        std::string output = gen.generate(&call);

        assert(contains(output, "std::sort") && "Should contain function name");
        assert(contains(output, "(") && "Should have opening paren");
        assert(contains(output, ")") && "Should have closing paren");

        std::cout << "Test 1 PASS: FunctionCall generates proper C++ call" << std::endl;
        ++passed;

        delete arg2;
        delete arg1;
    }

    // --- Test 2: const parameter in function ---
    {
        Function fn("f1", "printName");
        PrimitiveType* retType = new PrimitiveType("t0", "void");
        fn.setChild("returnType", retType);

        Parameter* param = new Parameter("p1", "name");
        PrimitiveType* paramType = new PrimitiveType("t1", "string");
        param->setChild("type", paramType);

        // Mark as const reference via LangSpecific annotation
        LangSpecific* constAnno = new LangSpecific();
        constAnno->language = "cpp";
        constAnno->idiomType = "const_ref";
        param->addChild("annotations", constAnno);

        fn.addChild("parameters", param);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        assert(contains(output, "printName") && "Should contain function name");
        assert(contains(output, "name") && "Should contain parameter name");
        // With const_ref annotation, should produce "const std::string&"
        assert((contains(output, "const") || contains(output, "&")) &&
               "const_ref annotation should produce const reference parameter");

        std::cout << "Test 2 PASS: const reference parameter annotation" << std::endl;
        ++passed;

        delete constAnno;
        delete paramType;
        delete param;
        delete retType;
    }

    // --- Test 3: LangSpecific annotation for #include ---
    {
        Module mod("m1", "Sorter", "cpp");

        LangSpecific* includeAnno = new LangSpecific();
        includeAnno->language = "cpp";
        includeAnno->idiomType = "include";
        includeAnno->rawSyntax = "#include <vector>";
        mod.addChild("annotations", includeAnno);

        Function* fn = new Function("f1", "sort");
        PrimitiveType* retType = new PrimitiveType("t1", "void");
        fn->setChild("returnType", retType);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert(contains(output, "#include") && "Should emit #include directive");
        assert(contains(output, "vector") && "Should include vector header");

        std::cout << "Test 3 PASS: LangSpecific 'include' emits #include directive" << std::endl;
        ++passed;

        delete retType;
        delete fn;
        delete includeAnno;
    }

    // --- Test 4: LangSpecific annotation for using namespace ---
    {
        Module mod("m1", "App", "cpp");

        LangSpecific* usingAnno = new LangSpecific();
        usingAnno->language = "cpp";
        usingAnno->idiomType = "using_namespace";
        usingAnno->rawSyntax = "using namespace std;";
        mod.addChild("annotations", usingAnno);

        Function* fn = new Function("f1", "main");
        PrimitiveType* retType = new PrimitiveType("t1", "int");
        fn->setChild("returnType", retType);
        mod.addChild("functions", fn);

        CppGenerator gen;
        std::string output = gen.generate(&mod);

        assert(contains(output, "using namespace") && "Should emit using namespace declaration");

        std::cout << "Test 4 PASS: LangSpecific 'using_namespace' emits using declaration" << std::endl;
        ++passed;

        delete retType;
        delete fn;
        delete usingAnno;
    }

    // --- Test 5: void return type function ---
    {
        Function fn("f1", "doNothing");
        PrimitiveType* retType = new PrimitiveType("t1", "void");
        fn.setChild("returnType", retType);

        CppGenerator gen;
        std::string output = gen.generate(&fn);

        assert(contains(output, "void") && "Should contain void return type");
        assert(contains(output, "doNothing") && "Should contain function name");

        std::cout << "Test 5 PASS: void return type function" << std::endl;
        ++passed;

        delete retType;
    }

    // --- Test 6: IndexAccess generates bracket notation ---
    {
        IndexAccess access;
        VariableReference* target = new VariableReference("v1", "arr");
        IntegerLiteral* index = new IntegerLiteral("i1", 0);
        access.setChild("target", target);
        access.setChild("index", index);

        CppGenerator gen;
        std::string output = gen.generate(&access);

        assert(contains(output, "arr") && "Should contain target name");
        assert(contains(output, "[") && "Should contain opening bracket");
        assert(contains(output, "0") && "Should contain index");
        assert(contains(output, "]") && "Should contain closing bracket");

        std::cout << "Test 6 PASS: IndexAccess generates arr[0]" << std::endl;
        ++passed;

        delete index;
        delete target;
    }

    // --- Test 7: MemberAccess generates dot or arrow notation ---
    {
        MemberAccess access;
        access.memberName = "size";
        VariableReference* target = new VariableReference("v1", "vec");
        access.setChild("target", target);

        CppGenerator gen;
        std::string output = gen.generate(&access);

        assert(contains(output, "vec") && "Should contain target name");
        assert(contains(output, "size") && "Should contain member name");
        // Could be vec.size or vec->size depending on pointer context
        assert((contains(output, ".") || contains(output, "->")) &&
               "Should use dot or arrow notation");

        std::cout << "Test 7 PASS: MemberAccess generates member access" << std::endl;
        ++passed;

        delete target;
    }

    // --- Summary ---
    std::cout << "\n=== Step 44 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
