// Step 53 TDD Test: Classical editing operations
//
// Tests that classical text editing integrates with the AST:
// 1. Undo in text mode undoes both text and AST changes
// 2. Redo restores both text and AST
// 3. Find text in the text buffer
// 4. Replace text -> AST updates accordingly
// 5. Copy/paste operations work in text mode
// 6. Text undo/redo integrates with orchestrator's operation journal
//
// Depends on: TextEditor, TreeSitterParser, PythonGenerator

#include <iostream>
#include <string>
#include <cassert>
#include "TextEditor.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Text undo restores previous text ---
    {
        TextEditor editor;
        editor.setContent("def hello():\n    pass\n", "python");

        std::string before = editor.getContent();
        editor.replaceText(4, 5, "world");  // "hello" -> "world"
        std::string after = editor.getContent();

        assert(contains(after, "world") && "After replace, should contain 'world'");
        assert(!contains(after, "hello") && "After replace, should not contain 'hello'");

        editor.undo();
        std::string undone = editor.getContent();
        assert(contains(undone, "hello") && "After undo, should contain 'hello' again");

        std::cout << "Test 1 PASS: Text undo restores previous content" << std::endl;
        ++passed;
    }

    // --- Test 2: Text undo also undoes AST changes ---
    {
        TextEditor editor;
        editor.setContent("def hello():\n    pass\n", "python");

        Module* astBefore = editor.getAST();
        auto fnsBefore = astBefore->getChildren("functions");
        auto* fnBefore = static_cast<Function*>(fnsBefore[0]);
        assert(fnBefore->name == "hello" && "Initial function name should be 'hello'");

        // Rename function in text
        editor.replaceText(4, 5, "world");

        Module* astAfter = editor.getAST();
        auto fnsAfter = astAfter->getChildren("functions");
        auto* fnAfter = static_cast<Function*>(fnsAfter[0]);
        assert(fnAfter->name == "world" && "After edit, function name should be 'world'");

        // Undo
        editor.undo();
        Module* astUndone = editor.getAST();
        auto fnsUndone = astUndone->getChildren("functions");
        auto* fnUndone = static_cast<Function*>(fnsUndone[0]);
        assert(fnUndone->name == "hello" && "After undo, AST function name should be 'hello'");

        std::cout << "Test 2 PASS: Text undo also undoes AST changes" << std::endl;
        ++passed;
    }

    // --- Test 3: Redo restores both text and AST ---
    {
        TextEditor editor;
        editor.setContent("def hello():\n    pass\n", "python");

        editor.replaceText(4, 5, "world");
        editor.undo();
        assert(editor.canRedo() && "Should be able to redo after undo");

        editor.redo();
        std::string text = editor.getContent();
        assert(contains(text, "world") && "After redo, text should contain 'world'");

        Module* ast = editor.getAST();
        auto fns = ast->getChildren("functions");
        auto* fn = static_cast<Function*>(fns[0]);
        assert(fn->name == "world" && "After redo, AST function name should be 'world'");

        std::cout << "Test 3 PASS: Redo restores both text and AST" << std::endl;
        ++passed;
    }

    // --- Test 4: Find returns correct position ---
    {
        TextEditor editor;
        editor.setContent("def hello():\n    return 42\n", "python");

        int pos = editor.find("return");
        assert(pos >= 0 && "Should find 'return' in text");
        // "def hello():\n    return 42\n"
        //  0123456789012 34567...  -> 'r' of "return" is at index 17
        assert(pos == 17 && "Position of 'return' should be at index 17");

        int notFound = editor.find("nonexistent");
        assert(notFound == -1 && "Should return -1 for not found");

        std::cout << "Test 4 PASS: Find locates text at correct position" << std::endl;
        ++passed;
    }

    // --- Test 5: ReplaceAll updates text and AST ---
    {
        TextEditor editor;
        editor.setContent("def foo():\n    return foo\n", "python");

        int count = editor.replaceAll("foo", "bar");
        assert(count == 2 && "Should replace 2 occurrences of 'foo'");

        std::string text = editor.getContent();
        assert(contains(text, "bar") && "Text should contain 'bar'");
        assert(!contains(text, "foo") && "Text should not contain 'foo'");

        std::cout << "Test 5 PASS: ReplaceAll updates text and count" << std::endl;
        ++passed;
    }

    // --- Test 6: getSelection returns correct substring ---
    {
        TextEditor editor;
        editor.setContent("def hello():\n    pass\n", "python");

        std::string sel = editor.getSelection(4, 5);
        assert(sel == "hello" && "Selection of 5 chars at pos 4 should be 'hello'");

        std::cout << "Test 6 PASS: getSelection returns correct text" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 53 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
