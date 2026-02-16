// Step 364: C Integration + Self-Hosting Prep (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "Pipeline.h"
#include "AnnotationInference.h"
#include "MemoryStrategyInference.h"
#include "CrossLanguageProjector.h"
#include "MCPServer.h"
#include "ast/CParser.h"
#include "ast/CGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/PythonGenerator.h"
#include "ast/RustGenerator.h"

static bool contains(const std::string& text, const std::string& token) {
    return text.find(token) != std::string::npos;
}

static bool hasSuggestion(const std::vector<MemoryStrategyInference::Suggestion>& suggestions,
                          const std::string& annotationType,
                          const std::string& strategy) {
    for (const auto& s : suggestions) {
        if (s.annotationType == annotationType && s.strategy == strategy) return true;
    }
    return false;
}

static bool hasInference(const std::vector<AnnotationInference::InferredAnnotation>& inferred,
                         const std::string& annotationType,
                         const std::string& value) {
    for (const auto& a : inferred) {
        if (a.annotationType == annotationType && a.value == value) return true;
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: Parse C -> generate C -> reparse keeps core declarations
    {
        std::string src = "typedef struct Point { int x; int y; } Point;\\n"
                          "int add(int a, int b) { return a + b; }\\n";
        auto mod = CParser::parseC(src);
        CGenerator gen;
        std::string out = gen.generate(mod.get());
        auto roundtrip = CParser::parseC(out);
        assert(!roundtrip->getChildren("classes").empty());
        assert(contains(out, "typedef struct Point"));
        std::cout << "Test 1 PASSED: C parse/generate roundtrip keeps declarations\\n";
        passed++;
    }

    // Test 2: C struct projects as Python class shape
    {
        auto* cls = new ClassDeclaration("c1", "Point");
        auto* x = new Variable("v1", "x");
        x->setChild("type", new PrimitiveType("t1", "int"));
        cls->addChild("fields", x);

        PythonGenerator py;
        std::string out = py.generate(cls);
        assert(contains(out, "class Point"));
        assert(contains(out, "x"));
        std::cout << "Test 2 PASSED: C struct emits Python class form\\n";
        passed++;
    }

    // Test 3: Python-style class emits C struct + constructor-style function
    {
        auto* cls = new ClassDeclaration("c1", "Widget");
        auto* field = new Variable("v1", "value");
        field->setChild("type", new PrimitiveType("t1", "int"));
        cls->addChild("fields", field);
        auto* init = new MethodDeclaration("m1", "init");
        init->className = "Widget";
        init->setChild("returnType", new PrimitiveType("rt", "void"));
        cls->addChild("methods", init);
        CGenerator cgen;
        std::string outStruct = cgen.generate(cls);
        std::string outCtor = cgen.generate(init);
        assert(contains(outStruct, "typedef struct Widget"));
        assert(contains(outCtor, "Widget_init("));
        std::cout << "Test 3 PASSED: Python class shape + constructor-style method generation\\n";
        passed++;
    }

    // Test 4: C manual ownership projects to Rust ownership semantics
    {
        Module cmod("m1", "cmod", "c");
        auto* fn = new Function("f1", "make");
        fn->addChild("annotations", new OwnerAnnotation("a1", "Manual"));
        cmod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto rustMod = projector.project(&cmod, "rust");
        auto* rustFn = static_cast<Function*>(rustMod->getChildren("functions")[0]);

        bool hasBox = false;
        for (auto* a : rustFn->getChildren("annotations")) {
            if (a->conceptType == "OwnerAnnotation") {
                auto* oa = static_cast<OwnerAnnotation*>(a);
                hasBox = hasBox || (oa->strategy == "Box");
            }
        }
        assert(hasBox);

        RustGenerator rust;
        std::string out = rust.generate(rustMod.get());
        assert(contains(out, "@owner(Box)"));
        std::cout << "Test 4 PASSED: C manual owner adapts to Rust Box\\n";
        passed++;
    }

    // Test 5: Pipeline.run C->C++ succeeds and emits C++ translation unit shell
    {
        Pipeline p;
        std::string src = "struct Point { int x; int y; };\\n";
        auto result = p.run(src, "c", "cpp");
        assert(result.success);
        assert(!result.generatedCode.empty());
        assert(contains(result.generatedCode, "namespace"));
        std::cout << "Test 5 PASSED: pipeline C->C++ run succeeds with generated output\\n";
        passed++;
    }

    // Test 6: C inference captures malloc + array access + pointer parameter
    {
        Module mod("m1", "infer", "c");
        auto* fn = new Function("f1", "process");
        auto* param = new Parameter("p1", "cb");
        param->setChild("type", new PrimitiveType("pt", "void (*)(int)"));
        fn->addChild("parameters", param);

        auto* allocCall = new FunctionCall();
        allocCall->id = "fc1";
        allocCall->functionName = "malloc";
        fn->addChild("body", allocCall);

        auto* idx = new IndexAccess();
        idx->id = "idx1";
        idx->setChild("target", new VariableReference("vr1", "arr"));
        idx->setChild("index", new VariableReference("vr2", "i"));
        auto* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", idx);
        fn->addChild("body", stmt);
        mod.addChild("functions", fn);

        MemoryStrategyInference mem;
        AnnotationInference anno;
        auto suggestions = mem.inferAnnotations(&mod);
        auto inferred = anno.inferAll(&mod);

        assert(hasSuggestion(suggestions, "OwnerAnnotation", "Manual"));
        assert(hasSuggestion(suggestions, "OwnerAnnotation", "Borrowed"));
        assert(hasInference(inferred, "BoundsCheckAnnotation", "unchecked"));
        std::cout << "Test 6 PASSED: C inference covers malloc/array/pointer patterns\\n";
        passed++;
    }

    // Test 7: Parse simplified C header from dependency-style source
    {
        std::string header =
            "#ifndef TINY_HDR_H\\n"
            "#define TINY_HDR_H\\n"
            "#include <stddef.h>\\n"
            "typedef struct Tiny { int size; } Tiny;\\n"
            "enum TinyMode { TinyFast = 1, TinySafe = 2 };\\n"
            "#endif\\n";

        auto mod = CParser::parseC(header);
        assert(!mod->getChildren("classes").empty());
        assert(!mod->getChildren("statements").empty());
        std::cout << "Test 7 PASSED: simplified C header parses into expected AST\\n";
        passed++;
    }

    // Test 8: MCP tools list stable and pipeline tool accepts language parameter
    {
        MCPServer server;
        auto tools = server.getTools();
        const size_t expectedCount = tools.size();

        json req = {
            {"jsonrpc", "2.0"},
            {"id", 1},
            {"method", "tools/list"},
            {"params", json::object()}
        };
        auto resp = server.handleRequest(req);
        assert(resp.contains("result"));
        assert(resp["result"].contains("tools"));
        auto listed = resp["result"]["tools"];
        assert(listed.is_array());
        assert(listed.size() == expectedCount);

        bool hasRunPipeline = false;
        for (const auto& t : listed) {
            if (t.value("name", "") == "whetstone_run_pipeline") {
                hasRunPipeline = true;
                auto props = t["inputSchema"]["properties"];
                assert(props.contains("sourceLanguage"));
                assert(props["sourceLanguage"]["type"] == "string");
            }
        }
        assert(hasRunPipeline);
        std::cout << "Test 8 PASSED: MCP tools stable; run_pipeline accepts language string\\n";
        passed++;
    }

    std::cout << "\\nResults: " << passed << "/8\\n";
    assert(passed == 8);
    return 0;
}
