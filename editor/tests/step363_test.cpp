// Step 363: C Memory Annotation Mapping (12 tests)

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include "MemoryStrategyInference.h"
#include "AnnotationInference.h"
#include "CrossLanguageProjector.h"
#include "ast/CParser.h"

static bool hasMemorySuggestion(
    const std::vector<MemoryStrategyInference::Suggestion>& suggestions,
    const std::string& nodeId,
    const std::string& annotationType,
    const std::string& strategy) {
    for (const auto& s : suggestions) {
        if (s.nodeId == nodeId && s.annotationType == annotationType && s.strategy == strategy) {
            return true;
        }
    }
    return false;
}

static bool hasInferred(
    const std::vector<AnnotationInference::InferredAnnotation>& inferred,
    const std::string& nodeId,
    const std::string& annotationType,
    const std::string& value) {
    for (const auto& a : inferred) {
        if (a.nodeId == nodeId && a.annotationType == annotationType && a.value == value) {
            return true;
        }
    }
    return false;
}

int main() {
    int passed = 0;

    // Test 1: C defaults applied on module
    {
        Module mod("m1", "CDefaults", "c");
        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "m1", "OwnerAnnotation", "Manual"));
        assert(hasMemorySuggestion(suggestions, "m1", "LifetimeAnnotation", "Scope"));
        assert(hasMemorySuggestion(suggestions, "m1", "ReclaimAnnotation", "Explicit"));
        std::cout << "Test 1 PASSED: c defaults owner/lifetime/reclaim\n";
        passed++;
    }

    // Test 2: malloc pattern -> Owner(Manual) + Reclaim(Explicit)
    {
        Module mod("m1", "CMalloc", "c");
        Function fn("f1", "alloc_buf");
        FunctionCall* mallocCall = new FunctionCall();
        mallocCall->id = "c1";
        mallocCall->functionName = "malloc";
        fn.addChild("body", mallocCall);
        mod.addChild("functions", &fn);

        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "f1", "OwnerAnnotation", "Manual"));
        assert(hasMemorySuggestion(suggestions, "f1", "ReclaimAnnotation", "Explicit"));
        std::cout << "Test 2 PASSED: malloc infers manual+explicit\n";
        passed++;
    }

    // Test 3: local variable -> Lifetime(Scope)
    {
        Module mod("m1", "CLocal", "c");
        Function fn("f1", "work");
        Variable* v = new Variable("v1", "x");
        v->setChild("type", new PrimitiveType("t1", "int"));
        fn.addChild("body", v);
        mod.addChild("functions", &fn);

        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "v1", "LifetimeAnnotation", "Scope"));
        std::cout << "Test 3 PASSED: local variable infers scope lifetime\n";
        passed++;
    }

    // Test 4: pointer parameter -> Owner(Borrowed)
    {
        Module mod("m1", "CParam", "c");
        Function fn("f1", "consume");
        Parameter* p = new Parameter("p1", "ptr");
        p->setChild("type", new PrimitiveType("t1", "int*"));
        fn.addChild("parameters", p);
        mod.addChild("functions", &fn);

        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "p1", "OwnerAnnotation", "Borrowed"));
        std::cout << "Test 4 PASSED: pointer parameter infers borrowed owner\n";
        passed++;
    }

    // Test 5: return pointer convention -> Borrowed for get* names
    {
        Module mod("m1", "CReturn", "c");
        Function fn("f1", "get_buffer");
        fn.setChild("returnType", new PrimitiveType("t1", "char*"));
        mod.addChild("functions", &fn);

        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "f1", "OwnerAnnotation", "Borrowed"));
        std::cout << "Test 5 PASSED: get* pointer return infers borrowed owner\n";
        passed++;
    }

    // Test 6: static variable -> Lifetime(Static)
    {
        Module mod("m1", "CStatic", "c");
        Function fn("f1", "counter");
        Variable* v = new Variable("v1", "counter");
        v->setChild("type", new PrimitiveType("t1", "static int"));
        fn.addChild("body", v);
        mod.addChild("functions", &fn);

        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        assert(hasMemorySuggestion(suggestions, "v1", "LifetimeAnnotation", "Static"));
        std::cout << "Test 6 PASSED: static variable infers static lifetime\n";
        passed++;
    }

    // Test 7: goto usage -> Complexity(high) + Risk(medium)
    {
        Module mod("m1", "CGoto", "c");
        Function fn("f1", "jumping");
        FunctionCall* go = new FunctionCall();
        go->id = "g1";
        go->functionName = "goto";
        fn.addChild("body", go);
        mod.addChild("functions", &fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInferred(inferred, "f1", "ComplexityAnnotation", "high"));
        assert(hasInferred(inferred, "f1", "RiskAnnotation", "medium"));
        std::cout << "Test 7 PASSED: goto infers complexity/risk signals\n";
        passed++;
    }

    // Test 8: void* cast pattern -> Risk(high) + Ambiguity(medium)
    {
        Module mod("m1", "CVoid", "c");
        Function fn("f1", "casty");
        Variable* v = new Variable("v1", "raw");
        v->setChild("type", new PrimitiveType("t1", "void*"));
        fn.addChild("body", v);
        mod.addChild("functions", &fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInferred(inferred, "f1", "RiskAnnotation", "high"));
        assert(hasInferred(inferred, "f1", "AmbiguityAnnotation", "medium"));
        std::cout << "Test 8 PASSED: void* infers risk+ambiguity\n";
        passed++;
    }

    // Test 9: array access without checks -> BoundsCheck(unchecked)
    {
        Module mod("m1", "CArray", "c");
        Function fn("f1", "idx");
        IndexAccess* idx = new IndexAccess();
        idx->id = "i1";
        idx->setChild("target", new VariableReference("vr1", "arr"));
        idx->setChild("index", new VariableReference("vr2", "i"));
        ExpressionStatement* stmt = new ExpressionStatement();
        stmt->id = "s1";
        stmt->setChild("expression", idx);
        fn.addChild("body", stmt);
        mod.addChild("functions", &fn);

        AnnotationInference inf;
        auto inferred = inf.inferAll(&mod);
        assert(hasInferred(inferred, "f1", "BoundsCheckAnnotation", "unchecked"));
        std::cout << "Test 9 PASSED: unchecked array access inferred\n";
        passed++;
    }

    // Test 10: header guard pattern -> Synthetic(guard)
    {
        std::string source = "#ifndef SAMPLE_H\n#define SAMPLE_H\nint x;\n#endif\n";
        auto mod = CParser::parseC(source);
        AnnotationInference inf;
        auto inferred = inf.inferAll(mod.get());
        assert(hasInferred(inferred, mod->id, "SyntheticAnnotation", "guard"));
        std::cout << "Test 10 PASSED: header guard infers synthetic marker\n";
        passed++;
    }

    // Test 11: inference confidence scores remain reasonable
    {
        Module mod("m1", "CConfidence", "c");
        Function fn("f1", "alloc");
        FunctionCall* mallocCall = new FunctionCall();
        mallocCall->id = "c1";
        mallocCall->functionName = "malloc";
        fn.addChild("body", mallocCall);
        mod.addChild("functions", &fn);

        MemoryStrategyInference inf;
        auto suggestions = inf.inferAnnotations(&mod);
        bool foundHigh = false;
        for (const auto& s : suggestions) {
            assert(s.confidence >= 0.0 && s.confidence <= 1.0);
            if (s.confidence >= 0.70) foundHigh = true;
        }
        assert(foundHigh);
        std::cout << "Test 11 PASSED: confidence values are bounded and useful\n";
        passed++;
    }

    // Test 12: cross-language C ownership adaptation to Rust
    {
        Module mod("m1", "COwnership", "c");
        Function* fn = new Function("f1", "transfer");
        auto* fnOwner = new OwnerAnnotation("a1", "Manual");
        fn->addChild("annotations", fnOwner);

        Parameter* p = new Parameter("p1", "src");
        p->setChild("type", new PrimitiveType("t1", "int*"));
        auto* paramOwner = new OwnerAnnotation("a2", "Borrowed");
        p->addChild("annotations", paramOwner);
        fn->addChild("parameters", p);
        mod.addChild("functions", fn);

        CrossLanguageProjector projector;
        auto rustMod = projector.project(&mod, "rust");
        auto* rustFn = static_cast<Function*>(rustMod->getChildren("functions")[0]);

        bool hasBoxOwner = false;
        for (auto* anno : rustFn->getChildren("annotations")) {
            if (anno->conceptType == "OwnerAnnotation") {
                auto* owner = static_cast<OwnerAnnotation*>(anno);
                if (owner->strategy == "Box") hasBoxOwner = true;
            }
        }
        assert(hasBoxOwner);

        auto* rustParam = static_cast<Parameter*>(rustFn->getChildren("parameters")[0]);
        auto* rustType = rustParam->getChild("type");
        assert(rustType != nullptr);
        assert(rustType->conceptType == "PrimitiveType");
        auto* rustPrim = static_cast<PrimitiveType*>(rustType);
        assert(!rustPrim->kind.empty() && rustPrim->kind[0] == '&');

        std::cout << "Test 12 PASSED: C owner/manual->Box and borrowed pointer->&T style\n";
        passed++;
    }

    std::cout << "\nResults: " << passed << "/12\n";
    assert(passed == 12);
    return 0;
}
