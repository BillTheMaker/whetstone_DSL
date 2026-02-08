// Step 58 TDD Test: Mode-specific editor behavior
//
// Verifies that EditorMode:
// 1. Python mode has correct indent/comment/brackets
// 2. C++ mode has correct indent/comment/brackets
// 3. Elisp mode has correct indent/comment/brackets
// 4. shouldIndentAfter detects indent triggers
// 5. toggleLineComment adds/removes comments
// 6. getClosingBracket returns correct pair
// 7. Snippets are accessible by trigger
// 8. Mode name round-trip works

#include <iostream>
#include <string>
#include <cassert>
#include "EditorMode.h"

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Python mode defaults ---
    {
        EditorMode mode("python");
        assert(mode.getType() == EditorModeType::Python && "Should be Python mode");
        assert(mode.getCommentStyle().lineComment == "#" && "Python comment is #");
        assert(mode.getTabSize() == 4 && "Python tab size is 4");
        assert(!mode.getIndentRule().useTabs && "Python uses spaces");

        std::cout << "Test 1 PASS: Python mode defaults" << std::endl;
        ++passed;
    }

    // --- Test 2: C++ mode defaults ---
    {
        EditorMode mode("cpp");
        assert(mode.getType() == EditorModeType::Cpp && "Should be C++ mode");
        assert(mode.getCommentStyle().lineComment == "//" && "C++ line comment is //");
        assert(mode.getCommentStyle().blockStart == "/*" && "C++ block start is /*");
        assert(mode.getCommentStyle().blockEnd == "*/" && "C++ block end is */");

        std::cout << "Test 2 PASS: C++ mode defaults" << std::endl;
        ++passed;
    }

    // --- Test 3: Elisp mode defaults ---
    {
        EditorMode mode("elisp");
        assert(mode.getType() == EditorModeType::Elisp && "Should be Elisp mode");
        assert(mode.getCommentStyle().lineComment == ";;" && "Elisp comment is ;;");
        assert(mode.getTabSize() == 2 && "Elisp tab size is 2");

        std::cout << "Test 3 PASS: Elisp mode defaults" << std::endl;
        ++passed;
    }

    // --- Test 4: shouldIndentAfter detects Python colon ---
    {
        EditorMode mode("python");
        assert(mode.shouldIndentAfter("def foo():") && "Should indent after def:");
        assert(mode.shouldIndentAfter("if x > 0:") && "Should indent after if:");
        assert(!mode.shouldIndentAfter("x = 42") && "Should not indent after assignment");

        EditorMode cppMode("cpp");
        assert(cppMode.shouldIndentAfter("int main() {") && "Should indent after {");
        assert(!cppMode.shouldIndentAfter("return 0;") && "Should not indent after ;");

        std::cout << "Test 4 PASS: shouldIndentAfter detects triggers" << std::endl;
        ++passed;
    }

    // --- Test 5: toggleLineComment adds and removes comments ---
    {
        EditorMode mode("python");

        // Add comment
        std::string commented = mode.toggleLineComment("    x = 42");
        assert(commented == "    # x = 42" && "Should add # comment preserving indent");

        // Remove comment
        std::string uncommented = mode.toggleLineComment("    # x = 42");
        assert(uncommented == "    x = 42" && "Should remove # comment");

        // C++ mode
        EditorMode cppMode("cpp");
        std::string cppCommented = cppMode.toggleLineComment("    int x = 42;");
        assert(cppCommented == "    // int x = 42;" && "C++ should add //");

        std::string cppUncommented = cppMode.toggleLineComment("    // int x = 42;");
        assert(cppUncommented == "    int x = 42;" && "C++ should remove //");

        std::cout << "Test 5 PASS: toggleLineComment works" << std::endl;
        ++passed;
    }

    // --- Test 6: getClosingBracket ---
    {
        EditorMode mode("python");
        assert(mode.getClosingBracket('(') == ')' && "( should close with )");
        assert(mode.getClosingBracket('[') == ']' && "[ should close with ]");
        assert(mode.getClosingBracket('{') == '}' && "{ should close with }");
        assert(mode.getClosingBracket('x') == 0 && "x has no closing bracket");

        EditorMode cppMode("cpp");
        assert(cppMode.getClosingBracket('<') == '>' && "< should close with > in C++");

        std::cout << "Test 6 PASS: getClosingBracket returns correct pair" << std::endl;
        ++passed;
    }

    // --- Test 7: Snippets accessible by trigger ---
    {
        EditorMode mode("python");
        auto* defSnip = mode.findSnippet("def");
        assert(defSnip != nullptr && "Should find 'def' snippet");
        assert(defSnip->expansion.find("def ") != std::string::npos && "Snippet should contain 'def'");

        auto* noSnip = mode.findSnippet("nonexistent");
        assert(noSnip == nullptr && "Should not find nonexistent snippet");

        EditorMode cppMode("cpp");
        auto* mainSnip = cppMode.findSnippet("main");
        assert(mainSnip != nullptr && "C++ should have 'main' snippet");

        std::cout << "Test 7 PASS: Snippets accessible by trigger" << std::endl;
        ++passed;
    }

    // --- Test 8: Mode name and language round-trip ---
    {
        assert(EditorMode::modeFromLanguage("python") == EditorModeType::Python);
        assert(EditorMode::modeFromLanguage("cpp") == EditorModeType::Cpp);
        assert(EditorMode::modeFromLanguage("elisp") == EditorModeType::Elisp);
        assert(EditorMode::modeFromLanguage("unknown") == EditorModeType::PlainText);

        assert(std::string(EditorMode::modeName(EditorModeType::Python)) == "Python");
        assert(std::string(EditorMode::modeName(EditorModeType::Cpp)) == "C++");

        std::cout << "Test 8 PASS: Mode name and language round-trip" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 58 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
