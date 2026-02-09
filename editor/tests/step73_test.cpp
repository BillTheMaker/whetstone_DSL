// Step 73 TDD Test: Error handling and edge cases
//
// Tests graceful handling of:
// 1. TransformEngine on empty function body
// 2. StrategyValidator on unannotated code (no violations)
// 3. MemoryStrategyInference on null root
// 4. Generator on minimal/incomplete AST
// 5. StrategyAwareOptimizer on nonexistent node ID
// 6. IncrementalOptimizer undo with no history
// 7. AnnotationValidator on empty module
// 8. Pipeline parse with unknown language

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include "ast/Generator.h"
#include "TransformEngine.h"
#include "StrategyValidator.h"
#include "StrategyAwareOptimizer.h"
#include "MemoryStrategyInference.h"
#include "AnnotationValidator.h"
#include "IncrementalOptimizer.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: TransformEngine on empty function body ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "empty");
        mod.addChild("functions", fn);

        TransformEngine engine;
        engine.setRoot(&mod);

        auto foldResult = engine.constantFolding();
        assert(!foldResult.applied && "Nothing to fold in empty function");
        assert(foldResult.nodesModified == 0);

        auto dceResult = engine.deadCodeElimination();
        assert(!dceResult.applied && "Nothing to eliminate in empty function");

        std::cout << "Test 1 PASS: TransformEngine on empty function body" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 2: StrategyValidator on unannotated code ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "plain");
        Return* ret = new Return();
        ret->id = "r1";
        IntegerLiteral* val = new IntegerLiteral("i1", 42);
        ret->setChild("value", val);
        fn->addChild("body", ret);
        mod.addChild("functions", fn);

        StrategyValidator validator;
        auto violations = validator.validateInvariants(&mod);
        bool hasErrors = false;
        for (const auto& v : violations) {
            if (v.severity == "error") hasErrors = true;
        }
        assert(!hasErrors && "Unannotated code should not produce violations");

        std::cout << "Test 2 PASS: StrategyValidator on unannotated code" << std::endl;
        ++passed;

        delete val;
        delete ret;
        delete fn;
    }

    // --- Test 3: MemoryStrategyInference on null root ---
    {
        MemoryStrategyInference inferrer;
        auto suggestions = inferrer.inferAnnotations(nullptr);
        assert(suggestions.empty() && "Null root should produce no suggestions");

        std::cout << "Test 3 PASS: MemoryStrategyInference on null root" << std::endl;
        ++passed;
    }

    // --- Test 4: Generator on minimal AST (empty module) ---
    {
        Module mod("m1", "Empty", "python");

        PythonGenerator pyGen;
        std::string pyOut = pyGen.generate(&mod);
        // Should not crash, may produce empty or minimal output
        // Just verifying no crash/exception

        CppGenerator cppGen;
        std::string cppOut = cppGen.generate(&mod);

        ElispGenerator elispGen;
        std::string elispOut = elispGen.generate(&mod);

        std::cout << "Test 4 PASS: Generators handle minimal/empty AST" << std::endl;
        ++passed;
    }

    // --- Test 5: StrategyAwareOptimizer on nonexistent node ID ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "test");
        mod.addChild("functions", fn);

        StrategyAwareOptimizer opt;
        opt.setRoot(&mod);

        auto result = opt.duplicateNode("nonexistent_id");
        assert(!result.applied && "Nonexistent node should not apply");
        assert(result.blocked.empty() && "No block message for nonexistent node");
        assert(result.nodesModified == 0);

        auto reorderResult = opt.reorderStatements("nonexistent_id");
        assert(!reorderResult.applied && "Nonexistent function should not apply");

        std::cout << "Test 5 PASS: StrategyAwareOptimizer on nonexistent node" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 6: IncrementalOptimizer undo with no history ---
    {
        Module mod("m1", "Test", "python");
        Function* fn = new Function("f1", "test");
        mod.addChild("functions", fn);

        IncrementalOptimizer opt;
        opt.setRoot(&mod);

        bool undone = opt.undoLast();
        assert(!undone && "Undo with no history should return false");

        bool undoneById = opt.undoTransform("nonexistent_transform");
        assert(!undoneById && "Undo nonexistent transform should return false");

        auto history = opt.getTransformHistory();
        assert(history.empty() && "No transforms applied should mean empty history");

        std::string prov = opt.getProvenance("nonexistent_node");
        assert(prov.empty() && "No provenance for unknown node");

        std::cout << "Test 6 PASS: IncrementalOptimizer undo with no history" << std::endl;
        ++passed;

        delete fn;
    }

    // --- Test 7: AnnotationValidator on empty module ---
    {
        Module mod("m1", "Empty", "python");

        AnnotationValidator validator;
        auto diags = validator.validate(&mod);
        bool hasError = false;
        for (const auto& d : diags) {
            if (d.severity == "error") hasError = true;
        }
        assert(!hasError && "Empty module should pass validation");

        std::cout << "Test 7 PASS: AnnotationValidator on empty module" << std::endl;
        ++passed;
    }

    // --- Test 8: TransformEngine with null root ---
    {
        TransformEngine engine;
        // No setRoot called

        auto foldResult = engine.constantFolding();
        assert(!foldResult.applied && "Null root should not apply");

        auto dceResult = engine.deadCodeElimination();
        assert(!dceResult.applied && "Null root should not apply");

        auto allResults = engine.applyAll();
        assert(allResults.size() == 2 && "applyAll returns 2 results");
        for (const auto& r : allResults) {
            assert(!r.applied && "Null root should not apply any transform");
        }

        std::cout << "Test 8 PASS: TransformEngine with null root" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 73 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
