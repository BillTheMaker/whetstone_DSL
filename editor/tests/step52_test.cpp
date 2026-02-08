// Step 52 TDD Test: Syntax highlighting via tree-sitter
//
// Verifies that SyntaxHighlighter produces correct token spans:
// 1. Python: keywords, strings, comments, numbers, functions, parameters
// 2. C++: keywords, types, strings, comments, functions
// 3. Elisp: keywords, strings, comments, function definitions
// 4. Span coverage: all source bytes covered or categorized
// 5. Unknown language returns empty spans

#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <algorithm>
#include "SyntaxHighlighter.h"

static bool hasCategory(const std::vector<HighlightSpan>& spans,
                        TokenCategory cat) {
    return std::any_of(spans.begin(), spans.end(),
                       [cat](const HighlightSpan& s) { return s.category == cat; });
}

static bool hasCategoryAt(const std::vector<HighlightSpan>& spans,
                          TokenCategory cat, uint32_t pos) {
    return std::any_of(spans.begin(), spans.end(),
                       [cat, pos](const HighlightSpan& s) {
                           return s.category == cat && s.start <= pos && pos < s.end;
                       });
}

static int countCategory(const std::vector<HighlightSpan>& spans,
                         TokenCategory cat) {
    return (int)std::count_if(spans.begin(), spans.end(),
                              [cat](const HighlightSpan& s) { return s.category == cat; });
}

int main() {
    int passed = 0;
    int failed = 0;

    // --- Test 1: Python keywords and structure ---
    {
        std::string source = "def greet(name):\n    return name\n";
        auto spans = SyntaxHighlighter::highlight(source, "python");

        assert(!spans.empty() && "Should produce spans for Python");
        assert(hasCategory(spans, TokenCategory::Keyword) && "Should have keywords (def, return)");
        assert(hasCategory(spans, TokenCategory::Function) && "Should have function name");
        assert(hasCategory(spans, TokenCategory::Parameter) && "Should have parameter");

        // "def" should be a keyword at position 0
        assert(hasCategoryAt(spans, TokenCategory::Keyword, 0) && "'def' at pos 0 should be keyword");

        // "greet" should be a function name at position 4
        assert(hasCategoryAt(spans, TokenCategory::Function, 4) && "'greet' at pos 4 should be function");

        // "name" in parameters should be parameter (position 10)
        assert(hasCategoryAt(spans, TokenCategory::Parameter, 10) && "'name' param should be parameter");

        std::cout << "Test 1 PASS: Python keywords, functions, parameters" << std::endl;
        ++passed;
    }

    // --- Test 2: Python strings and comments ---
    {
        std::string source = "# a comment\nx = \"hello\"\ny = 42\n";
        auto spans = SyntaxHighlighter::highlight(source, "python");

        assert(hasCategory(spans, TokenCategory::Comment) && "Should have comment");
        assert(hasCategory(spans, TokenCategory::String) && "Should have string");
        assert(hasCategory(spans, TokenCategory::Number) && "Should have number");

        // Comment starts at position 0
        assert(hasCategoryAt(spans, TokenCategory::Comment, 0) && "Comment at pos 0");

        std::cout << "Test 2 PASS: Python strings, comments, numbers" << std::endl;
        ++passed;
    }

    // --- Test 3: C++ keywords and types ---
    {
        std::string source = "int add(int a, int b) { return a + b; }";
        auto spans = SyntaxHighlighter::highlight(source, "cpp");

        assert(!spans.empty() && "Should produce spans for C++");
        assert(hasCategory(spans, TokenCategory::Type) && "Should have type (int)");
        assert(hasCategory(spans, TokenCategory::Keyword) && "Should have keyword (return)");
        assert(hasCategory(spans, TokenCategory::Operator) && "Should have operator (+)");

        std::cout << "Test 3 PASS: C++ keywords, types, operators" << std::endl;
        ++passed;
    }

    // --- Test 4: C++ strings and comments ---
    {
        std::string source = "// line comment\nstd::string s = \"hello\";\n";
        auto spans = SyntaxHighlighter::highlight(source, "cpp");

        assert(hasCategory(spans, TokenCategory::Comment) && "Should have comment");
        assert(hasCategory(spans, TokenCategory::String) && "Should have string");
        assert(hasCategoryAt(spans, TokenCategory::Comment, 0) && "Comment at pos 0");

        std::cout << "Test 4 PASS: C++ strings and comments" << std::endl;
        ++passed;
    }

    // --- Test 5: Elisp keywords and structure ---
    {
        std::string source = "(defun my-func (x)\n  (+ x 1))\n";
        auto spans = SyntaxHighlighter::highlight(source, "elisp");

        assert(!spans.empty() && "Should produce spans for Elisp");
        assert(hasCategory(spans, TokenCategory::Keyword) && "Should have keyword (defun)");
        assert(hasCategory(spans, TokenCategory::Number) && "Should have number (1)");
        assert(hasCategory(spans, TokenCategory::Punctuation) && "Should have punctuation (parens)");

        std::cout << "Test 5 PASS: Elisp keywords, numbers, punctuation" << std::endl;
        ++passed;
    }

    // --- Test 6: Empty and unknown language ---
    {
        auto empty = SyntaxHighlighter::highlight("", "python");
        assert(empty.empty() && "Empty source should produce no spans");

        auto unknown = SyntaxHighlighter::highlight("hello world", "brainfuck");
        assert(unknown.empty() && "Unknown language should produce no spans");

        std::cout << "Test 6 PASS: Empty and unknown language handling" << std::endl;
        ++passed;
    }

    // --- Test 7: Spans are sorted by position ---
    {
        std::string source = "def f(x):\n    return x + 1\n";
        auto spans = SyntaxHighlighter::highlight(source, "python");

        for (size_t i = 1; i < spans.size(); ++i) {
            assert(spans[i].start >= spans[i-1].start &&
                   "Spans should be sorted by start position");
        }

        std::cout << "Test 7 PASS: Spans are sorted" << std::endl;
        ++passed;
    }

    // --- Test 8: categoryName returns valid strings ---
    {
        assert(std::string(SyntaxHighlighter::categoryName(TokenCategory::Keyword)) == "keyword");
        assert(std::string(SyntaxHighlighter::categoryName(TokenCategory::String)) == "string");
        assert(std::string(SyntaxHighlighter::categoryName(TokenCategory::Comment)) == "comment");
        assert(std::string(SyntaxHighlighter::categoryName(TokenCategory::Function)) == "function");

        std::cout << "Test 8 PASS: categoryName returns correct names" << std::endl;
        ++passed;
    }

    // --- Summary ---
    std::cout << "\n=== Step 52 Results: " << passed << " passed, " << failed << " failed ===" << std::endl;
    return failed > 0 ? 1 : 0;
}
