// Step 64 TDD Test: Memory annotation validation
//
// Tests validation of canonical memory annotations:
// 1. Valid @Reclaim(Tracing) passes
// 2. @Deallocate(Explicit) without deallocation point → "Missing Intent" error
// 3. @Owner(Single) on aliased variable → error
// 4. Conflicting parent/child annotations → error
// 5. Valid @Lifetime(RAII) passes

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include "AnnotationValidator.h"

static bool hasDiagnostic(const std::vector<AnnotationValidator::Diagnostic>& diags,
                           const std::string& severity,
                           const std::string& messageFragment) {
    for (const auto& d : diags) {
        if (d.severity == severity && d.message.find(messageFragment) != std::string::npos) {
            return true;
        }
    }
    return false;
}

int main() {
    int passed = 0;
    int failed = 0;

    AnnotationValidator validator;

    // --- Test 1: Valid @Reclaim(Tracing) passes ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "process");
        ReclaimAnnotation* anno = new ReclaimAnnotation();
        anno->id = "a1";
        anno->strategy = "Tracing";
        fn->addChild("annotations", anno);
        mod.addChild("functions", fn);

        auto diags = validator.validate(&mod);

        bool hasError = false;
        for (const auto& d : diags) {
            if (d.severity == "error") hasError = true;
        }
        assert(!hasError && "Valid @Reclaim(Tracing) should produce no errors");

        std::cout << "Test 1 PASS: Valid @Reclaim(Tracing) passes validation" << std::endl;
        ++passed;

        delete anno;
        delete fn;
    }

    // --- Test 2: @Deallocate(Explicit) without deallocation → Missing Intent error ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "leak");
        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        // Function allocates but never deallocates
        Variable* var = new Variable("v1", "ptr");
        fn->addChild("body", var);
        // No delete/free in body → missing deallocation

        mod.addChild("functions", fn);

        auto diags = validator.validate(&mod);
        assert(hasDiagnostic(diags, "error", "Missing Intent") &&
               "@Deallocate(Explicit) without deallocation should produce 'Missing Intent' error");

        std::cout << "Test 2 PASS: @Deallocate(Explicit) without dealloc → Missing Intent error" << std::endl;
        ++passed;

        delete var;
        delete anno;
        delete fn;
    }

    // --- Test 3: @Owner(Single) on aliased variable → error ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "aliasTest");

        OwnerAnnotation* anno = new OwnerAnnotation();
        anno->id = "a1";
        anno->strategy = "Single";
        fn->addChild("annotations", anno);

        // Create a variable that gets aliased (assigned to two targets)
        Variable* original = new Variable("v1", "original");
        fn->addChild("body", original);

        // Assignment: alias = original (creates an alias)
        Assignment* assign = new Assignment();
        assign->id = "as1";
        VariableReference* target = new VariableReference("vr1", "alias");
        VariableReference* value = new VariableReference("vr2", "original");
        assign->setChild("target", target);
        assign->setChild("value", value);
        fn->addChild("body", assign);

        mod.addChild("functions", fn);

        auto diags = validator.validate(&mod);
        assert(hasDiagnostic(diags, "error", "alias") &&
               "@Owner(Single) with aliasing should produce error");

        std::cout << "Test 3 PASS: @Owner(Single) aliasing detected as error" << std::endl;
        ++passed;

        delete value;
        delete target;
        delete assign;
        delete original;
        delete anno;
        delete fn;
    }

    // --- Test 4: Conflicting parent/child annotations → error ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "conflictTest");

        // Parent has @Owner(Single)
        OwnerAnnotation* parentAnno = new OwnerAnnotation();
        parentAnno->id = "a1";
        parentAnno->strategy = "Single";
        fn->addChild("annotations", parentAnno);

        // Child variable has @Owner(Shared_ARC) — conflicts with parent
        Variable* var = new Variable("v1", "child");
        OwnerAnnotation* childAnno = new OwnerAnnotation();
        childAnno->id = "a2";
        childAnno->strategy = "Shared_ARC";
        var->addChild("annotations", childAnno);
        fn->addChild("body", var);

        mod.addChild("functions", fn);

        auto diags = validator.validate(&mod);
        assert(hasDiagnostic(diags, "error", "conflict") &&
               "Conflicting parent/child annotations should produce error");

        std::cout << "Test 4 PASS: Conflicting parent/child annotations detected" << std::endl;
        ++passed;

        delete childAnno;
        delete var;
        delete parentAnno;
        delete fn;
    }

    // --- Test 5: Valid @Lifetime(RAII) with proper scope passes ---
    {
        Module mod("m1", "Test", "cpp");
        Function* fn = new Function("f1", "raiiTest");

        LifetimeAnnotation* anno = new LifetimeAnnotation();
        anno->id = "a1";
        anno->strategy = "RAII";
        fn->addChild("annotations", anno);

        Variable* var = new Variable("v1", "resource");
        fn->addChild("body", var);
        // RAII: destructor handles cleanup automatically at scope end — valid

        mod.addChild("functions", fn);

        auto diags = validator.validate(&mod);
        bool hasError = false;
        for (const auto& d : diags) {
            if (d.severity == "error") hasError = true;
        }
        assert(!hasError && "Valid @Lifetime(RAII) should pass validation");

        std::cout << "Test 5 PASS: Valid @Lifetime(RAII) passes" << std::endl;
        ++passed;

        delete var;
        delete anno;
        delete fn;
    }

    // --- Summary ---
    std::cout << "\n=== Step 64 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
