// Step 376: Scheme-Specific Annotations + CL Differentiation (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "AnnotationInference.h"
#include "MemoryStrategyInference.h"
#include "CrossLanguageProjector.h"
#include "Pipeline.h"
#include "ast/SchemeParser.h"
#include "ast/CommonLispParser.h"

static bool hasInference(const std::vector<AnnotationInference::InferredAnnotation>& inferred,
                         const std::string& annotationType,
                         const std::string& value = "") {
    for (const auto& a : inferred) {
        if (a.annotationType != annotationType) continue;
        if (value.empty() || a.value == value) return true;
    }
    return false;
}

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

    // Test 1: Scheme tail recursion -> TailCall
    {
        auto mod = SchemeParser::parseScheme("(define (loopf n) (loopf n))");
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInference(inferred, "TailCallAnnotation"));
        std::cout << "Test 1 PASSED: Scheme tail recursion -> TailCall\n";
        passed++;
    }

    // Test 2: call/cc -> Exec(continuation)
    {
        auto mod = SchemeParser::parseScheme("(define (cc f) (call/cc f))");
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInference(inferred, "ExecAnnotation", "continuation"));
        std::cout << "Test 2 PASSED: call/cc -> Exec(continuation)\n";
        passed++;
    }

    // Test 3: define-syntax -> Meta(hygienic)
    {
        auto mod = SchemeParser::parseScheme("(define-syntax m (syntax-rules () ((_ x) x)))");
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInference(inferred, "MetaAnnotation", "hygienic"));
        std::cout << "Test 3 PASSED: define-syntax -> Meta(hygienic)\n";
        passed++;
    }

    // Test 4: CL -> Scheme projection path works
    {
        Pipeline p;
        auto res = p.run("(defun id (x) x)", "common-lisp", "scheme");
        assert(res.success);
        assert(!res.generatedCode.empty());
        std::cout << "Test 4 PASSED: CL -> Scheme projection path\n";
        passed++;
    }

    // Test 5: Scheme -> CL projection path works
    {
        Pipeline p;
        auto res = p.run("(define (id x) x)", "scheme", "common-lisp");
        assert(res.success);
        assert(!res.generatedCode.empty());
        std::cout << "Test 5 PASSED: Scheme -> CL projection path\n";
        passed++;
    }

    // Test 6: both Scheme and CL use GC defaults
    {
        MemoryStrategyInference mem;
        Module scm("m1", "scm", "scheme");
        Module cl("m2", "cl", "common-lisp");
        auto s1 = mem.inferAnnotations(&scm);
        auto s2 = mem.inferAnnotations(&cl);
        assert(hasMemory(s1, "ReclaimAnnotation", "Tracing"));
        assert(hasMemory(s2, "ReclaimAnnotation", "Tracing"));
        std::cout << "Test 6 PASSED: Scheme/CL GC defaults\n";
        passed++;
    }

    // Test 7: differentiate @Meta between CL and Scheme macros
    {
        auto cl = CommonLispParser::parseCommonLisp("(defmacro m (x) x)");
        auto scm = SchemeParser::parseScheme("(define-syntax m (syntax-rules () ((_ x) x)))");
        AnnotationInference inf;
        auto clInf = inf.inferAll(cl.get());
        auto scmInf = inf.inferAll(scm.get());
        assert(hasInference(clInf, "MetaAnnotation", "quoted"));
        assert(hasInference(scmInf, "MetaAnnotation", "hygienic"));
        std::cout << "Test 7 PASSED: differentiated macro meta states\n";
        passed++;
    }

    // Test 8: differentiate @Binding types
    {
        auto cl = CommonLispParser::parseCommonLisp("(defvar *special* 1)");
        auto scm = SchemeParser::parseScheme("(define (mk) (make-parameter 42))");
        AnnotationInference inf;
        auto clInf = inf.inferAll(cl.get());
        auto scmInf = inf.inferAll(scm.get());
        assert(hasInference(clInf, "BindingAnnotation", "dynamic"));
        assert(hasInference(scmInf, "BindingAnnotation", "parameter"));
        std::cout << "Test 8 PASSED: differentiated binding types\n";
        passed++;
    }

    // Test 9: differentiate @Exception types
    {
        auto cl = CommonLispParser::parseCommonLisp("(defun f () (handler-case x))");
        auto scm = SchemeParser::parseScheme("(define (g) (guard (e (#t e)) e))");
        AnnotationInference inf;
        auto clInf = inf.inferAll(cl.get());
        auto scmInf = inf.inferAll(scm.get());
        assert(hasInference(clInf, "ExceptionAnnotation", "condition"));
        assert(hasInference(scmInf, "ExceptionAnnotation", "guard"));
        std::cout << "Test 9 PASSED: differentiated exception styles\n";
        passed++;
    }

    // Test 10: cross-language annotation fidelity Scheme <-> CL
    {
        Module mod("m1", "scheme", "scheme");
        auto* fn = new Function("f1", "cc");
        auto* ex = new ExecAnnotation();
        ex->id = "a1";
        ex->mode = "continuation";
        fn->addChild("annotations", ex);
        mod.addChild("functions", fn);
        auto* mac = new MacroDefinition("m1", "mac");
        auto* meta = new MetaAnnotation();
        meta->id = "a2";
        meta->state = "hygienic";
        mac->addChild("annotations", meta);
        mod.addChild("statements", mac);

        CrossLanguageProjector projector;
        auto cl = projector.project(&mod, "common-lisp");
        auto back = projector.project(cl.get(), "scheme");
        assert(projector.annotationsPreserved(&mod, back.get()));
        std::cout << "Test 10 PASSED: annotation fidelity across Scheme/CL\n";
        passed++;
    }

    // Test 11: Scheme tail recursion also maps to Loop(tail-recursive)
    {
        auto mod = SchemeParser::parseScheme("(define (loopf n) (loopf n))");
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInference(inferred, "LoopAnnotation", "tail-recursive"));
        std::cout << "Test 11 PASSED: Scheme loop tail-recursive signal\n";
        passed++;
    }

    // Test 12: confidence values remain bounded
    {
        auto mod = SchemeParser::parseScheme(
            "(define-syntax m (syntax-rules () ((_ x) x)))\n"
            "(define (cc f) (call/cc f))\n");
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        bool strong = false;
        for (const auto& a : inferred) {
            assert(a.confidence >= 0.0 && a.confidence <= 1.0);
            if (a.confidence >= 0.85) strong = true;
        }
        assert(strong);
        std::cout << "Test 12 PASSED: confidence bounds\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
