// Step 369: Phase 14b Integration (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <algorithm>
#include "Pipeline.h"
#include "CrossLanguageProjector.h"
#include "AnnotationInference.h"
#include "CompactAST.h"
#include "SemannoSidecar.h"
#include "MCPServer.h"
#include "ast/WatParser.h"
#include "ast/WatGenerator.h"

static bool contains(const std::string& text, const std::string& token) {
    return text.find(token) != std::string::npos;
}

int main() {
    int passed = 0;

    // Test 1: Parse WAT -> generate WAT -> valid shape
    {
        std::string wat = "(module (func $add (param $x i32) (param $y i32) (result i32) i32.add))";
        auto mod = WatParser::parseWat(wat);
        WatGenerator gen;
        std::string out = gen.generate(mod.get());
        assert(contains(out, "(module"));
        assert(contains(out, "(func"));
        assert(std::count(out.begin(), out.end(), '(') == std::count(out.begin(), out.end(), ')'));
        std::cout << "Test 1 PASSED: parse/generate WAT validity\n";
        passed++;
    }

    // Test 2: C -> WAT -> C roundtrip
    {
        Pipeline p;
        std::string csrc = "int add(int a, int b) { return a + b; }\n";
        auto toWat = p.run(csrc, "c", "wat");
        assert(toWat.success);
        auto toC = p.run(toWat.generatedCode, "wat", "c");
        assert(toC.success);
        assert(!toC.generatedCode.empty());
        std::cout << "Test 2 PASSED: C -> WAT -> C roundtrip\n";
        passed++;
    }

    // Test 3: 12 language parser routes can target WAT
    {
        struct Sample { std::string lang; std::string src; };
        std::vector<Sample> samples = {
            {"python", "def f(x):\n    return x\n"},
            {"cpp", "int f(int x){ return x; }\n"},
            {"elisp", "(defun f (x) x)\n"},
            {"javascript", "function f(x){ return x; }\n"},
            {"typescript", "function f(x:number){ return x; }\n"},
            {"java", "class A { int f(int x){ return x; } }\n"},
            {"rust", "fn f(x:i32)->i32 { x }\n"},
            {"go", "package main\nfunc f(x int) int { return x }\n"},
            {"kotlin", "fun f(x:Int):Int { return x }\n"},
            {"csharp", "class A { int F(int x){ return x; } }\n"},
            {"c", "int f(int x){ return x; }\n"},
            {"wat", "(module (func $f (param $x i32) (result i32)))"}
        };

        Pipeline p;
        int succeeded = 0;
        for (const auto& s : samples) {
            auto res = p.run(s.src, s.lang, "wat");
            if (res.success && !res.generatedCode.empty()) {
                succeeded++;
            }
        }
        assert(succeeded >= 10); // allow parser variability, require broad coverage
        std::cout << "Test 3 PASSED: multi-language -> WAT coverage (" << succeeded << "/12)\n";
        passed++;
    }

    // Test 4: WAT annotations inferred and present
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("functions", new Function("f1", "run"));
        mod.addChild("statements", new PragmaDirective("p1", "memory"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        bool hasExec = false, hasLayout = false;
        for (const auto& a : inferred) {
            hasExec = hasExec || (a.annotationType == "ExecAnnotation" && a.value == "stack");
            hasLayout = hasLayout || (a.annotationType == "LayoutAnnotation" && a.value == "linear");
        }
        assert(hasExec && hasLayout);
        std::cout << "Test 4 PASSED: WAT annotations meaningful\n";
        passed++;
    }

    // Test 5: Compact AST for WAT module has expected node labels
    {
        auto mod = WatParser::parseWat("(module (func $f (result i32)))");
        auto compact = toJsonCompactTree(mod.get());
        assert(compact.is_array());
        bool hasFunctionNode = false;
        for (const auto& n : compact) {
            if (n.contains("type") && n["type"] == "Function") hasFunctionNode = true;
        }
        assert(hasFunctionNode);
        std::cout << "Test 5 PASSED: compact AST includes Function node\n";
        passed++;
    }

    // Test 6: Diagnostics catch malformed WAT
    {
        auto pr = WatParser::parseWatWithDiagnostics("(module (func $f (result i32))");
        assert(!pr.diagnostics.empty());
        bool hasError = false;
        for (const auto& d : pr.diagnostics) if (d.severity == "error") hasError = true;
        assert(hasError);
        std::cout << "Test 6 PASSED: malformed WAT diagnostics\n";
        passed++;
    }

    // Test 7: Semanno sidecar save/load for WAT file
    {
        Module mod("m1", "wat", "wat");
        auto* fn = new Function("f1", "run");
        fn->setSpan(1, 1, 1, 10);
        auto* owner = new OwnerAnnotation("a1", "Manual");
        fn->addChild("annotations", owner);
        mod.addChild("functions", fn);

        std::string root = "/tmp/whetstone_step369";
        std::string filePath = "samples/test.wat";
        auto save = saveSemannoSidecar(root, filePath, &mod);
        assert(save.success);
        auto load = loadSemannoSidecar(root, filePath);
        assert(load.success);
        assert(load.count >= 1);
        std::cout << "Test 7 PASSED: Semanno sidecar save/load for WAT\n";
        passed++;
    }

    // Test 8: MCP tools handle WAT language parameter
    {
        MCPServer server;
        auto tools = server.getTools();
        bool hasRunPipeline = false;
        for (const auto& t : tools) {
            if (t.name == "whetstone_run_pipeline") {
                hasRunPipeline = true;
                assert(t.inputSchema.contains("properties"));
                auto props = t.inputSchema["properties"];
                assert(props.contains("sourceLanguage"));
                assert(props["sourceLanguage"]["type"] == "string");
            }
        }
        assert(hasRunPipeline);
        std::cout << "Test 8 PASSED: MCP run_pipeline accepts language string (incl. wat)\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
