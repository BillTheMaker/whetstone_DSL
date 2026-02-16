// Step 367: WAT Annotation Mapping (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "MemoryStrategyInference.h"
#include "AnnotationInference.h"
#include "CrossLanguageProjector.h"
#include "SemannoFormat.h"
#include "Pipeline.h"
#include "ast/WatParser.h"

static bool hasMemorySuggestion(const std::vector<MemoryStrategyInference::Suggestion>& suggestions,
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

    // Test 1: WAT function gets Exec(stack)
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("functions", new Function("f1", "run"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "ExecAnnotation", "stack"));
        std::cout << "Test 1 PASSED: function -> Exec(stack)\n";
        passed++;
    }

    // Test 2: memory op gets Owner(Manual)
    {
        Module mod("m1", "wat", "wat");
        auto* fn = new Function("f1", "grow");
        auto* call = new FunctionCall();
        call->id = "c1";
        call->functionName = "memory.grow";
        fn->addChild("body", call);
        mod.addChild("functions", fn);

        MemoryStrategyInference mem;
        auto suggestions = mem.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "OwnerAnnotation", "Manual"));
        std::cout << "Test 2 PASSED: memory op -> Owner(Manual)\n";
        passed++;
    }

    // Test 3: export -> Visibility(public)
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("statements", new PragmaDirective("p1", "export"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "VisibilityAnnotation", "public"));
        std::cout << "Test 3 PASSED: export -> Visibility(public)\n";
        passed++;
    }

    // Test 4: import -> Link(import)
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("imports", new Import("i1", "env.print", "runtime", ""));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "LinkAnnotation", "import"));
        std::cout << "Test 4 PASSED: import -> Link(import)\n";
        passed++;
    }

    // Test 5: WAT defaults applied
    {
        Module mod("m1", "wat", "wat");
        MemoryStrategyInference mem;
        auto suggestions = mem.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "OwnerAnnotation", "Manual"));
        assert(hasMemorySuggestion(suggestions, "ReclaimAnnotation", "Explicit"));
        std::cout << "Test 5 PASSED: WAT defaults applied\n";
        passed++;
    }

    // Test 6: table dispatch -> Shim(vtable)
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("statements", new PragmaDirective("p1", "table"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "ShimAnnotation", "vtable"));
        std::cout << "Test 6 PASSED: table -> Shim(vtable)\n";
        passed++;
    }

    // Test 7: memory declarations -> Align + Layout
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("statements", new PragmaDirective("p1", "memory"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInference(inferred, "AlignAnnotation", "4"));
        assert(hasInference(inferred, "LayoutAnnotation", "linear"));
        std::cout << "Test 7 PASSED: memory -> Align/Layout\n";
        passed++;
    }

    // Test 8: confidence values stay bounded
    {
        Module mod("m1", "wat", "wat");
        mod.addChild("functions", new Function("f1", "run"));
        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        bool foundStrong = false;
        for (const auto& a : inferred) {
            assert(a.confidence >= 0.0 && a.confidence <= 1.0);
            if (a.confidence >= 0.75) foundStrong = true;
        }
        assert(foundStrong);
        std::cout << "Test 8 PASSED: confidence bounded and useful\n";
        passed++;
    }

    // Test 9: WAT annotations preserved projecting to C
    {
        Module mod("m1", "wat", "wat");
        auto* fn = new Function("f1", "run");
        auto* exec = new ExecAnnotation();
        exec->id = "a1";
        exec->mode = "stack";
        fn->addChild("annotations", exec);
        mod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto cMod = projector.project(&mod, "c");
        assert(projector.annotationsPreserved(&mod, cMod.get()));
        std::cout << "Test 9 PASSED: projection preserves WAT annotations\n";
        passed++;
    }

    // Test 10: Semanno roundtrip for Exec(stack)
    {
        ExecAnnotation exec;
        exec.id = "a1";
        exec.mode = "stack";
        exec.runtimeHint = "wasm";
        std::string line = SemannoEmitter::emit(&exec);
        auto parsed = SemannoParser::parse(line);
        assert(parsed.type == "exec");
        assert(parsed.properties["mode"] == "stack");
        std::cout << "Test 10 PASSED: semanno roundtrip\n";
        passed++;
    }

    // Test 11: pipeline parse wat + infer stack annotation
    {
        Pipeline p;
        std::vector<ParseDiagnostic> diags;
        auto mod = p.parse("(module (func $f (result i32)))", "wat", diags);
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInference(inferred, "ExecAnnotation", "stack"));
        std::cout << "Test 11 PASSED: pipeline wat parse supports inference\n";
        passed++;
    }

    // Test 12: annotation survives wat -> c -> wat projection
    {
        Module mod("m1", "wat", "wat");
        auto* fn = new Function("f1", "run");
        auto* vis = new VisibilityAnnotation();
        vis->id = "v1";
        vis->level = "public";
        fn->addChild("annotations", vis);
        mod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto cMod = projector.project(&mod, "c");
        auto watBack = projector.project(cMod.get(), "wat");
        assert(projector.annotationsPreserved(&mod, watBack.get()));
        std::cout << "Test 12 PASSED: annotation roundtrip through projection\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
