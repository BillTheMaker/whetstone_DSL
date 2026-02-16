// Step 377: Phase 14d Integration + Sprint 14 Summary (8 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "Pipeline.h"
#include "CrossLanguageProjector.h"
#include "MemoryStrategyInference.h"
#include "AnnotationInference.h"
#include "ast/CGenerator.h"
#include "ast/WatGenerator.h"
#include "ast/CommonLispGenerator.h"
#include "ast/SchemeGenerator.h"

static bool hasMemory(const std::vector<MemoryStrategyInference::Suggestion>& suggestions,
                      const std::string& annotationType,
                      const std::string& strategy) {
    for (const auto& s : suggestions) {
        if (s.annotationType == annotationType && s.strategy == strategy) return true;
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: 14-language parse+generate smoke
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
            {"wat", "(module (func $f (param $x i32) (result i32)))"},
            {"common-lisp", "(defun f (x) x)"},
            {"scheme", "(define (f x) x)"}
        };
        Pipeline p;
        int ok = 0;
        for (const auto& s : samples) {
            std::vector<ParseDiagnostic> diags;
            auto mod = p.parse(s.src, s.lang, diags);
            if (!mod) continue;
            auto out = p.generate(mod.get(), s.lang);
            if (!out.empty()) ok++;
        }
        assert(ok == 14);
        std::cout << "Test 1 PASSED: all 14 language parse/generate routes\n";
        passed++;
    }

    // Test 2: C <-> WAT <-> CL <-> Scheme projection matrix
    {
        Pipeline p;
        std::vector<std::string> langs = {"c", "wat", "common-lisp", "scheme"};
        std::vector<std::string> srcs = {
            "int f(int x){ return x; }",
            "(module (func $f (param $x i32) (result i32)))",
            "(defun f (x) x)",
            "(define (f x) x)"
        };
        int ok = 0;
        for (size_t i = 0; i < langs.size(); ++i) {
            for (size_t j = 0; j < langs.size(); ++j) {
                if (i == j) continue;
                auto res = p.run(srcs[i], langs[i], langs[j]);
                if (res.success && !res.generatedCode.empty()) ok++;
            }
        }
        assert(ok == 12);
        std::cout << "Test 2 PASSED: 4-language cross projection matrix\n";
        passed++;
    }

    // Test 3: Lisp source -> WAT target
    {
        Pipeline p;
        auto res1 = p.run("(defun add (x y) (+ x y))", "common-lisp", "wat");
        auto res2 = p.run("(define (id x) x)", "scheme", "wat");
        assert(res1.success && res2.success);
        assert(res1.generatedCode.find("(module") != std::string::npos);
        assert(res2.generatedCode.find("(module") != std::string::npos);
        std::cout << "Test 3 PASSED: Lisp -> WAT integration\n";
        passed++;
    }

    // Test 4: C -> Common Lisp memory model translation path
    {
        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        auto cMod = p.parse("int* alloc_buf(){ return 0; }", "c", diags);
        assert(cMod != nullptr);
        CrossLanguageProjector projector;
        auto clMod = projector.project(cMod.get(), "common-lisp");
        MemoryStrategyInference mem;
        auto suggestions = mem.inferAnnotations(clMod.get());
        assert(hasMemory(suggestions, "ReclaimAnnotation", "Tracing"));
        assert(hasMemory(suggestions, "OwnerAnnotation", "Shared_GC"));
        std::cout << "Test 4 PASSED: C -> CL memory defaults translation\n";
        passed++;
    }

    // Test 5: Semanno comment prefixes for C/WAT/CL/Scheme
    {
        Function fn("f1", "annotated");
        fn.addChild("annotations", new OwnerAnnotation("a1", "Manual"));
        CGenerator cGen;
        WatGenerator watGen;
        CommonLispGenerator clGen;
        SchemeGenerator scmGen;
        auto cOut = cGen.generate(&fn);
        auto watOut = watGen.generate(&fn);
        auto clOut = clGen.generate(&fn);
        auto scmOut = scmGen.generate(&fn);
        assert(cOut.find("// @owner(") != std::string::npos);
        assert(watOut.find(";; @owner(") != std::string::npos);
        assert(clOut.find(";; @owner(") != std::string::npos);
        assert(scmOut.find("; @owner(") != std::string::npos);
        std::cout << "Test 5 PASSED: semanno prefixes for 4 new languages\n";
        passed++;
    }

    // Test 6: language-appropriate default inference for C/WAT/CL/Scheme
    {
        MemoryStrategyInference mem;
        Module c("m1", "c", "c");
        Module wat("m2", "wat", "wat");
        Module cl("m3", "cl", "common-lisp");
        Module scm("m4", "scm", "scheme");
        auto cS = mem.inferAnnotations(&c);
        auto wS = mem.inferAnnotations(&wat);
        auto clS = mem.inferAnnotations(&cl);
        auto scmS = mem.inferAnnotations(&scm);
        assert(hasMemory(cS, "OwnerAnnotation", "Manual"));
        assert(hasMemory(wS, "OwnerAnnotation", "Manual"));
        assert(hasMemory(clS, "ReclaimAnnotation", "Tracing"));
        assert(hasMemory(scmS, "ReclaimAnnotation", "Tracing"));
        std::cout << "Test 6 PASSED: defaults by language\n";
        passed++;
    }

    // Test 7: Pipeline.run succeeds with each of the 4 new languages as source
    {
        Pipeline p;
        auto c = p.run("int f(){ return 1; }", "c", "scheme");
        auto wat = p.run("(module (func $f (result i32)))", "wat", "common-lisp");
        auto cl = p.run("(defun f () 1)", "common-lisp", "c");
        auto scm = p.run("(define (f) 1)", "scheme", "wat");
        assert(c.success && wat.success && cl.success && scm.success);
        std::cout << "Test 7 PASSED: Pipeline.run for all 4 new language sources\n";
        passed++;
    }

    // Test 8: Sprint 14 totals sanity (14 routes + matrix validity)
    {
        std::vector<std::string> supported = {
            "python","cpp","elisp","javascript","typescript","java","rust","go",
            "kotlin","csharp","c","wat","common-lisp","scheme"
        };
        assert(supported.size() == 14);

        Pipeline p;
        auto a = p.run("(define (f x) x)", "scheme", "common-lisp");
        auto b = p.run("(defun f (x) x)", "common-lisp", "wat");
        auto c = p.run("(module (func $f (param $x i32) (result i32)))", "wat", "c");
        assert(a.success && b.success && c.success);
        std::cout << "Test 8 PASSED: sprint 14 integration totals sanity\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/8\n";
    assert(passed == 8);
    return 0;
}
