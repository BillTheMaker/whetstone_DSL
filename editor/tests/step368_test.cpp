// Step 368: WAT Cross-Language Projection (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "Pipeline.h"
#include "CrossLanguageProjector.h"
#include "SemannoFormat.h"
#include "ast/WatParser.h"

static bool contains(const std::string& text, const std::string& token) {
    return text.find(token) != std::string::npos;
}

int main() {
    int passed = 0;

    // Test 1: Python function -> WAT
    {
        Pipeline p;
        std::string py = "def add(x, y):\n    return x + y\n";
        auto res = p.run(py, "python", "wat");
        assert(res.success);
        assert(contains(res.generatedCode, "(module"));
        assert(contains(res.generatedCode, "(func"));
        std::cout << "Test 1 PASSED: Python -> WAT\n";
        passed++;
    }

    // Test 2: C function -> WAT
    {
        Pipeline p;
        std::string csrc = "int add(int a, int b) { return a + b; }\n";
        auto res = p.run(csrc, "c", "wat");
        assert(res.success);
        assert(contains(res.generatedCode, "(module"));
        std::cout << "Test 2 PASSED: C -> WAT\n";
        passed++;
    }

    // Test 3: WAT -> Python
    {
        Pipeline p;
        std::string wat = "(module (func $add (param $x i32) (param $y i32) (result i32)))";
        auto res = p.run(wat, "wat", "python");
        assert(res.success);
        assert(contains(res.generatedCode, "def"));
        std::cout << "Test 3 PASSED: WAT -> Python\n";
        passed++;
    }

    // Test 4: WAT -> C
    {
        Pipeline p;
        std::string wat = "(module (func $f (result i32)))";
        auto res = p.run(wat, "wat", "c");
        assert(res.success);
        assert(!res.generatedCode.empty());
        std::cout << "Test 4 PASSED: WAT -> C\n";
        passed++;
    }

    // Test 5: WAT -> Rust
    {
        Pipeline p;
        std::string wat = "(module (func $f (result i32)))";
        auto res = p.run(wat, "wat", "rust");
        assert(res.success);
        assert(contains(res.generatedCode, "fn"));
        std::cout << "Test 5 PASSED: WAT -> Rust\n";
        passed++;
    }

    // Test 6: Java -> WAT
    {
        Pipeline p;
        std::string java = "class A { int add(int x, int y) { return x + y; } }\n";
        auto res = p.run(java, "java", "wat");
        assert(res.success);
        assert(contains(res.generatedCode, "(module"));
        std::cout << "Test 6 PASSED: Java -> WAT\n";
        passed++;
    }

    // Test 7: Python -> WAT -> Python keeps function identity
    {
        Pipeline p;
        std::string py = "def add(x, y):\n    return x + y\n";
        auto toWat = p.run(py, "python", "wat");
        assert(toWat.success);
        auto back = p.run(toWat.generatedCode, "wat", "python");
        assert(back.success);
        assert(contains(back.generatedCode, "add"));
        std::cout << "Test 7 PASSED: Python -> WAT -> Python identity\n";
        passed++;
    }

    // Test 8: memory annotation preserved across WAT <-> C
    {
        Module watMod("m1", "watmod", "wat");
        auto* fn = new Function("f1", "grow");
        fn->addChild("annotations", new OwnerAnnotation("a1", "Manual"));
        watMod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto cMod = projector.project(&watMod, "c");
        auto back = projector.project(cMod.get(), "wat");
        assert(projector.annotationsPreserved(&watMod, back.get()));
        std::cout << "Test 8 PASSED: WAT<->C annotation preservation\n";
        passed++;
    }

    // Test 9: WAT imports survive projection metadata to C
    {
        auto wat = WatParser::parseWat("(module (import \"env\" \"print\" (func $print)))");
        CrossLanguageProjector projector;
        auto cMod = projector.project(wat.get(), "c");
        assert(!cMod->getChildren("imports").empty());
        std::cout << "Test 9 PASSED: imports survive projection metadata\n";
        passed++;
    }

    // Test 10: multi-function WAT module -> C output non-empty
    {
        Pipeline p;
        std::string wat = "(module (func $a (result i32)) (func $b (result i32)))";
        auto res = p.run(wat, "wat", "c");
        assert(res.success);
        assert(!res.generatedCode.empty());
        std::cout << "Test 10 PASSED: multi-function WAT -> C output\n";
        passed++;
    }

    // Test 11: Semanno roundtrip on WAT-style annotation
    {
        ExecAnnotation exec;
        exec.id = "e1";
        exec.mode = "stack";
        std::string line = SemannoEmitter::emit(&exec);
        auto parsed = SemannoParser::parse(line);
        assert(parsed.type == "exec");
        assert(parsed.properties["mode"] == "stack");
        std::cout << "Test 11 PASSED: semanno roundtrip\n";
        passed++;
    }

    // Test 12: pipeline run with WAT source succeeds
    {
        Pipeline p;
        std::string wat = "(module (func $f (result i32)))";
        auto res = p.run(wat, "wat", "wat");
        assert(res.success);
        assert(contains(res.generatedCode, "(module"));
        std::cout << "Test 12 PASSED: pipeline WAT source run\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
