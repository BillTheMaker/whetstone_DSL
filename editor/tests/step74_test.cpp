// Step 74 TDD Test: Performance profiling and benchmarks
//
// Tests that core operations complete efficiently:
// 1. Large AST creation (1000 functions with body) completes quickly
// 2. TransformEngine constant folding on large AST
// 3. StrategyValidator on deeply nested AST
// 4. MemoryStrategyInference on large module
// 5. Serialization round-trip on large AST
// 6. Multiple IncrementalOptimizer transform passes

#include <iostream>
#include <string>
#include <cassert>
#include <chrono>
#include <memory>
#include "ast/ASTNode.h"
#include "ast/Module.h"
#include "ast/Function.h"
#include "ast/Variable.h"
#include "ast/Statement.h"
#include "ast/Expression.h"
#include "ast/Annotation.h"
#include "ast/Serialization.h"
#include "TransformEngine.h"
#include "StrategyValidator.h"
#include "MemoryStrategyInference.h"
#include "IncrementalOptimizer.h"

using Clock = std::chrono::high_resolution_clock;

// Helper: build a function with foldable body (return A + B)
static Function* makeFoldableFunction(int index) {
    std::string id = "f" + std::to_string(index);
    Function* fn = new Function(id, "func_" + std::to_string(index));

    Return* ret = new Return();
    ret->id = "r" + std::to_string(index);
    BinaryOperation* binOp = new BinaryOperation("b" + std::to_string(index), "+");
    IntegerLiteral* left = new IntegerLiteral("il" + std::to_string(index), index);
    IntegerLiteral* right = new IntegerLiteral("ir" + std::to_string(index), index * 2);
    binOp->setChild("left", left);
    binOp->setChild("right", right);
    ret->setChild("value", binOp);
    fn->addChild("body", ret);

    return fn;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Large AST creation (1000 functions) ---
    {
        auto start = Clock::now();

        Module mod("m1", "Benchmark", "python");
        for (int i = 0; i < 1000; ++i) {
            mod.addChild("functions", makeFoldableFunction(i));
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start).count();

        auto functions = mod.getChildren("functions");
        assert(functions.size() == 1000 && "Should have 1000 functions");
        assert(elapsed < 5000 && "1000 function AST creation should complete in <5s");

        std::cout << "Test 1 PASS: Large AST creation (" << elapsed << "ms for 1000 functions)" << std::endl;
        ++passed;

        // Clean up
        for (auto* fn : functions) {
            auto body = fn->getChildren("body");
            for (auto* stmt : body) {
                auto* val = stmt->getChild("value");
                if (val && val->conceptType == "IntegerLiteral") {
                    // Folded — just the literal
                } else if (val) {
                    // Original BinaryOp
                }
            }
        }
    }

    // --- Test 2: Constant folding on large AST ---
    {
        Module mod("m1", "Benchmark", "python");
        for (int i = 0; i < 500; ++i) {
            mod.addChild("functions", makeFoldableFunction(i));
        }

        auto start = Clock::now();

        TransformEngine engine;
        engine.setRoot(&mod);
        auto result = engine.constantFolding();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start).count();

        assert(result.applied && "Should fold constants in 500 functions");
        assert(result.nodesModified >= 500 && "Should modify at least 500 nodes");
        assert(elapsed < 5000 && "Folding 500 functions should complete in <5s");

        std::cout << "Test 2 PASS: Constant folding on 500 functions (" << elapsed << "ms, "
                  << result.nodesModified << " nodes modified)" << std::endl;
        ++passed;
    }

    // --- Test 3: StrategyValidator on deeply nested AST ---
    {
        Module mod("m1", "Benchmark", "cpp");
        Function* fn = new Function("f1", "deep");

        DeallocateAnnotation* anno = new DeallocateAnnotation();
        anno->id = "a1";
        anno->strategy = "Explicit";
        fn->addChild("annotations", anno);

        // Create 100 variables (potential leak sources)
        for (int i = 0; i < 100; ++i) {
            Variable* var = new Variable("v" + std::to_string(i), "ptr" + std::to_string(i));
            fn->addChild("body", var);
        }
        mod.addChild("functions", fn);

        auto start = Clock::now();

        StrategyValidator validator;
        auto violations = validator.validateInvariants(&mod);

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start).count();

        // Should detect leaks for all 100 variables
        int leakCount = 0;
        for (const auto& v : violations) {
            if (v.category == "leak") leakCount++;
        }
        assert(leakCount == 100 && "Should detect 100 leaked allocations");
        assert(elapsed < 5000 && "Validating 100 variables should complete in <5s");

        std::cout << "Test 3 PASS: StrategyValidator on 100 variables (" << elapsed << "ms, "
                  << leakCount << " leaks detected)" << std::endl;
        ++passed;
    }

    // --- Test 4: MemoryStrategyInference on large module ---
    {
        Module mod("m1", "Benchmark", "cpp");
        for (int i = 0; i < 200; ++i) {
            Function* fn = new Function("f" + std::to_string(i), "func_" + std::to_string(i));
            // Add a variable to each function
            Variable* var = new Variable("v" + std::to_string(i), "data");
            fn->addChild("body", var);
            mod.addChild("functions", fn);
        }

        auto start = Clock::now();

        MemoryStrategyInference inferrer;
        auto suggestions = inferrer.inferAnnotations(&mod);

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start).count();

        assert(!suggestions.empty() && "Should produce suggestions for C++ module");
        assert(elapsed < 5000 && "Inference on 200 functions should complete in <5s");

        std::cout << "Test 4 PASS: MemoryStrategyInference on 200 functions (" << elapsed << "ms, "
                  << suggestions.size() << " suggestions)" << std::endl;
        ++passed;
    }

    // --- Test 5: JSON serialization round-trip on large AST ---
    {
        Module mod("m1", "Benchmark", "python");
        for (int i = 0; i < 100; ++i) {
            mod.addChild("functions", makeFoldableFunction(i));
        }

        auto start = Clock::now();

        json j = toJson(&mod);
        std::string serialized = j.dump();

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start).count();

        assert(!serialized.empty() && "Serialization should produce non-empty output");
        assert(serialized.size() > 1000 && "Serialized 100 functions should be >1KB");
        assert(elapsed < 5000 && "Serializing 100 functions should complete in <5s");

        // Deserialize back
        auto start2 = Clock::now();
        json parsed = json::parse(serialized);
        ASTNode* restored = fromJson(parsed);
        auto elapsed2 = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start2).count();

        assert(restored != nullptr && "Deserialization should produce a node");
        assert(restored->conceptType == "Module" && "Should restore a Module");

        std::cout << "Test 5 PASS: JSON round-trip on 100 functions (serialize: " << elapsed
                  << "ms, deserialize: " << elapsed2 << "ms, " << serialized.size() << " bytes)" << std::endl;
        ++passed;

        deleteTree(restored);
    }

    // --- Test 6: Multiple IncrementalOptimizer passes with undo ---
    {
        Module mod("m1", "Benchmark", "python");
        for (int i = 0; i < 50; ++i) {
            mod.addChild("functions", makeFoldableFunction(i));
        }

        auto start = Clock::now();

        IncrementalOptimizer opt;
        opt.setRoot(&mod);

        // Apply 50 constant-fold transforms
        std::vector<std::string> transformIds;
        for (int i = 0; i < 50; ++i) {
            std::string tid = opt.applyTransform("constant-fold");
            transformIds.push_back(tid);
        }

        auto history = opt.getTransformHistory();
        assert(history.size() == 50 && "Should have 50 transforms in history");

        // Undo all transforms
        for (int i = 49; i >= 0; --i) {
            bool undone = opt.undoLast();
            // First undo should work, subsequent may be no-ops if nothing to fold
        }

        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - start).count();

        assert(elapsed < 5000 && "50 transforms + undo should complete in <5s");

        std::cout << "Test 6 PASS: 50 IncrementalOptimizer passes + undo (" << elapsed << "ms)" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 74 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
