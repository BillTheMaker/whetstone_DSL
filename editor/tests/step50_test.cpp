// Step 50 TDD Test: Text editor component
//
// Validates the TextEditor and TextASTSync components exist and
// basic operations work: create, set content, get content, basic
// insert/delete, language selection.

#include <iostream>
#include <string>
#include <cassert>
#include "TextASTSync.h"
#include "TextEditor.h"

static bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: TextASTSync default state ---
    {
        TextASTSync sync;
        assert(sync.getAST() == nullptr && "Default AST should be null");
        assert(!sync.isParsePending() && "No parse should be pending initially");

        std::cout << "Test 1 PASS: TextASTSync default state" << std::endl;
        ++passed;
    }

    // --- Test 2: TextEditor set/get content ---
    {
        TextEditor editor;
        editor.setContent("hello world", "python");
        assert(editor.getContent() == "hello world" && "Content should match");

        std::cout << "Test 2 PASS: TextEditor set/get content" << std::endl;
        ++passed;
    }

    // --- Test 3: TextEditor insert ---
    {
        TextEditor editor;
        editor.setContent("hello world", "python");
        editor.insertText(5, " beautiful");
        assert(editor.getContent() == "hello beautiful world" && "Insert should work");

        std::cout << "Test 3 PASS: TextEditor insert" << std::endl;
        ++passed;
    }

    // --- Test 4: TextEditor delete ---
    {
        TextEditor editor;
        editor.setContent("hello world", "python");
        editor.deleteText(5, 6);  // delete " world"
        assert(editor.getContent() == "hello" && "Delete should work");

        std::cout << "Test 4 PASS: TextEditor delete" << std::endl;
        ++passed;
    }

    // --- Test 5: TextASTSync Python parse produces valid AST ---
    {
        TextASTSync sync;
        sync.setText("def test_func(x, y):\n    return x + y\n", "python");
        sync.syncNow();

        Module* ast = sync.getAST();
        assert(ast != nullptr && "AST should exist after sync");
        assert(ast->targetLanguage == "python" && "Language should be python");

        auto functions = ast->getChildren("functions");
        assert(functions.size() == 1 && "Should have one function");

        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "test_func" && "Function name should match");

        auto params = fn->getChildren("parameters");
        assert(params.size() == 2 && "Should have two parameters");

        std::cout << "Test 5 PASS: Python parse produces valid AST" << std::endl;
        ++passed;
    }

    // --- Test 6: TextEditor produces AST from content ---
    {
        TextEditor editor;
        editor.setContent("def my_func():\n    pass\n", "python");

        Module* ast = editor.getAST();
        assert(ast != nullptr && "TextEditor should produce AST");

        auto functions = ast->getChildren("functions");
        assert(!functions.empty() && "Should have parsed function");

        auto* fn = static_cast<Function*>(functions[0]);
        assert(fn->name == "my_func" && "Function name should match");

        std::cout << "Test 6 PASS: TextEditor produces AST from content" << std::endl;
        ++passed;
    }

    // --- Test 7: TextEditor undo/redo availability ---
    {
        TextEditor editor;
        editor.setContent("test", "python");
        assert(!editor.canUndo() && "Should not be able to undo initially");
        assert(!editor.canRedo() && "Should not be able to redo initially");

        editor.insertText(4, "!");
        assert(editor.canUndo() && "Should be able to undo after edit");

        editor.undo();
        assert(editor.canRedo() && "Should be able to redo after undo");
        assert(editor.getContent() == "test" && "Undo should restore text");

        std::cout << "Test 7 PASS: Undo/redo availability tracking" << std::endl;
        ++passed;
    }

    // --- Test 8: TextASTSync C++ parse ---
    {
        TextASTSync sync;
        sync.setText("void hello() { return; }", "cpp");
        sync.syncNow();

        Module* ast = sync.getAST();
        assert(ast != nullptr && "C++ AST should exist");
        assert(ast->targetLanguage == "cpp" && "Language should be cpp");

        std::cout << "Test 8 PASS: C++ parse via TextASTSync" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 50 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
