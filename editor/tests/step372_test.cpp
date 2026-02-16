// Step 372: Lisp Annotation Mapping (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "AnnotationInference.h"
#include "MemoryStrategyInference.h"
#include "CrossLanguageProjector.h"
#include "ast/PythonGenerator.h"
#include "ast/CppGenerator.h"
#include "ast/ClassDeclaration.h"

static bool hasInference(const std::vector<AnnotationInference::InferredAnnotation>& inferred,
                         const std::string& annotationType,
                         const std::string& value) {
    for (const auto& a : inferred) {
        if (a.annotationType == annotationType && a.value == value) return true;
    }
    return false;
}

static bool hasMemorySuggestion(const std::vector<MemoryStrategyInference::Suggestion>& suggestions,
                                const std::string& annotationType,
                                const std::string& strategy) {
    for (const auto& s : suggestions) {
        if (s.annotationType == annotationType && s.strategy == strategy) return true;
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: macro -> @Meta(quoted)
    {
        Module mod("m1", "cl", "common-lisp");
        mod.addChild("statements", new MacroDefinition("mac1", "when1"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "MetaAnnotation", "quoted"));
        std::cout << "Test 1 PASSED: macro -> Meta(quoted)\n";
        passed++;
    }

    // Test 2: dynamic variable -> @Binding(dynamic)
    {
        Module mod("m1", "cl", "common-lisp");
        mod.addChild("variables", new Variable("v1", "*runtime-mode*"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "BindingAnnotation", "dynamic"));
        std::cout << "Test 2 PASSED: dynamic variable -> Binding(dynamic)\n";
        passed++;
    }

    // Test 3: values form -> @Contract(multiple-values)
    {
        Module mod("m1", "cl", "common-lisp");
        auto* fn = new Function("f1", "pair");
        auto* valuesCall = new FunctionCall();
        valuesCall->id = "c1";
        valuesCall->functionName = "values";
        valuesCall->addChild("arguments", new IntegerLiteral("i1", 1));
        valuesCall->addChild("arguments", new IntegerLiteral("i2", 2));
        auto* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", valuesCall);
        fn->addChild("body", stmt);
        mod.addChild("functions", fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "ContractAnnotation", "multiple-values"));
        std::cout << "Test 3 PASSED: values -> Contract annotation\n";
        passed++;
    }

    // Test 4: tail-recursive function -> @TailCall
    {
        Module mod("m1", "cl", "common-lisp");
        auto* fn = new Function("f1", "fact");
        auto* call = new FunctionCall();
        call->id = "c1";
        call->functionName = "fact";
        auto* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", call);
        fn->addChild("body", stmt);
        mod.addChild("functions", fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "TailCallAnnotation", ""));
        std::cout << "Test 4 PASSED: tail recursion detected\n";
        passed++;
    }

    // Test 5: CLOS class -> @Visibility(public)
    {
        Module mod("m1", "cl", "common-lisp");
        mod.addChild("classes", new ClassDeclaration("c1", "point"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "VisibilityAnnotation", "public"));
        std::cout << "Test 5 PASSED: CLOS class -> Visibility(public)\n";
        passed++;
    }

    // Test 6: optimize declaration -> @Policy(perf=critical, style=literal)
    {
        Module mod("m1", "cl", "common-lisp");
        auto* fn = new Function("f1", "hot");
        auto* optimize = new FunctionCall();
        optimize->id = "c2";
        optimize->functionName = "optimize";
        auto* decl = new FunctionCall();
        decl->id = "c1";
        decl->functionName = "declare";
        decl->addChild("arguments", optimize);
        auto* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", decl);
        fn->addChild("body", stmt);
        mod.addChild("functions", fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "PolicyAnnotation", "critical"));
        assert(hasInference(inferred, "PolicyAnnotation", "literal"));
        std::cout << "Test 6 PASSED: optimize -> Policy mapping\n";
        passed++;
    }

    // Test 7: CL memory defaults -> GC
    {
        Module mod("m1", "cl", "common-lisp");
        MemoryStrategyInference mem;
        auto suggestions = mem.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "ReclaimAnnotation", "Tracing"));
        assert(hasMemorySuggestion(suggestions, "OwnerAnnotation", "Shared_GC"));
        std::cout << "Test 7 PASSED: CL memory defaults (GC)\n";
        passed++;
    }

    // Test 8: confidence values are bounded and include strong signal
    {
        Module mod("m1", "cl", "common-lisp");
        mod.addChild("statements", new MacroDefinition("mac1", "m"));
        mod.addChild("variables", new Variable("v1", "*x*"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        bool strong = false;
        for (const auto& a : inferred) {
            assert(a.confidence >= 0.0 && a.confidence <= 1.0);
            if (a.confidence >= 0.90) strong = true;
        }
        assert(strong);
        std::cout << "Test 8 PASSED: confidence bounds\n";
        passed++;
    }

    // Test 9: CL @Binding(dynamic) -> Python projection preserves annotation
    {
        Module mod("m1", "cl", "common-lisp");
        auto* fn = new Function("f1", "f");
        auto* b = new BindingAnnotation();
        b->id = "a1";
        b->time = "dynamic";
        fn->addChild("annotations", b);
        mod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto pyMod = projector.project(&mod, "python");
        PythonGenerator gen;
        auto out = gen.generate(pyMod.get());
        assert(projector.annotationsPreserved(&mod, pyMod.get()));
        assert(out.find("def f(") != std::string::npos);
        std::cout << "Test 9 PASSED: binding(dynamic) preserved to Python projection\n";
        passed++;
    }

    // Test 10: CL @Binding(dynamic) -> C++ projection preserves annotation
    {
        Module mod("m1", "cl", "common-lisp");
        auto* fn = new Function("f1", "f");
        auto* b = new BindingAnnotation();
        b->id = "a1";
        b->time = "dynamic";
        fn->addChild("annotations", b);
        mod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto cppMod = projector.project(&mod, "cpp");
        CppGenerator gen;
        auto out = gen.generate(cppMod.get());
        assert(projector.annotationsPreserved(&mod, cppMod.get()));
        assert(out.find("f(") != std::string::npos);
        std::cout << "Test 10 PASSED: binding(dynamic) preserved to C++ projection\n";
        passed++;
    }

    // Test 11: (declare (type ...)) -> Complexity(declared-type)
    {
        Module mod("m1", "cl", "common-lisp");
        auto* fn = new Function("f1", "typed");
        auto* typeCall = new FunctionCall();
        typeCall->id = "c2";
        typeCall->functionName = "type";
        typeCall->addChild("arguments", new VariableReference("r1", "fixnum"));
        auto* decl = new FunctionCall();
        decl->id = "c1";
        decl->functionName = "declare";
        decl->addChild("arguments", typeCall);
        auto* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", decl);
        fn->addChild("body", stmt);
        mod.addChild("functions", fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "ComplexityAnnotation", "declared-type"));
        std::cout << "Test 11 PASSED: declare(type) -> Complexity mapping\n";
        passed++;
    }

    // Test 12: CLOS method -> synthetic method-combination hint
    {
        Module mod("m1", "cl", "common-lisp");
        mod.addChild("functions", new MethodDeclaration("m1", "area"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "SyntheticAnnotation", "clos-method"));
        std::cout << "Test 12 PASSED: CLOS method -> synthetic hint\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
