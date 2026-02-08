// Step 51 TDD Test: Text -> AST synchronization
//
// Tests bidirectional sync between text editing and AST:
// 1. Text change -> re-parse via tree-sitter -> AST updates
// 2. AST change (structured edit) -> regenerate text
// 3. Round-trip: text -> AST -> text produces semantically equivalent output
// 4. Debouncing: rapid text changes don't cause excessive re-parsing
//
// Depends on: TextASTSync, TreeSitterParser, PythonGenerator, CppGenerator

#include <iostream>
#include <string>
#include <cassert>
#include <memory>
#include "TextASTSync.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Set text -> AST updates ---
    {
        TextASTSync sync;
        sync.setText("def greet(name):\n    return name\n", "python");
        sync.syncNow();

        Module* ast = sync.getAST();
        assert(ast != nullptr && "AST should be non-null after setText");

        auto functions = ast->getChildren("functions");
        assert(!functions.empty() && "AST should have parsed functions");

        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "greet" && "Function name should be 'greet'");

        std::cout << "Test 1 PASS: setText -> AST has correct function" << std::endl;
        ++passed;
    }

    // --- Test 2: AST change -> text updates ---
    {
        TextASTSync sync;
        sync.setText("def f(x):\n    return x\n", "python");
        sync.syncNow();

        // Modify AST: change function name
        Module* ast = sync.getAST();
        auto functions = ast->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);
        fn->name = "compute";

        // Trigger text regeneration from modified AST
        // (setAST or a notifyASTChanged method)
        std::string newText = sync.getText();
        assert(contains(newText, "compute") && "Text should reflect renamed function");
        assert(!contains(newText, "def f(") && "Old function name should be gone");

        std::cout << "Test 2 PASS: AST change -> text updates" << std::endl;
        ++passed;
    }

    // --- Test 3: Round-trip text -> AST -> text ---
    {
        std::string original = "def add(a, b):\n    return a + b\n";
        TextASTSync sync;
        sync.setText(original, "python");
        sync.syncNow();

        // Get the regenerated text from the AST
        std::string roundTripped = sync.getText();

        // The output should be semantically equivalent
        assert(contains(roundTripped, "def add") && "Should contain function name");
        assert(contains(roundTripped, "a") && "Should contain parameter a");
        assert(contains(roundTripped, "b") && "Should contain parameter b");
        assert(contains(roundTripped, "return") && "Should contain return");
        assert(contains(roundTripped, "+") && "Should contain operator");

        std::cout << "Test 3 PASS: Round-trip preserves semantics" << std::endl;
        ++passed;
    }

    // --- Test 4: Debounce --- rapid changes don't cause immediate re-parse ---
    {
        TextASTSync sync;
        sync.setText("def a():\n    pass\n", "python");
        // Don't call syncNow --- simulate rapid edits
        sync.setText("def ab():\n    pass\n", "python");
        sync.setText("def abc():\n    pass\n", "python");

        // Parse should be pending but not yet executed
        assert(sync.isParsePending() && "Parse should be pending after rapid edits");

        // Now force sync
        sync.syncNow();
        assert(!sync.isParsePending() && "Parse should no longer be pending after syncNow");

        Module* ast = sync.getAST();
        auto functions = ast->getChildren("functions");
        auto* fn = static_cast<Function*>(functions[0]);
        // Should have the LAST text, not intermediate ones
        assert(fn->name == "abc" && "Should have parsed the final text, not intermediate");

        std::cout << "Test 4 PASS: Debounce coalesces rapid edits" << std::endl;
        ++passed;
    }

    // --- Test 5: C++ text -> AST sync ---
    {
        TextASTSync sync;
        sync.setText("int f(int x) { return x + 1; }", "cpp");
        sync.syncNow();

        Module* ast = sync.getAST();
        assert(ast != nullptr && "AST should be non-null for C++");
        assert(ast->targetLanguage == "cpp" && "Target language should be cpp");

        auto functions = ast->getChildren("functions");
        assert(!functions.empty() && "Should parse C++ function");

        std::cout << "Test 5 PASS: C++ text -> AST sync works" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 51 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
